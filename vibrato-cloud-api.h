#include <QObject>
#include <QString>
#include <QJsonArray>
#include <QJsonObject>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class VibratoCloudAPI : QObject
{
    Q_OBJECT
public:
    /*
     * Initialization.
     * You may either not provide credentials and call the
     * login(username, password) function later. You may provide an API token.
     * Or you may provide your username and password right away. (Which simply
     * calls the login(username, password) function.
     */
    VibratoCloudAPI();
    VibratoCloudAPI(QString token);
    VibratoCloudAPI(QString username, QString password);

    ~VibratoCloudAPI();

    /*
     * Authenticates your username and password over basic authentication.
     * Logging in will return a new API token that expires in 30 days.
     * You may use the tokenChanged signal to retrieve the token that
     * is pulled down.
     * Returns true if succesfully retrieves token.
     */
    bool login(QString username, QString password);

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
     * Easy way to get
     */
    QUrl buildPath(QString path);

    /*
     * Using the API token, you may fetch JSON objects.
     * Note that you fetch objects with the sync_hash rather
     * than the "id". Think of sync_hashes as the unique
     * fingerprint of your note, notebook or tag.
     */
    QJsonArray fetchNotes();
    QJsonArray fetchNotebooks();
    QJsonArray fetchTags();

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

    bool deleteote(QString sync_hash);
    bool deleteNotebook(QString sync_hash);
    bool deleteTag(QString sync_hash);

    /*
     * Cryptography functions.
     * These will be called internally when sending and recieving objects.
     * The encryption key is generated using a combination of the user's
     * email and password.
     */
    QString encryptString(QString string);
    QString decryptString(QString string);

signals:
    void tokenChanged(QString apiToken);

private:
    // Initialization function called at the beginning of all constructors.
    void init();

    QUrl m_syncServerURL = QUrl("https://api.vibrato.app");
    QString m_token;

    /*
     * Qt Request Objects
     */
    QNetworkAccessManager *m_networkAccessManager;

    /*
     * Useful function to build item API endpoints.
     * Example:
     * itemEndpoint("notes", "xxxx-xxxx-xxxx"); ==> "/notes/xxxx-xxxx-xxxx/
     */
    QUrl itemEndpoint(QString type, QString sync_hash);

    // Generic HTTP request functions
    QNetworkReply *request(QString apiEndpoint, QNetworkRequest request=QNetworkRequest(), QString httpMethod="GET", QString data="");
    QJsonArray    genericList(QString apiEndpoint);
    QJsonObject   genericRetrieve(QUrl apiEndpoint);
    QJsonObject   genericUpdate(QString apiEndpoint, QJsonObject data, bool partial=true);
    QJsonObject   genericCreate(QString apiEndpoint, QJsonObject data);
    bool          genericDelete(QString apiEndpoint);
};
