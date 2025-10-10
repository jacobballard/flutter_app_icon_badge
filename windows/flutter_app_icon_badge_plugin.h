#ifndef FLUTTER_PLUGIN_FLUTTER_APP_ICON_BADGE_PLUGIN_H_
#define FLUTTER_PLUGIN_FLUTTER_APP_ICON_BADGE_PLUGIN_H_

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>

#include <memory>

namespace flutter_app_icon_badge {

class FlutterAppIconBadgePlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  FlutterAppIconBadgePlugin();

  virtual ~FlutterAppIconBadgePlugin();

  // Disallow copy and assign.
  FlutterAppIconBadgePlugin(const FlutterAppIconBadgePlugin&) = delete;
  FlutterAppIconBadgePlugin& operator=(const FlutterAppIconBadgePlugin&) = delete;

  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
};

}  // namespace flutter_app_icon_badge

#endif  // FLUTTER_PLUGIN_FLUTTER_APP_ICON_BADGE_PLUGIN_H_
