#pragma once

#include <QObject>
#include <QTemporaryDir>
#include <QUrl>

// Sample data and typed C++ dialog configuration for the standalone gallery.
class GalleryFixtures : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl folderUrl READ folderUrl CONSTANT)
    Q_PROPERTY(QUrl emptyFolderUrl READ emptyFolderUrl CONSTANT)

public:
    explicit GalleryFixtures(QObject* parent = nullptr);

    QUrl folderUrl() const { return mFolderUrl; }
    QUrl emptyFolderUrl() const { return mEmptyFolderUrl; }

    Q_INVOKABLE bool configureMessageDialog(QObject* dialog, const QString& preset);

signals:
    void messageClosed(QObject* dialog, int buttonIndex,
                       const QString& buttonText, bool accepted);

private:
    QTemporaryDir mDirectory;
    QUrl mFolderUrl;
    QUrl mEmptyFolderUrl;
};
