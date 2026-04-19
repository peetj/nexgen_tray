#include <QApplication>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QAction>
#include <QSettings>
#include <QJsonObject>

#include "nexgen/sys/hotkeys/HotkeyManager.h"
#include "nexgen/sys/ipc/IpcClient.h"

#include "nexgen/themes/ThemeManager.h"
#include "nexgen/themes/ThemeTypes.h"

static constexpr int kHotkeyClockToggle = 1001;

static QJsonObject cmd(const char* c) {
  return QJsonObject{{"cmd", QString::fromUtf8(c)}};
}

static void sendReloadTheme(nexgen::sys::ipc::IpcClient& ipc, QSystemTrayIcon& tray) {
  const auto resp = ipc.request(QStringLiteral("nexgen.clock"), cmd("reloadTheme"), 150);
  if (!resp.value("ok").toBool()) {
    // Silent-ish; clock may not be running.
    tray.showMessage(
      QStringLiteral("Theme updated"),
      QStringLiteral("Clock theme will apply next time it starts."),
      QSystemTrayIcon::Information,
      2000
    );
  }
}

int main(int argc, char** argv) {
  QApplication app(argc, argv);
  app.setQuitOnLastWindowClosed(false);

  // Unified settings namespace
  QSettings settings("Nexgen", "Utilities");

  nexgen::themes::ThemeManager themes;
  themes.load(settings);
  themes.applyTo(app);

  QObject::connect(&themes, &nexgen::themes::ThemeManager::themeChanged, [&] {
    themes.applyTo(app);
    themes.save(settings);
  });

  QSystemTrayIcon tray;
  tray.setToolTip(QStringLiteral("Nexgen Utilities"));
  tray.setIcon(QIcon::fromTheme(QStringLiteral("applications-system")));

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

  QMenu menu;

  // Clock
  menu.addAction(QStringLiteral("Clock (Ctrl+Alt+T)"), toggleClockFn);

  // Theme
  auto* themeMenu = menu.addMenu(QStringLiteral("Theme"));
  auto* modeMenu = themeMenu->addMenu(QStringLiteral("Mode"));

  QActionGroup* modeGroup = new QActionGroup(&menu);
  modeGroup->setExclusive(true);

  auto addMode = [&](const QString& name, nexgen::themes::ThemeMode mode) {
    QAction* a = modeMenu->addAction(name);
    a->setCheckable(true);
    a->setActionGroup(modeGroup);
    a->setChecked(themes.mode() == mode);
    QObject::connect(a, &QAction::triggered, [&] {
      themes.setMode(mode);
      themes.applyTo(app);
      themes.save(settings);
      sendReloadTheme(ipc, tray);
    });
  };

  addMode(QStringLiteral("System"), nexgen::themes::ThemeMode::System);
  addMode(QStringLiteral("Light"),  nexgen::themes::ThemeMode::Light);
  addMode(QStringLiteral("Dark"),   nexgen::themes::ThemeMode::Dark);

  menu.addSeparator();
  menu.addAction(QStringLiteral("Quit"), &app, &QCoreApplication::quit);

  tray.setContextMenu(&menu);
  tray.show();

  // Hotkeys (hub owns hotkeys)
  nexgen::sys::hotkeys::HotkeyManager hotkeys;
  hotkeys.setCallback([&](int id) {
    if (id == kHotkeyClockToggle) toggleClockFn();
  });

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
