#include <QApplication>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QAction>
#include <QSettings>
#include <QJsonObject>

#include "nexgen/sys/Settings.h"
#include "nexgen/sys/hotkeys/HotkeyManager.h"
#include "nexgen/sys/ipc/IpcClient.h"

#include "nexgen/themes/ThemeManager.h"

static constexpr int kHotkeyClockToggle = 1001;

static QJsonObject cmd(const char* c) {
  return QJsonObject{{"cmd", QString::fromUtf8(c)}};
}

int main(int argc, char** argv) {
  QApplication app(argc, argv);
  app.setQuitOnLastWindowClosed(false);

  // Unified settings namespace
  QSettings settings("Nexgen", "Utilities");

  // Theme (placeholder palette for now; will be replaced when we extract real themes)
  nexgen::themes::ThemeManager themes;
  themes.load(settings);
  themes.applyTo(app);
  QObject::connect(&themes, &nexgen::themes::ThemeManager::themeChanged, [&] {
    themes.applyTo(app);
    themes.save(settings);
  });

  // Tray
  QSystemTrayIcon tray;
  tray.setToolTip(QStringLiteral("Nexgen Utilities"));
  tray.setIcon(QIcon::fromTheme(QStringLiteral("applications-system")));

  QMenu menu;

  auto* toggleClock = menu.addAction(QStringLiteral("Clock (Ctrl+Alt+T)"));
  auto* quit = menu.addAction(QStringLiteral("Quit"));

  nexgen::sys::ipc::IpcClient ipc;

  auto toggleClockFn = [&] {
    const auto resp = ipc.request(QStringLiteral("nexgen.clock"), cmd("toggle"), 200);
    if (!resp.value("ok").toBool()) {
      tray.showMessage(
        QStringLiteral("Clock not running"),
        QStringLiteral("Could not reach nexgen.clock yet (clock utility not started)."),
        QSystemTrayIcon::Information,
        3000
      );
    }
  };

  QObject::connect(toggleClock, &QAction::triggered, toggleClockFn);
  QObject::connect(quit, &QAction::triggered, &app, &QCoreApplication::quit);

  tray.setContextMenu(&menu);
  tray.show();

  // Hotkeys (hub owns hotkeys)
  nexgen::sys::hotkeys::HotkeyManager hotkeys;
  hotkeys.setCallback([&](int id) {
    if (id == kHotkeyClockToggle) toggleClockFn();
  });

  // Ctrl+Alt+T
  const bool ok = hotkeys.registerHotkey(kHotkeyClockToggle, true, true, false, 'T');
  if (!ok) {
    tray.showMessage(
      QStringLiteral("Hotkey registration failed"),
      QStringLiteral("Ctrl+Alt+T could not be registered (already in use?)."),
      QSystemTrayIcon::Warning,
      5000
    );
  }

  const int rc = app.exec();
  hotkeys.unregisterHotkey(kHotkeyClockToggle);
  return rc;
}
