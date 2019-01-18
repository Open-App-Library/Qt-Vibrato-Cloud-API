#include <QCoreApplication>
#include <QDebug>
#include <QString>
#include "vibrato-cloud-api.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QNetworkAccessManager *internet = new QNetworkAccessManager();

    QNetworkRequest req(QUrl("http://localhost:8000/users/lofgin/"));
    req.setHeader(QNetworkRequest::ContentTypeHeader,
                  "application/json");

    QNetworkReply *r = internet->get(req);

    // The programatic equivalent to twiddling your thumbs waiting for
    // the process to complete.
    while ( r->isRunning() ) {
        app.processEvents();
    }

    QString resp = r->readAll();

    qDebug() << resp;
    qDebug() << r->error();

    QUrl myapi = QUrl("http://google.io/api");
    QUrl page = myapi;
    page.setPath("users/login");
    qDebug() << myapi.toString();
    qDebug() << page.toString();

    app.exit();
    return 0;
}
