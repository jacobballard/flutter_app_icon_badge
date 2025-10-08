// This is a basic Flutter integration test.
//
// To perform an interaction with a widget in your test, use the WidgetTester
// utility that Flutter provides. For example, you can send tap and scroll
// gestures. You can also use WidgetTester to find child widgets in the widget
// tree, read text, and verify that the values of widget properties are correct.

import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';

import 'package:flutter_app_icon_badge_example/main.dart' as app;

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  testWidgets('Badge plugin integration test', (WidgetTester tester) async {
    // Build our app and trigger a frame.
    app.main();

    // Trigger a frame.
    await tester.pumpAndSettle();

    // Verify that badge support status is displayed.
    expect(
      find.byWidgetPredicate(
        (Widget widget) =>
            widget is Text && widget.data?.startsWith('Badge supported:') == true,
      ),
      findsOneWidget,
    );

    // Test add badge button
    await tester.tap(find.text('Add badge'));
    await tester.pumpAndSettle();

    // Test remove badge button
    await tester.tap(find.text('Remove badge'));
    await tester.pumpAndSettle();
  });
}
