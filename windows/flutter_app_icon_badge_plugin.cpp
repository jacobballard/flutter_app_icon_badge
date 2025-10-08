#include "include/flutter_app_icon_badge/flutter_app_icon_badge_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <VersionHelpers.h>

// For RPC constants
#include <rpcndr.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <map>
#include <memory>
#include <sstream>

// WinRT includes for badge notifications
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Data.Xml.Dom.h>
#include <winrt/Windows.UI.Notifications.h>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Data::Xml::Dom;
using namespace Windows::UI::Notifications;

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
  void UpdateBadge(int count);
  void RemoveBadge();
  bool IsAppBadgeSupported();
  bool IsAppFocused();
  
  // WinRT initialization
  void EnsureWinRTInitialized();
  
 private:
  bool winrt_initialized_ = false;
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

FlutterAppIconBadgePlugin::FlutterAppIconBadgePlugin() {}

FlutterAppIconBadgePlugin::~FlutterAppIconBadgePlugin() {}

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
          UpdateBadge(count);
          result->Success();
          return;
        }
      }
      result->Error("INVALID_ARGUMENT", "Count parameter is required");
    } catch (const std::exception& e) {
      result->Error("BADGE_ERROR", "Failed to update badge", flutter::EncodableValue(e.what()));
    }
  } else if (method_call.method_name().compare("removeBadge") == 0) {
    try {
      RemoveBadge();
      result->Success();
    } catch (const std::exception& e) {
      result->Error("BADGE_ERROR", "Failed to remove badge", flutter::EncodableValue(e.what()));
    }
  } else if (method_call.method_name().compare("isAppBadgeSupported") == 0) {
    result->Success(flutter::EncodableValue(IsAppBadgeSupported()));
  } else if (method_call.method_name().compare("isAppFocused") == 0) {
    result->Success(flutter::EncodableValue(IsAppFocused()));
  } else {
    result->NotImplemented();
  }
}

void FlutterAppIconBadgePlugin::UpdateBadge(int count) {
  try {
    EnsureWinRTInitialized();
    
    // If count is 0, clear the badge instead
    if (count <= 0) {
      auto badgeUpdater = BadgeUpdateManager::CreateBadgeUpdaterForApplication();
      badgeUpdater.Clear();
      return;
    }
    
    // Get the badge template for numeric badges
    auto badgeXml = BadgeUpdateManager::GetTemplateContent(BadgeTemplateType::BadgeNumber);
    
    // Set the badge value
    auto badgeElement = badgeXml.SelectSingleNode(L"/badge").as<XmlElement>();
    badgeElement.SetAttribute(L"value", winrt::to_hstring(count));
    
    // Create the badge notification
    auto badge = BadgeNotification(badgeXml);
    
    // Create the badge updater for the application
    auto badgeUpdater = BadgeUpdateManager::CreateBadgeUpdaterForApplication();
    
    // Update the badge
    badgeUpdater.Update(badge);
  } catch (const winrt::hresult_error& ex) {
    // Handle WinRT specific exceptions
    std::string error_msg = "WinRT error: " + winrt::to_string(ex.message());
    throw std::runtime_error(error_msg);
  } catch (const std::exception& ex) {
    // Handle standard exceptions
    std::string error_msg = "Standard error: " + std::string(ex.what());
    throw std::runtime_error(error_msg);
  } catch (...) {
    // Handle unknown exceptions
    throw std::runtime_error("Unknown error occurred while updating badge");
  }
}

void FlutterAppIconBadgePlugin::RemoveBadge() {
  try {
    EnsureWinRTInitialized();
    
    // Create the badge updater for the application
    auto badgeUpdater = BadgeUpdateManager::CreateBadgeUpdaterForApplication();
    
    // Clear the badge
    badgeUpdater.Clear();
  } catch (const winrt::hresult_error& ex) {
    // Handle WinRT specific exceptions
    std::string error_msg = "WinRT error: " + winrt::to_string(ex.message());
    throw std::runtime_error(error_msg);
  } catch (const std::exception& ex) {
    // Handle standard exceptions
    std::string error_msg = "Standard error: " + std::string(ex.what());
    throw std::runtime_error(error_msg);
  } catch (...) {
    // Handle unknown exceptions
    throw std::runtime_error("Unknown error occurred while removing badge");
  }
}

bool FlutterAppIconBadgePlugin::IsAppBadgeSupported() {
  // Badge notifications are supported on Windows 10 and later
  return IsWindows10OrGreater();
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

}  // namespace

void FlutterAppIconBadgePluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  FlutterAppIconBadgePlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
