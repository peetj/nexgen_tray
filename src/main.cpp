#include <QApplication>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QAction>
#include <QActionGroup>
#include <QSettings>
#include <QJsonObject>
#include <QCursor>

#include "nexgen/sys/hotkeys/HotkeyManager.h"
#include "nexgen/sys/ipc/IpcClient.h"

#include "nexgen/themes/ThemeManager.h"
#include "nexgen/themes/ThemeTypes.h"

static constexpr int kHotkeyClockToggle = 1001;

static QJsonObject cmd(const char* c) {
  return QJsonObject{{"cmd", QString::fromUtf8(c)}};
}

static void sendReloadTheme(nexgen::sys::ipc::IpcClient& ipc, QSystemTrayIcon& tray) {
  const auto resp = ipc.request(QStringLiteral("nexgen.clock"), cmd("reloadTheme"), 300);
  if (resp.value("ok").toBool()) {
    tray.showMessage(QStringLiteral("Theme"), QStringLiteral("Clock theme reloaded."), QSystemTrayIcon::Information, 1200);
  } else {
    tray.showMessage(QStringLiteral("Theme"), QStringLiteral("Clock not running (will apply next time)."), QSystemTrayIcon::Information, 2000);
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

  // Clock settings
  auto* clockMenu = menu.addMenu(QStringLiteral("Clock"));

  clockMenu->addAction(QStringLiteral("Move to cursor"), [&] {
    // Put the clock's top-left near the current cursor.
    const QPoint p = QCursor::pos();
    ipc.request(QStringLiteral("nexgen.clock"), QJsonObject{{"cmd", QStringLiteral("move")}, {"x", p.x()}, {"y", p.y()}}, 300);
  });

  // Size presets
  auto* sizeMenu = clockMenu->addMenu(QStringLiteral("Size"));
  QActionGroup* sizeGroup = new QActionGroup(&menu);
  sizeGroup->setExclusive(true);

  auto addSize = [&](const QString& name, const QString& preset) {
    QAction* a = sizeMenu->addAction(name);
    a->setCheckable(true);
    a->setActionGroup(sizeGroup);
    QObject::connect(a, &QAction::triggered, [&, preset] {
      ipc.request(QStringLiteral("nexgen.clock"), QJsonObject{{"cmd", QStringLiteral("setSize")}, {"preset", preset}}, 300);
    });
  };

  addSize(QStringLiteral("Tiny"), QStringLiteral("tiny"));
  addSize(QStringLiteral("Small"), QStringLiteral("small"));
  addSize(QStringLiteral("Default"), QStringLiteral("default"));
  addSize(QStringLiteral("Large"), QStringLiteral("large"));
  addSize(QStringLiteral("Very large"), QStringLiteral("very_large"));

  // Opacity presets
  auto* opacityMenu = clockMenu->addMenu(QStringLiteral("Opacity"));
  QActionGroup* opacityGroup = new QActionGroup(&menu);
  opacityGroup->setExclusive(true);

  auto addOpacity = [&](const QString& name, double v) {
    QAction* a = opacityMenu->addAction(name);
    a->setCheckable(true);
    a->setActionGroup(opacityGroup);
    QObject::connect(a, &QAction::triggered, [&, v] {
      ipc.request(QStringLiteral("nexgen.clock"), QJsonObject{{"cmd", QStringLiteral("setOpacity")}, {"value", v}}, 300);
    });
  };

  addOpacity(QStringLiteral("100%"), 1.0);
  addOpacity(QStringLiteral("85%"), 0.85);
  addOpacity(QStringLiteral("70%"), 0.70);
  addOpacity(QStringLiteral("55%"), 0.55);

  // Translucent background toggle
  QAction* translucentA = clockMenu->addAction(QStringLiteral("Translucent background"));
  translucentA->setCheckable(true);
  translucentA->setChecked(true);
  QObject::connect(translucentA, &QAction::toggled, [&, translucentA](bool on) {
    Q_UNUSED(translucentA);
    ipc.request(QStringLiteral("nexgen.clock"), QJsonObject{{"cmd", QStringLiteral("setTranslucent")}, {"on", on}}, 300);
  });

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
    // IMPORTANT: capture `mode` by value.
    // Using `[&]` here would capture the local parameter by reference, leaving a dangling
    // reference after addMode() returns (symptom: mode appears stuck at 0).
    QObject::connect(a, &QAction::triggered, [&, mode] {
      themes.setMode(mode);
      themes.applyTo(app);
      themes.save(settings);
      settings.sync();
      sendReloadTheme(ipc, tray);
      const auto dbg = ipc.request(QStringLiteral("nexgen.clock"), cmd("getThemeDebug"), 300);
      if (dbg.value("ok").toBool()) {
        const auto w = dbg.value("window").toObject();
        tray.showMessage(QStringLiteral("Clock palette"),
          QStringLiteral("mode=%1 bg rgba(%2,%3,%4,%5)")
            .arg(dbg.value("rawMode").toInt())
            .arg(w.value("r").toInt())
            .arg(w.value("g").toInt())
            .arg(w.value("b").toInt())
            .arg(w.value("a").toInt()),
          QSystemTrayIcon::Information, 2000);
      }
    });
  };

  addMode(QStringLiteral("System"), nexgen::themes::ThemeMode::System);
  addMode(QStringLiteral("Light"),  nexgen::themes::ThemeMode::Light);
  addMode(QStringLiteral("Dark"),   nexgen::themes::ThemeMode::Dark);

  themeMenu->addSeparator();
  themeMenu->addAction(QStringLiteral("Reset theme settings"), [&] {
    settings.remove(QStringLiteral("Theme"));
    settings.sync();
    themes.load(settings);
    themes.applyTo(app);
    sendReloadTheme(ipc, tray);
    tray.showMessage(QStringLiteral("Theme"), QStringLiteral("Theme settings reset."), QSystemTrayIcon::Information, 1500);
  });

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
