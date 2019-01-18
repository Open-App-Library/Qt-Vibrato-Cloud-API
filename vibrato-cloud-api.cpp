#include "vibrato-cloud-api.h"
#include <QApplication>

VibratoCloudAPI::VibratoCloudAPI()
{
    init();
}

VibratoCloudAPI::VibratoCloudAPI(QString token)
{
    init();
    setToken(token);
}

VibratoCloudAPI::VibratoCloudAPI(QString username, QString password)
{
    init();
    login(username, password);
}

VibratoCloudAPI::~VibratoCloudAPI()
{
    delete m_networkAccessManager;
}

bool VibratoCloudAPI::login(QString username, QString password)
{
    QString authString = QString("%1:%2").arg(username, password);
    QByteArray ba;
    ba.append(authString);

    // HTTP Basic Authententication needs to be encoded using base64
    QString base64 = ba.toBase64();

    QNetworkRequest req(QUrl("http://localhost:8000/users/login/"));
    req.setHeader(QNetworkRequest::ContentTypeHeader,
                  "application/json");
}

QString VibratoCloudAPI::token() const
{
    return m_token;
}

bool VibratoCloudAPI::setToken(QString token)
{
    // TODO: Add validation code and only set m_token if valid.
    m_token = token;
    emit tokenChanged(m_token);
}

QJsonArray VibratoCloudAPI::fetchNotes()
{
    return genericList(QUrl("/notes/"));
}

QJsonArray VibratoCloudAPI::fetchNotebooks()
{
    return genericList(QUrl("/notebooks/"));
}

QJsonArray VibratoCloudAPI::fetchTags()
{
    return genericList(QUrl("/tags/"));
}

QJsonObject VibratoCloudAPI::fetchNote(QString sync_hash)
{
    return genericRetrieve(itemEndpoint("notes", sync_hash));
}

QJsonObject VibratoCloudAPI::fetchNotebook(QString sync_hash)
{
    return genericRetrieve(itemEndpoint("notebooks", sync_hash));
}

QJsonObject VibratoCloudAPI::fetchTag(QString sync_hash)
{
    return genericRetrieve(itemEndpoint("tags", sync_hash));
}

QJsonObject VibratoCloudAPI::updateNote(QString sync_hash, QJsonObject data, bool partial)
{
    return genericUpdate(itemEndpoint("notes", sync_hash), data, partial);
}

QJsonObject VibratoCloudAPI::updateNotebook(QString sync_hash, QJsonObject data, bool partial)
{
    return genericUpdate(itemEndpoint("notebooks", sync_hash), data, partial);
}

QJsonObject VibratoCloudAPI::updateTag(QString sync_hash, QJsonObject data, bool partial)
{
    return genericUpdate(itemEndpoint("tags", sync_hash), data, partial);
}

QJsonObject VibratoCloudAPI::createNote(QJsonObject data)
{
    return genericCreate(QUrl("/notes/"), data);
}

QJsonObject VibratoCloudAPI::createNotebook(QJsonObject data)
{
    return genericCreate(QUrl("/notebooks/"), data);
}

QJsonObject VibratoCloudAPI::createTag(QJsonObject data)
{
    return genericCreate(QUrl("/tags/"), data);
}

bool VibratoCloudAPI::deleteote(QString sync_hash)
{
    return genericDelete(itemEndpoint("notes", sync_hash));
}

bool VibratoCloudAPI::deleteNotebook(QString sync_hash)
{
    return genericDelete(itemEndpoint("notebooks", sync_hash));
}

bool VibratoCloudAPI::deleteTag(QString sync_hash)
{
    return genericDelete(itemEndpoint("tags", sync_hash));
}

QString VibratoCloudAPI::encryptString(QString string)
{
    // TODO: Implement encryptString!
    return string;
}

QString VibratoCloudAPI::decryptString(QString string)
{
    // TODO: Implement decryptString!
    return string;
}

void VibratoCloudAPI::init()
{
    m_networkAccessManager = new QNetworkAccessManager();
}

QUrl VibratoCloudAPI::itemEndpoint(QString type, QString sync_hash)
{
    return QUrl(QString("/%1/%2/").arg(type, sync_hash));
}

QNetworkReply *VibratoCloudAPI::request(QString apiEndpoint, QNetworkRequest request, QString httpMethod, QString data)
{
    request.setUrl(buildPath(apiEndpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                  "application/json");

    QNetworkReply *r;

    if (httpMethod.toUpper() == "GET")
     m_networkAccessManager->get(request);
    else if (httpMethod.toUpper() == "POST")
     m_networkAccessManager->post(request, data.toUtf8());


    // The programatic equivalent to twiddling your
    // thumbs waiting for the process to complete.
    while ( r->isRunning() ) {
        qApp->processEvents();
    }
    return r;
}

/*
 * GENERIC HTTP REQUEST FUNCTIONS
 */

QJsonArray VibratoCloudAPI::genericList(QUrl apiEndpoint)
{

}

QJsonObject VibratoCloudAPI::genericRetrieve(QUrl apiEndpoint)
{

}

QJsonObject VibratoCloudAPI::genericUpdate(QUrl apiEndpoint, QJsonObject data, bool partial)
{

}

QJsonObject VibratoCloudAPI::genericCreate(QUrl apiEndpoint, QJsonObject data)
{

}

bool VibratoCloudAPI::genericDelete(QUrl apiEndpoint)
{

}
