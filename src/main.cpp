#include <QApplication>
#include <QMenu>
#include <QSystemTrayIcon>

int main(int argc, char** argv) {
  QApplication app(argc, argv);
  app.setQuitOnLastWindowClosed(false);

  QSystemTrayIcon tray;
  tray.setToolTip(QStringLiteral("Nexgen Utilities"));
  tray.setIcon(QIcon::fromTheme(QStringLiteral("applications-system")));

  QMenu menu;
  auto* quit = menu.addAction(QStringLiteral("Quit"));
  QObject::connect(quit, &QAction::triggered, &app, &QCoreApplication::quit);

  tray.setContextMenu(&menu);
  tray.show();

  return app.exec();
}
