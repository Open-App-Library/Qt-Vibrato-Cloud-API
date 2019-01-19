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

VibratoCloudAPI::VibratoCloudAPI(QString email, QString password)
{
    init();
    login(email, password);
}

VibratoCloudAPI::~VibratoCloudAPI()
{
    delete m_networkAccessManager;
}

VibratoCloudAPI::AuthenticationStatus VibratoCloudAPI::login(QString email, QString password)
{
    AuthenticationStatus returnStatus;

    QString authString = QString("%1:%2").arg(email, password);
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

    /*
     * Before returning, generate a private key based on the user's email
     * and password.
     */
    if (success) {
        returnStatus.privateKey = "THIS NEEDS TO BE IMPLEMENTED!";
    }

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

    // Do a quick test on an API endpoint to see if it works.
    QNetworkReply *test = basicRequest(buildUrl("/users/me/"), createAuthenticatedNetworkRequest());

    // Return true if success.
    if ( test->error() == QNetworkReply::NoError )
        return true;

    // Return false if error.
    qWarning() << "Newly set API key is not valid :(" << test->errorString();
    return false;
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
    // TODO: Decrypt!
    return genericAuthenticatedGet( buildUrl("/notes/") );
}

QJsonObject VibratoCloudAPI::fetchNotebooks()
{
    // TODO: Decrypt!
    return genericAuthenticatedGet( buildUrl("/notebooks/") );
}

QJsonObject VibratoCloudAPI::fetchTags()
{
    // TODO: Decrypt!
    return genericAuthenticatedGet( buildUrl("/tags/") );
}

QJsonObject VibratoCloudAPI::fetchNote(QString sync_hash)
{
    // TODO: Decrypt!
    return genericAuthenticatedGet(buildItemUrl("notes", sync_hash));
}

QJsonObject VibratoCloudAPI::fetchNotebook(QString sync_hash)
{
    // TODO: Decrypt!
    return genericAuthenticatedGet(buildItemUrl("notebooks", sync_hash));
}

QJsonObject VibratoCloudAPI::fetchTag(QString sync_hash)
{
    // TODO: Decrypt!
    return genericAuthenticatedGet(buildItemUrl("tags", sync_hash));
}

QJsonObject VibratoCloudAPI::updateNote(QString sync_hash, QJsonObject data, bool partial)
{
    // TODO: Encrypt!
    return genericAuthenticatedUpdate(buildItemUrl("notes", sync_hash), data, partial);
}

QJsonObject VibratoCloudAPI::updateNotebook(QString sync_hash, QJsonObject data, bool partial)
{
    // TODO: Encrypt!
    return genericAuthenticatedUpdate(buildItemUrl("notebooks", sync_hash), data, partial);
}

QJsonObject VibratoCloudAPI::updateTag(QString sync_hash, QJsonObject data, bool partial)
{
    // TODO: Encrypt!
    return genericAuthenticatedUpdate(buildItemUrl("tags", sync_hash), data, partial);
}

QJsonObject VibratoCloudAPI::createNote(QJsonObject data)
{
    // TODO: Encrypt!
    return genericAuthenticatedCreate(buildUrl("/notes/"), data);
}

QJsonObject VibratoCloudAPI::createNotebook(QJsonObject data)
{
    // TODO: Encrypt!
    return genericAuthenticatedCreate(buildUrl("/notebooks/"), data);
}

QJsonObject VibratoCloudAPI::createTag(QJsonObject data)
{
    // TODO: Encrypt!
    return genericAuthenticatedCreate(buildUrl("/tags/"), data);
}

bool VibratoCloudAPI::deleteNote(QString sync_hash)
{
    return genericAuthenticatedDelete(buildItemUrl("notes", sync_hash));
}

bool VibratoCloudAPI::deleteNotebook(QString sync_hash)
{
    return genericAuthenticatedDelete(buildItemUrl("notebooks", sync_hash));
}

bool VibratoCloudAPI::deleteTag(QString sync_hash)
{
    return genericAuthenticatedDelete(buildItemUrl("tags", sync_hash));
}

QJsonObject VibratoCloudAPI::encryptNote(QJsonObject note)
{
    // TODO: Implement!
    return note;
}

QJsonObject VibratoCloudAPI::encryptNotebook(QJsonObject notebook)
{
    // TODO: Implement!
    return notebook;
}

QJsonObject VibratoCloudAPI::encryptTag(QJsonObject tag)
{
    // TODO: Implement!
    return tag;
}

QJsonObject VibratoCloudAPI::decryptNote(QJsonObject note)
{
    // TODO: Implement!
    return note;
}

QJsonObject VibratoCloudAPI::decryptNotebook(QJsonObject notebook)
{
    // TODO: Implement!
    return notebook;
}

QJsonObject VibratoCloudAPI::decryptTag(QJsonObject tag)
{
    // TODO: Implement!
    return tag;
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
    return validateJsonResponse(r, "genericGet function");
}

QJsonObject VibratoCloudAPI::genericUpdate(QUrl url, QJsonObject data, bool partial, QNetworkRequest request)
{
    QString method = partial ? VIBRATO_HTTP_PATCH : VIBRATO_HTTP_PUT;
    QString json = QJsonDocument(data).toJson();
    QNetworkReply *r  = basicRequest(url, request, method, json);
    return validateJsonResponse(r, "genericUpdate function");
}

QJsonObject VibratoCloudAPI::genericCreate(QUrl url, QJsonObject data, QNetworkRequest request)
{
    QString json = QJsonDocument(data).toJson();
    QNetworkReply *r  = basicRequest(url, request, VIBRATO_HTTP_POST, json);
    return validateJsonResponse(r, "genericCreate function");
}

bool VibratoCloudAPI::genericDelete(QUrl url, QNetworkRequest request)
{
    QNetworkReply *r  = basicRequest(url, request, VIBRATO_HTTP_DELETE);
    int httpCode = r->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
    bool success = httpCode == 204;

    if (!success)
        qWarning() << QJsonDocument(validateJsonResponse(r, "genericDelete function")).toJson();

    return success;
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

QJsonObject VibratoCloudAPI::validateJsonResponse(QNetworkReply *reply, QString identifier)
{
    QString message = reply->attribute( QNetworkRequest::HttpReasonPhraseAttribute ).toString();
    int httpCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();

    if (reply->error() != QNetworkReply::NoError) {
        QJsonObject obj;
        obj.insert("detail", message);
        obj.insert("code", httpCode);
        return obj;
    }

    if ( networkReplyIsJson(reply) ) {
        QJsonDocument doc = QJsonDocument::fromJson( reply->readAll() );
        if (doc.isObject()) {
            return doc.object();
        } else {
            qWarning() << identifier << "function gave back a strange result" << doc.toJson();
            QJsonObject obj;
            obj.insert("detail", "Strange JSON object returned from server.");
            return obj;
        }
    } else {
        QJsonObject obj;
        obj.insert("detail", "Invalid response from server.");
        return obj;
    }
}
