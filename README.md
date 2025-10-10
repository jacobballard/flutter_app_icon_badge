# Flutter App Icon Badge plugin

> **ℹ️ This is a community fork of [badver/flutter_app_icon_badge](https://github.com/badver/flutter_app_icon_badge).**
>
> This fork may include additional features, bug fixes, and platform support not present in the original repository.


This [Flutter](https://flutter.io) plugin you can use to change the badge on the icon of your app

## Supported platforms
* iOS
* ~~Android~~ - **REMOVED** in v2.1.0 (unreliable support across devices)
* macOS
* Windows - ✅ **NEW!** Full support using taskbar overlay icons
* Linux - work in progress (need help)

## Getting Started

### iOS

On iOS, the notification permission is required to update the badge.
It is automatically asked when the badge is added or removed.

Please also add the following to your Info.plist:
```xml
<key>UIBackgroundModes</key>
    <array>
        <string>remote-notification</string>
    </array>
```

### Windows

On Windows 10 and later, badge notifications are fully supported using the Windows Runtime (WinRT) API. The badge will appear on both the taskbar icon and the Start menu tile. No additional configuration is required.

### Dart

First, you just have to import the package in your dart files with:
```dart
import 'package:flutter_app_icon_badge/flutter_app_icon_badge.dart';
```

Then you can add a badge:
```dart
FlutterAppIconBadge.updateBadge(1);
```

Remove a badge:
```dart
FlutterAppIconBadge.removeBadge();
```

Or just check if the device supports this feature with:
```dart
FlutterAppIconBadge.isAppBadgeSupported();
```

Another useful method in this plugin - detect if flutter desktop window in focus or not:
```dart
FlutterAppIconBadge.isAppFocused();
```