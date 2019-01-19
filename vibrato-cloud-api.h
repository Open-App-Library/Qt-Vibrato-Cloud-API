#include <QObject>
#include <QString>
#include <QJsonArray>
#include <QJsonObject>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#define VIBRATO_HTTP_GET    "GET"
#define VIBRATO_HTTP_POST   "POST"
#define VIBRATO_HTTP_PUT    "PUT"
#define VIBRATO_HTTP_PATCH  "PATCH"
#define VIBRATO_HTTP_DELETE "DELETE"

class VibratoCloudAPI : QObject
{
    Q_OBJECT
public:
    /*
     * Initialization.
     * You may either not provide credentials and call the
     * login(email, password) function later. You may provide an API token.
     * Or you may provide your email and password right away. (Which simply
     * calls the login(email, password) function.
     */
    VibratoCloudAPI();
    VibratoCloudAPI(QString token);
    VibratoCloudAPI(QString email, QString password);
    ~VibratoCloudAPI();

    // Typedefs
    typedef QNetworkReply::NetworkError ResponseCode;
    typedef struct {
        bool success;
        QString errorMessage;
        ResponseCode responseCode;
        QString privateKey;
    } AuthenticationStatus;

    /*
     * Authenticates your email and password over basic authentication.
     * Logging in will return a new API token that expires in 30 days.
     * You may use the tokenChanged signal to retrieve the token that
     * is pulled down.
     * Returns true if succesfully retrieves token.
     */
    AuthenticationStatus login(QString email, QString password);

    /*
     * Getter and setter for the sync server URL.
     * If your URL has a trailing slash, the slash
     * will be removed.
     *
     * By default, this function will perform an http request to the URL.
     * A VibratoResponseCode (QNetworkReply::NetworkError) is returned.
     * You may test if the sync server is valid by comparing the returned
     * value to QNetorkReply::NoError. Full list of possible return values:
     * http://doc.qt.io/qt-5/qnetworkreply.html#NetworkError-enum
     */
    QUrl syncServerUrl() const;
    ResponseCode setSyncServerUrl(QUrl url, bool skip_test=false);

    /*
     * API token functions.
     * - token will return current token being used.
     * - setToken will of course set the token. It returns true if
     *   the token is valid. It also emits the tokenChanged if the
     *   token is valid.
     */
    QString token() const;
    bool setToken(QString token);

    /*
     * buildUrl:
     * Appends a path to the sync server URL.
     *
     * buildItemUrl:
     * Build a URL to an item.
     * Example #1:
     * buildItemUrl("notes", "xxxx-xxxx-xxxx"); ==> "{sync server URL}/notes/xxxx-xxxx-xxxx/
     * Example #2:
     * buildItemUrl("foo/bar", "loremipsum"); ==> "{sync server URL}/foo/bar/loremipsum/
     */
    QUrl buildUrl(QString path);
    QUrl buildItemUrl(QString type, QString sync_hash);


    /*
     * Using the API token, you may fetch JSON objects.
     * Note that you fetch objects with the sync_hash rather
     * than the "id". Think of sync_hashes as the unique
     * fingerprint of your note, notebook or tag.
     */
    QJsonObject fetchNotes();
    QJsonObject fetchNotebooks();
    QJsonObject fetchTags();

    QJsonObject fetchNote(QString sync_hash);
    QJsonObject fetchNotebook(QString sync_hash);
    QJsonObject fetchTag(QString sync_hash);

    /*
     * Functions for updating individual notes, notebooks and tags.
     * 'data' is the new data that you are updating the note with.
     * If 'partial' is set to true , an http PATCH will be sent. (default)
     * If 'partial' is set to false, an http PUT will be sent
     */
    QJsonObject updateNote(QString sync_hash, QJsonObject data, bool partial=true);
    QJsonObject updateNotebook(QString sync_hash, QJsonObject data, bool partial=true);
    QJsonObject updateTag(QString sync_hash, QJsonObject data, bool partial=true);

    QJsonObject createNote(QJsonObject data);
    QJsonObject createNotebook(QJsonObject data);
    QJsonObject createTag(QJsonObject data);

    bool deleteNote(QString sync_hash);
    bool deleteNotebook(QString sync_hash);
    bool deleteTag(QString sync_hash);

    /*
     * Cryptography functions.
     * These will be called internally when sending and recieving objects.
     * The encryption key is generated using a combination of the user's
     * email and password.
     */
    QString     encryptString(QString string);
    QString     decryptString(QString string);
    QJsonObject encryptNote(QJsonObject note);
    QJsonObject encryptNotebook(QJsonObject notebook);
    QJsonObject encryptTag(QJsonObject tag);
    QJsonObject decryptNote(QJsonObject note);
    QJsonObject decryptNotebook(QJsonObject notebook);
    QJsonObject decryptTag(QJsonObject tag);

signals:
    void tokenChanged(QString apiToken);

private:
    // Initialization function called at the beginning of all constructors.
    void init();

    QUrl m_syncServerUrl = QUrl("https://api.vibrato.app");
    QString m_token;

    /*
     * Qt Request Objects
     */
    QNetworkAccessManager *m_networkAccessManager;

    // Generic HTTP request functions
    QNetworkReply *basicRequest(QUrl url, QNetworkRequest request=QNetworkRequest(), QString httpMethod="GET", QString data="");
    QJsonObject   genericGet(QUrl url, QNetworkRequest request=QNetworkRequest());
    QJsonObject   genericUpdate(QUrl url, QJsonObject data, bool partial=true, QNetworkRequest request=QNetworkRequest());
    QJsonObject   genericCreate(QUrl url, QJsonObject data, QNetworkRequest request=QNetworkRequest());
    bool          genericDelete(QUrl url, QNetworkRequest request=QNetworkRequest());

    QNetworkRequest createAuthenticatedNetworkRequest(QNetworkRequest request=QNetworkRequest());
    QJsonObject   genericAuthenticatedGet(QUrl url, QNetworkRequest request=QNetworkRequest());
    QJsonObject   genericAuthenticatedUpdate(QUrl url, QJsonObject data, bool partial=true, QNetworkRequest request=QNetworkRequest());
    QJsonObject   genericAuthenticatedCreate(QUrl url, QJsonObject data, QNetworkRequest request=QNetworkRequest());
    bool          genericAuthenticatedDelete(QUrl url, QNetworkRequest request=QNetworkRequest());

    /*
     * Helpful little functions.
     * networkReplyIsJson simply checks the content type of a networki reply to ensure it is 'application/json'.
     *
     * validateJsonResponse processes a reply and returns a QJsonObject of either the returned object or an error
     * as a QJsonObject.
     */
    bool networkReplyIsJson(const QNetworkReply *reply);
    QJsonObject validateJsonResponse(QNetworkReply *reply, QString identifier="Request");
};
