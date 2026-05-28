#pragma once

#include <QObject>
#include <QTcpServer>
#include <QString>

class QSystemTrayIcon;

// Minimal HTTP JSON notify listener.
// POST /notify with header X-API-Key and JSON body: {"title":"..","message":"..","timeoutMs":7000}
class NotifyListener : public QObject {
  Q_OBJECT
public:
  struct Config {
    quint16 port = 17321;
    QString apiKey; // required
  };

  explicit NotifyListener(QSystemTrayIcon* tray, Config cfg, QObject* parent = nullptr);

  bool start(QString* errorOut = nullptr);

private:
  QSystemTrayIcon* m_tray = nullptr;
  Config m_cfg;
  QTcpServer m_server;

  void onNewConnection();
  void handleSocket(QTcpSocket* sock);

  static QByteArray makeTextResponse(int statusCode, const QByteArray& body);
};
