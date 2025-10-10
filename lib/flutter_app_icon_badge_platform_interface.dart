import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'flutter_app_icon_badge_method_channel.dart';

abstract class FlutterAppIconBadgePlatform extends PlatformInterface {
  /// Constructs a FlutterAppIconBadgePlatform.
  FlutterAppIconBadgePlatform() : super(token: _token);

  static final Object _token = Object();

  static FlutterAppIconBadgePlatform _instance = MethodChannelFlutterAppIconBadge();

  /// The default instance of [FlutterAppIconBadgePlatform] to use.
  ///
  /// Defaults to [MethodChannelFlutterAppIconBadge].
  static FlutterAppIconBadgePlatform get instance => _instance;

  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [FlutterAppIconBadgePlatform] when
  /// they register themselves.
  static set instance(FlutterAppIconBadgePlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  Future<String?> getPlatformVersion() {
    throw UnimplementedError('platformVersion() has not been implemented.');
  }
}
