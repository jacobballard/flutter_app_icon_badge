package dev.badver.flutter_app_icon_badge

import io.flutter.embedding.engine.plugins.FlutterPlugin
import io.flutter.plugin.common.MethodCall
import io.flutter.plugin.common.MethodChannel
import io.flutter.plugin.common.MethodChannel.MethodCallHandler
import io.flutter.plugin.common.MethodChannel.Result

/** FlutterAppIconBadgePlugin */
class FlutterAppIconBadgePlugin :
    FlutterPlugin,
    MethodCallHandler {
    // The MethodChannel that will the communication between Flutter and native Android
    //
    // This local reference serves to register the plugin with the Flutter Engine and unregister it
    // when the Flutter Engine is detached from the Activity
    private lateinit var channel: MethodChannel

    override fun onAttachedToEngine(flutterPluginBinding: FlutterPlugin.FlutterPluginBinding) {
        channel = MethodChannel(flutterPluginBinding.binaryMessenger, "flutter_app_icon_badge")
        channel.setMethodCallHandler(this)
    }

    override fun onMethodCall(
        call: MethodCall,
        result: Result
    ) {
        when (call.method) {
            "updateBadge" -> {
                // Android support has been removed in v2.1.0
                result.notImplemented()
            }
            "removeBadge" -> {
                // Android support has been removed in v2.1.0
                result.notImplemented()
            }
            "isAppBadgeSupported" -> {
                // Android support has been removed - badges are not supported
                result.success(false)
            }
            "isAppFocused" -> {
                // Android support has been removed in v2.1.0
                result.notImplemented()
            }
            else -> {
                result.notImplemented()
            }
        }
    }

    override fun onDetachedFromEngine(binding: FlutterPlugin.FlutterPluginBinding) {
        channel.setMethodCallHandler(null)
    }
}
