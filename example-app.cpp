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


    VibratoCloudAPI api;

    VibratoCloudAPI::ResponseCode err = api.setSyncServerUrl(QUrl("http://localhost:8000"));
    qDebug() << "Sync Server Status:" << err;

    qDebug("Logging in");
    api.login("test-user", "vibratonotes");

    QJsonObject obj;
    obj.insert("title", "Hello C++");
    qDebug() << api.createNote(obj);

    app.exit();
    return 0;
}
