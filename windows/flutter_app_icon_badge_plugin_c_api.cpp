#include "include/flutter_app_icon_badge/flutter_app_icon_badge_plugin_c_api.h"

#include <flutter/plugin_registrar_windows.h>

#include "flutter_app_icon_badge_plugin.h"

void FlutterAppIconBadgePluginCApiRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  flutter_app_icon_badge::FlutterAppIconBadgePlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
