Vibrato API Client Library in Qt
---

This library provides a Qt/C++ interface to the [Vibrato Notes](https://vibrato.app) cloud API.

**Example usage**:

```c++
VibratoCloudAPI api;

/*
 * Note: While this function uses your username and password,
 *       it does not actually store them on the computer.
 *       Your username and password are used in HTTP Basic
 *       Authentication to retrieve and API token. This token
 *       is what is used to make requests. This library does
 *       not store your API key on your computer for you.
 *       That is up to you.
 */
api.login("test-user", "vibratonotes");

QJsonObject obj;
obj.insert("title", "Hello from C++!");
qDebug() << api.createNote(obj);

QJsonObject my_notes = api.fetchNotes();
```