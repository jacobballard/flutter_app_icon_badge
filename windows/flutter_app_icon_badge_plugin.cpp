#include "include/flutter_app_icon_badge/flutter_app_icon_badge_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <VersionHelpers.h>

// For RPC constants
#include <rpcndr.h>

// For taskbar overlay icons
#include <shobjidl.h>
#include <comdef.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <map>
#include <memory>
#include <sstream>
#include <iomanip>

// WinRT includes for badge notifications
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Data.Xml.Dom.h>
#include <winrt/Windows.UI.Notifications.h>
#include <winrt/Windows.ApplicationModel.h>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Data::Xml::Dom;
using namespace Windows::UI::Notifications;
using namespace Windows::ApplicationModel;

namespace {

class FlutterAppIconBadgePlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  FlutterAppIconBadgePlugin();

  virtual ~FlutterAppIconBadgePlugin();

 private:
  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  
  // Helper methods for badge operations
  bool UpdateBadge(int count);
  bool RemoveBadge();
  bool IsAppBadgeSupported();
  bool IsAppFocused();
  
  // WinRT initialization
  void EnsureWinRTInitialized();
  
  // Check if app is packaged
  bool IsPackagedApp();
  
  // Taskbar overlay methods (for unpackaged apps)
  bool UpdateBadgeWithOverlay(int count);
  bool RemoveBadgeOverlay();
  HICON CreateBadgeIcon(int count);
  HWND GetMainWindowHandle();
  
 private:
  bool winrt_initialized_ = false;
  ITaskbarList3* taskbar_list_ = nullptr;
};

// static
void FlutterAppIconBadgePlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "flutter_app_icon_badge",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<FlutterAppIconBadgePlugin>();

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

FlutterAppIconBadgePlugin::FlutterAppIconBadgePlugin() {
  // Initialize COM for taskbar operations
  CoInitialize(nullptr);
  
  // Create taskbar list interface
  CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_INPROC_SERVER,
                   IID_ITaskbarList3, (void**)&taskbar_list_);
  
  if (taskbar_list_) {
    taskbar_list_->HrInit();
  }
}

FlutterAppIconBadgePlugin::~FlutterAppIconBadgePlugin() {
  if (taskbar_list_) {
    taskbar_list_->Release();
    taskbar_list_ = nullptr;
  }
  CoUninitialize();
}

void FlutterAppIconBadgePlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  if (method_call.method_name().compare("updateBadge") == 0) {
    try {
      const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
      if (arguments) {
        auto count_it = arguments->find(flutter::EncodableValue("count"));
        if (count_it != arguments->end()) {
          int count = std::get<int>(count_it->second);
          if (UpdateBadge(count)) {
            result->Success();
          } else {
            result->Error("BADGE_ERROR", "Failed to update badge");
          }
          return;
        }
      }
      result->Error("INVALID_ARGUMENT", "Count parameter is required");
    } catch (...) {
      result->Error("BADGE_ERROR", "Failed to update badge");
    }
  } else if (method_call.method_name().compare("removeBadge") == 0) {
    try {
      if (RemoveBadge()) {
        result->Success();
      } else {
        result->Error("BADGE_ERROR", "Failed to remove badge");
      }
    } catch (...) {
      result->Error("BADGE_ERROR", "Failed to remove badge");
    }
  } else if (method_call.method_name().compare("isAppBadgeSupported") == 0) {
    result->Success(flutter::EncodableValue(IsAppBadgeSupported()));
  } else if (method_call.method_name().compare("isAppFocused") == 0) {
    result->Success(flutter::EncodableValue(IsAppFocused()));
  } else {
    result->NotImplemented();
  }
}

bool FlutterAppIconBadgePlugin::UpdateBadge(int count) {
  // Try WinRT approach first (for packaged apps)
  if (IsPackagedApp()) {
    try {
      EnsureWinRTInitialized();
      
      if (count <= 0) {
        auto badgeUpdater = BadgeUpdateManager::CreateBadgeUpdaterForApplication();
        badgeUpdater.Clear();
        return true;
      }
      
      auto badgeXml = BadgeUpdateManager::GetTemplateContent(BadgeTemplateType::BadgeNumber);
      if (badgeXml) {
        auto badgeElement = badgeXml.SelectSingleNode(L"/badge").as<XmlElement>();
        if (badgeElement) {
          badgeElement.SetAttribute(L"value", winrt::to_hstring(count));
          auto badge = BadgeNotification(badgeXml);
          auto badgeUpdater = BadgeUpdateManager::CreateBadgeUpdaterForApplication();
          if (badgeUpdater) {
            badgeUpdater.Update(badge);
            return true;
          }
        }
      }
    } catch (...) {
      // Fall through to taskbar overlay approach
    }
  }
  
  // Use taskbar overlay approach (for unpackaged apps)
  return UpdateBadgeWithOverlay(count);
}

bool FlutterAppIconBadgePlugin::RemoveBadge() {
  // Try WinRT approach first (for packaged apps)
  if (IsPackagedApp()) {
    try {
      EnsureWinRTInitialized();
      auto badgeUpdater = BadgeUpdateManager::CreateBadgeUpdaterForApplication();
      if (badgeUpdater) {
        badgeUpdater.Clear();
        return true;
      }
    } catch (...) {
      // Fall through to taskbar overlay approach
    }
  }
  
  // Use taskbar overlay approach (for unpackaged apps)
  return RemoveBadgeOverlay();
}

bool FlutterAppIconBadgePlugin::IsAppBadgeSupported() {
  // Badge functionality is supported on Windows 7 and later
  // (WinRT badges on Win10+, taskbar overlays on Win7+)
  return IsWindows7OrGreater();
}

bool FlutterAppIconBadgePlugin::IsAppFocused() {
  // Check if the current window has focus
  HWND foregroundWindow = GetForegroundWindow();
  DWORD currentProcessId = GetCurrentProcessId();
  DWORD foregroundProcessId;
  
  GetWindowThreadProcessId(foregroundWindow, &foregroundProcessId);
  
  return currentProcessId == foregroundProcessId;
}

void FlutterAppIconBadgePlugin::EnsureWinRTInitialized() {
  if (!winrt_initialized_) {
    try {
      winrt::init_apartment(winrt::apartment_type::single_threaded);
      winrt_initialized_ = true;
    } catch (const winrt::hresult_error& ex) {
      // If already initialized, that's fine
      if (ex.code() == RPC_E_CHANGED_MODE) {
        winrt_initialized_ = true;
      } else {
        throw;
      }
    }
  }
}

bool FlutterAppIconBadgePlugin::IsPackagedApp() {
  try {
    EnsureWinRTInitialized();
    // Try to get the current package - this will fail for unpackaged apps
    auto package = Package::Current();
    return package != nullptr;
  } catch (...) {
    // If we can't get the package, we're probably unpackaged
    return false;
  }
}

bool FlutterAppIconBadgePlugin::UpdateBadgeWithOverlay(int count) {
  if (!taskbar_list_) {
    return false;
  }
  
  HWND hwnd = GetMainWindowHandle();
  if (!hwnd) {
    return false;
  }
  
  if (count <= 0) {
    // Remove overlay
    return SUCCEEDED(taskbar_list_->SetOverlayIcon(hwnd, nullptr, L""));
  }
  
  // Create badge icon
  HICON badgeIcon = CreateBadgeIcon(count);
  if (!badgeIcon) {
    return false;
  }
  
  // Set overlay icon
  wchar_t description[32];
  swprintf_s(description, L"Badge: %d", count);
  HRESULT hr = taskbar_list_->SetOverlayIcon(hwnd, badgeIcon, description);
  
  DestroyIcon(badgeIcon);
  return SUCCEEDED(hr);
}

bool FlutterAppIconBadgePlugin::RemoveBadgeOverlay() {
  if (!taskbar_list_) {
    return false;
  }
  
  HWND hwnd = GetMainWindowHandle();
  if (!hwnd) {
    return false;
  }
  
  return SUCCEEDED(taskbar_list_->SetOverlayIcon(hwnd, nullptr, L""));
}

HICON FlutterAppIconBadgePlugin::CreateBadgeIcon(int count) {
  // Create a small icon (16x16) with the badge number
  const int iconSize = 16;
  
  // Create device context
  HDC hdc = GetDC(nullptr);
  HDC memDC = CreateCompatibleDC(hdc);
  
  // Create bitmap
  HBITMAP hBitmap = CreateCompatibleBitmap(hdc, iconSize, iconSize);
  HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, hBitmap);
  
  // Create mask bitmap
  HBITMAP hMask = CreateBitmap(iconSize, iconSize, 1, 1, nullptr);
  
  // Fill with red background
  HBRUSH redBrush = CreateSolidBrush(RGB(255, 0, 0));
  RECT rect = {0, 0, iconSize, iconSize};
  FillRect(memDC, &rect, redBrush);
  DeleteObject(redBrush);
  
  // Draw text
  SetTextColor(memDC, RGB(255, 255, 255));
  SetBkMode(memDC, TRANSPARENT);
  
  wchar_t text[8];
  if (count > 99) {
    wcscpy_s(text, L"99+");
  } else {
    swprintf_s(text, L"%d", count);
  }
  
  // Use small font
  HFONT font = CreateFont(10, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                         CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                         DEFAULT_PITCH | FF_DONTCARE, L"Arial");
  HFONT oldFont = (HFONT)SelectObject(memDC, font);
  
  DrawText(memDC, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
  
  // Cleanup
  SelectObject(memDC, oldFont);
  SelectObject(memDC, oldBitmap);
  DeleteObject(font);
  DeleteDC(memDC);
  ReleaseDC(nullptr, hdc);
  
  // Create icon
  ICONINFO iconInfo = {};
  iconInfo.fIcon = TRUE;
  iconInfo.hbmMask = hMask;
  iconInfo.hbmColor = hBitmap;
  
  HICON hIcon = CreateIconIndirect(&iconInfo);
  
  DeleteObject(hBitmap);
  DeleteObject(hMask);
  
  return hIcon;
}

HWND FlutterAppIconBadgePlugin::GetMainWindowHandle() {
  // Find the main Flutter window
  DWORD processId = GetCurrentProcessId();
  HWND result = nullptr;
  
  struct EnumData {
    DWORD processId;
    HWND* result;
  } enumData = { processId, &result };
  
  EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
    EnumData* data = (EnumData*)lParam;
    DWORD windowProcessId;
    GetWindowThreadProcessId(hwnd, &windowProcessId);
    
    if (windowProcessId == data->processId && IsWindowVisible(hwnd)) {
      // Check if this is the main window (has a title)
      wchar_t title[256];
      if (GetWindowText(hwnd, title, sizeof(title)/sizeof(wchar_t)) > 0) {
        *(data->result) = hwnd;
        return FALSE; // Stop enumeration
      }
    }
    return TRUE; // Continue enumeration
  }, (LPARAM)&enumData);
  
  return result;
}

}  // namespace

void FlutterAppIconBadgePluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  FlutterAppIconBadgePlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
