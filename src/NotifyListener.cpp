#include "NotifyListener.h"

#include <QSystemTrayIcon>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>

static bool constantTimeEquals(const QByteArray& a, const QByteArray& b) {
  if (a.size() != b.size()) return false;
  uchar diff = 0;
  for (int i = 0; i < a.size(); ++i) diff |= static_cast<uchar>(a[i] ^ b[i]);
  return diff == 0;
}

NotifyListener::NotifyListener(QSystemTrayIcon* tray, Config cfg, QObject* parent)
  : QObject(parent), m_tray(tray), m_cfg(std::move(cfg)) {
  connect(&m_server, &QTcpServer::newConnection, this, &NotifyListener::onNewConnection);
}

bool NotifyListener::start(QString* errorOut) {
  if (!m_tray) {
    if (errorOut) *errorOut = QStringLiteral("Tray icon is null");
    return false;
  }
  if (m_cfg.apiKey.trimmed().isEmpty()) {
    if (errorOut) *errorOut = QStringLiteral("NotifyListener apiKey is empty");
    return false;
  }

  if (!m_server.listen(QHostAddress::AnyIPv4, m_cfg.port)) {
    if (errorOut) *errorOut = QStringLiteral("Failed to listen on port %1: %2")
      .arg(m_cfg.port)
      .arg(m_server.errorString());
    return false;
  }
  return true;
}

void NotifyListener::onNewConnection() {
  while (m_server.hasPendingConnections()) {
    auto* sock = m_server.nextPendingConnection();
    handleSocket(sock);
  }
}

// Extremely small HTTP/1.1 parser: good enough for short LAN POSTs.
void NotifyListener::handleSocket(QTcpSocket* sock) {
  sock->setParent(this);

  connect(sock, &QTcpSocket::readyRead, this, [this, sock] {
    // Read everything we currently have.
    const QByteArray data = sock->readAll();

    // Accumulate in a dynamic property (simple per-socket buffer).
    QByteArray buf = sock->property("_buf").toByteArray();
    buf += data;
    sock->setProperty("_buf", buf);

    const int headerEnd = buf.indexOf("\r\n\r\n");
    if (headerEnd < 0) return; // need more data

    const QByteArray headerBlock = buf.left(headerEnd);
    QByteArray body = buf.mid(headerEnd + 4);

    // Parse request line
    const QList<QByteArray> headerLines = headerBlock.split('\n');
    if (headerLines.isEmpty()) {
      sock->write(makeTextResponse(400, "Bad Request\n"));
      sock->disconnectFromHost();
      return;
    }

    const QByteArray requestLine = headerLines[0].trimmed();
    const QList<QByteArray> parts = requestLine.split(' ');
    if (parts.size() < 2) {
      sock->write(makeTextResponse(400, "Bad Request\n"));
      sock->disconnectFromHost();
      return;
    }

    const QByteArray method = parts[0];
    const QByteArray path = parts[1];

    // Parse headers we care about
    QByteArray apiKey;
    int contentLength = -1;

    for (int i = 1; i < headerLines.size(); ++i) {
      const QByteArray line = headerLines[i].trimmed();
      if (line.isEmpty()) continue;
      const int colon = line.indexOf(':');
      if (colon <= 0) continue;
      const QByteArray name = line.left(colon).trimmed().toLower();
      const QByteArray value = line.mid(colon + 1).trimmed();

      if (name == "x-api-key") apiKey = value;
      if (name == "content-length") contentLength = value.toInt();
    }

    // Ensure full body arrived
    if (contentLength >= 0 && body.size() < contentLength) {
      return; // wait for more
    }
    if (contentLength >= 0) body = body.left(contentLength);

    // Route
    if (path == "/health") {
      sock->write(makeTextResponse(200, "healthy\n"));
      sock->disconnectFromHost();
      return;
    }

    if (path != "/notify") {
      sock->write(makeTextResponse(404, "Not found\n"));
      sock->disconnectFromHost();
      return;
    }

    if (method != "POST") {
      sock->write(makeTextResponse(405, "Use POST\n"));
      sock->disconnectFromHost();
      return;
    }

    if (apiKey.isEmpty() || !constantTimeEquals(apiKey, m_cfg.apiKey.toUtf8())) {
      sock->write(makeTextResponse(401, "Unauthorized\n"));
      sock->disconnectFromHost();
      return;
    }

    QJsonParseError jerr;
    const auto doc = QJsonDocument::fromJson(body, &jerr);
    if (jerr.error != QJsonParseError::NoError || !doc.isObject()) {
      sock->write(makeTextResponse(400, "Invalid JSON\n"));
      sock->disconnectFromHost();
      return;
    }

    const auto obj = doc.object();
    const QString title = obj.value("title").toString();
    const QString message = obj.value("message").toString();
    const int timeoutMs = obj.value("timeoutMs").toInt(7000);

    if (title.trimmed().isEmpty() || message.trimmed().isEmpty()) {
      sock->write(makeTextResponse(400, "Payload requires: title, message\n"));
      sock->disconnectFromHost();
      return;
    }

    m_tray->showMessage(title, message, QSystemTrayIcon::Information, timeoutMs);

    sock->write(makeTextResponse(200, "OK\n"));
    sock->disconnectFromHost();
  });

  connect(sock, &QTcpSocket::disconnected, sock, &QTcpSocket::deleteLater);
}

QByteArray NotifyListener::makeTextResponse(int statusCode, const QByteArray& body) {
  QByteArray reason = "OK";
  if (statusCode == 400) reason = "Bad Request";
  else if (statusCode == 401) reason = "Unauthorized";
  else if (statusCode == 404) reason = "Not Found";
  else if (statusCode == 405) reason = "Method Not Allowed";
  else if (statusCode >= 500) reason = "Server Error";

  QByteArray resp;
  resp += "HTTP/1.1 " + QByteArray::number(statusCode) + " " + reason + "\r\n";
  resp += "Content-Type: text/plain; charset=utf-8\r\n";
  resp += "Content-Length: " + QByteArray::number(body.size()) + "\r\n";
  resp += "Connection: close\r\n";
  resp += "\r\n";
  resp += body;
  return resp;
}
