#include "vibrato-cloud-api.h"
#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>

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

VibratoCloudAPI::AuthenticationStatus VibratoCloudAPI::login(QString username, QString password)
{
    AuthenticationStatus returnStatus;

    QString authString = QString("%1:%2").arg(username, password);
    QByteArray ba;
    ba.append(authString);

    // HTTP Basic Authententication needs to be encoded using base64
    QString base64 = ba.toBase64();

    QNetworkRequest req;
    req.setHeader(QNetworkRequest::ContentTypeHeader,
                  "application/json");
    req.setRawHeader("Authorization",
                     QString("Basic %1").arg(base64).toLocal8Bit());
    QNetworkReply *reply = basicRequest(buildUrl("/users/login/"), req, VIBRATO_HTTP_POST);

    // Set some variables based on the reply
    QString errorMessage = "";
    ResponseCode status = reply->error();
    bool success = status == QNetworkReply::NoError;
    bool response_is_json = networkReplyIsJson(reply);

    // Parse the response to JSON
    QString response = reply->readAll();
    QJsonDocument jsonDoc;
    if (response_is_json)
        jsonDoc = QJsonDocument::fromJson(response.toUtf8());
    else
        jsonDoc = QJsonDocument::fromJson("{ \"detail\": \"Invalid response type from server.\"");

    // Set API token if valid. Otherwise print an error message.
    if (success) {
        QString token;
        QJsonValue tokenVal = jsonDoc.object().value("token");
        if (tokenVal == QJsonValue::Undefined) {
            // INVALID SUCCESS RESPONSE
            success = false;
            errorMessage = "Invalid response from the server";
        } else {
            // GOT THE TOKEN!
            token = tokenVal.toString();
            m_token = token;
        }
    }
    else if (!success) {
        QJsonValue detailVal = jsonDoc.object().value("detail");
        if (detailVal == QJsonValue::Undefined)
            errorMessage = "Unknown error";
        else
            errorMessage = detailVal.toString();
        qWarning() << "Login failed" << errorMessage;
    }

    returnStatus.success = success;
    returnStatus.errorMessage = errorMessage;
    returnStatus.responseCode = status;
    return returnStatus;
}

QUrl VibratoCloudAPI::syncServerUrl() const
{
    return m_syncServerUrl;
}

VibratoCloudAPI::ResponseCode VibratoCloudAPI::setSyncServerUrl(QUrl url, bool skip_test)
{
    // Removing trailing slash if exists
    QString pathString = url.path();
    if ( pathString == "/" ) {
        url.setPath("");
    } else if ( pathString.length() > 0 &&
         pathString[pathString.length()-1] == "/" ) {
        pathString.chop(1);
        url.setPath(pathString);
    }

    m_syncServerUrl = url;

    if ( skip_test )
        return QNetworkReply::NoError;

    // Make a basic GET request and return the status code.
    QNetworkReply *test200 = basicRequest(buildUrl("/"));
    return test200->error();
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

QUrl VibratoCloudAPI::buildUrl(QString path)
{
    // If empty, just return the syncServerURL
    if ( path.length() == 0 )
        return m_syncServerUrl;

    // If the first character of the path is not a slash, add one
    if ( path[0] != '/' )
        path.prepend('/');

    // Return the sync server url with the path appended
    return QUrl( m_syncServerUrl.toString().append(path) );
}

QUrl VibratoCloudAPI::buildItemUrl(QString type, QString sync_hash)
{
    return buildUrl( QString("/%1/%2/").arg(type, sync_hash) );
}


QJsonObject VibratoCloudAPI::fetchNotes()
{
    return genericGet( buildUrl("/notes/") );
}

QJsonObject VibratoCloudAPI::fetchNotebooks()
{
    return genericGet( buildUrl("/notebooks/") );
}

QJsonObject VibratoCloudAPI::fetchTags()
{
    return genericGet( buildUrl("/tags/") );
}

QJsonObject VibratoCloudAPI::fetchNote(QString sync_hash)
{
    return genericGet(buildItemUrl("notes", sync_hash));
}

QJsonObject VibratoCloudAPI::fetchNotebook(QString sync_hash)
{
    return genericGet(buildItemUrl("notebooks", sync_hash));
}

QJsonObject VibratoCloudAPI::fetchTag(QString sync_hash)
{
    return genericGet(buildItemUrl("tags", sync_hash));
}

QJsonObject VibratoCloudAPI::updateNote(QString sync_hash, QJsonObject data, bool partial)
{
    return genericUpdate(buildItemUrl("notes", sync_hash), data, partial);
}

QJsonObject VibratoCloudAPI::updateNotebook(QString sync_hash, QJsonObject data, bool partial)
{
    return genericUpdate(buildItemUrl("notebooks", sync_hash), data, partial);
}

QJsonObject VibratoCloudAPI::updateTag(QString sync_hash, QJsonObject data, bool partial)
{
    return genericUpdate(buildItemUrl("tags", sync_hash), data, partial);
}

QJsonObject VibratoCloudAPI::createNote(QJsonObject data)
{
    return genericCreate(buildUrl("/notes/"), data);
}

QJsonObject VibratoCloudAPI::createNotebook(QJsonObject data)
{
    return genericCreate(buildUrl("/notebooks/"), data);
}

QJsonObject VibratoCloudAPI::createTag(QJsonObject data)
{
    return genericCreate(buildUrl("/tags/"), data);
}

bool VibratoCloudAPI::deleteNote(QString sync_hash)
{
    return genericDelete(buildItemUrl("notes", sync_hash));
}

bool VibratoCloudAPI::deleteNotebook(QString sync_hash)
{
    return genericDelete(buildItemUrl("notebooks", sync_hash));
}

bool VibratoCloudAPI::deleteTag(QString sync_hash)
{
    return genericDelete(buildItemUrl("tags", sync_hash));
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

QNetworkReply *VibratoCloudAPI::basicRequest(QUrl url, QNetworkRequest request, QString httpMethod, QString data)
{
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                  "application/json");

    QNetworkReply *r;

    QString meth = httpMethod.toUpper().trimmed();
    if (meth == VIBRATO_HTTP_POST)
        r = m_networkAccessManager->post(request, data.toUtf8());
    else if (meth == VIBRATO_HTTP_PUT)
        r = m_networkAccessManager->put(request, data.toUtf8());
    else if (meth == VIBRATO_HTTP_PATCH)
        // TODO: DO PATCH THE PROPER WAY!
        r = m_networkAccessManager->put(request, data.toUtf8());
    else if (meth == VIBRATO_HTTP_DELETE)
        r = m_networkAccessManager->deleteResource(request);
    else {
        if (meth != VIBRATO_HTTP_GET) qWarning()
                << "You requested an invalid HTTP method"
                << httpMethod
                << "...Instead a GET method will be used.";
        r = m_networkAccessManager->get(request);
    }

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

QJsonObject VibratoCloudAPI::genericGet(QUrl url, QNetworkRequest request)
{
    QNetworkReply *r  = basicRequest(url, request);

    if ( networkReplyIsJson(r) ) {
        QJsonDocument doc = QJsonDocument::fromJson( r->readAll() );
        if (doc.isObject()) {
            return doc.object();
        } else {
            qWarning() << "genericList function gave back a strange result" << doc.toJson();
            QJsonObject obj;
            obj.insert("detail", "Invalid list returned from server.");
            return obj;
        }
    } else {
        QJsonObject obj;
        obj.insert("detail", "Invalid response from server.");
        return obj;
    }
}

QJsonObject VibratoCloudAPI::genericUpdate(QUrl url, QJsonObject data, bool partial, QNetworkRequest request)
{

}

QJsonObject VibratoCloudAPI::genericCreate(QUrl url, QJsonObject data, QNetworkRequest request)
{

}

bool VibratoCloudAPI::genericDelete(QUrl url, QNetworkRequest request)
{

}

QNetworkRequest VibratoCloudAPI::createAuthenticatedNetworkRequest(QNetworkRequest request)
{
    request.setRawHeader("Authorization",
                         QString("Token %1").arg(m_token).toLocal8Bit());
    return request;
}

QJsonObject VibratoCloudAPI::genericAuthenticatedGet(QUrl url, QNetworkRequest request)
{
    return genericGet(url, createAuthenticatedNetworkRequest(request));
}

QJsonObject VibratoCloudAPI::genericAuthenticatedUpdate(QUrl url, QJsonObject data, bool partial, QNetworkRequest request)
{
    return genericUpdate(url, data, partial, createAuthenticatedNetworkRequest(request));
}

QJsonObject VibratoCloudAPI::genericAuthenticatedCreate(QUrl url, QJsonObject data, QNetworkRequest request)
{
    return genericCreate(url, data, createAuthenticatedNetworkRequest(request));
}

bool VibratoCloudAPI::genericAuthenticatedDelete(QUrl url, QNetworkRequest request)
{
    return genericDelete(url, createAuthenticatedNetworkRequest(request));
}

bool VibratoCloudAPI::networkReplyIsJson(const QNetworkReply *reply)
{
    return
        reply->header(QNetworkRequest::ContentTypeHeader).toString()
        ==
        "application/json";
}
