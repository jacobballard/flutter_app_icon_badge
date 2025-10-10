import Flutter
import UIKit

public class FlutterAppIconBadgePlugin: NSObject, FlutterPlugin {
  public static func register(with registrar: FlutterPluginRegistrar) {
    let channel = FlutterMethodChannel(name: "flutter_app_icon_badge", binaryMessenger: registrar.messenger())
    let instance = FlutterAppIconBadgePlugin()
    registrar.addMethodCallDelegate(instance, channel: channel)
  }

  public func handle(_ call: FlutterMethodCall, result: @escaping FlutterResult) {
    switch call.method {
        case "updateBadge":
          if let args = call.arguments as? Dictionary<String, Any>,
            let count = args["count"] as? Int {
            UIApplication.shared.applicationIconBadgeNumber = count
            result(nil)
          } else {
            result(FlutterError.init(code: "bad args", message: nil, details: nil))
          }
        case "removeBadge":
          UIApplication.shared.applicationIconBadgeNumber = 0
          result(nil)
        case "isAppBadgeSupported":
          result(true)
        case "isAppFocused":
          result(UIApplication.shared.applicationState == .active)
        default:
          result(FlutterMethodNotImplemented)
    }
  }
}
