import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_app_icon_badge/flutter_app_icon_badge.dart';

void main() => runApp(const MyApp());

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  String _appBadgeSupported = 'Unknown';
  bool _isFocused = true;

  @override
  void initState() {
    super.initState();
    initPlatformState();
  }

  Future<void> initPlatformState() async {
    String appBadgeSupported;
    try {
      bool res = await FlutterAppIconBadge.isAppBadgeSupported();
      if (res) {
        appBadgeSupported = 'Supported';
      } else {
        appBadgeSupported = 'Not supported';
      }
    } on PlatformException {
      appBadgeSupported = 'Failed to get badge support.';
    }

    // If the widget was removed from the tree while the asynchronous platform
    // message was in flight, we want to discard the reply rather than calling
    // setState to update our non-existent appearance.
    if (!mounted) return;

    setState(() {
      _appBadgeSupported = appBadgeSupported;
    });
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Plugin example app'),
        ),
        body: SizedBox.expand(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            crossAxisAlignment: CrossAxisAlignment.center,
            children: <Widget>[
              Text('Badge supported: $_appBadgeSupported'),
              Text('Focused: $_isFocused'),
              ElevatedButton(
                onPressed: _addBadge,
                child: const Text('Add badge'),
              ),
              ElevatedButton(
                onPressed: _removeBadge,
                child: const Text('Remove badge'),
              ),
              ElevatedButton(
                onPressed: () {
                  Future.delayed(const Duration(seconds: 5), () async {
                    final isFocused = await FlutterAppIconBadge.isAppFocused();
                    setState(() {
                      _isFocused = isFocused;
                    });
                  });
                },
                child: const Text('Check focus after 5 seconds'),
              ),
            ],
          ),
        ),
      ),
    );
  }

  void _addBadge() {
    FlutterAppIconBadge.updateBadge(1);
  }

  void _removeBadge() {
    FlutterAppIconBadge.removeBadge();
  }
}
