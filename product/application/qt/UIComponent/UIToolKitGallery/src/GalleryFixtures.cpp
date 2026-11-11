#include "GalleryFixtures.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QHash>
#include <QPointer>
#include <QStringList>
#include <QVariant>

#include <UTMessageDialog/UTMessageDialogController.h>
#include <UTMessageDialog/UTMessagePresets.h>

GalleryFixtures::GalleryFixtures(QObject* parent)
    : QObject(parent)
    , mDirectory(QDir::tempPath() + QStringLiteral("/UIToolKitGallery-XXXXXX"))
{
    if (!mDirectory.isValid())
    {
        qWarning() << "Gallery: could not create the sample directory";
        return;
    }

    const QDir directory(mDirectory.path());
    if (!directory.mkdir(QStringLiteral("documents"))
        || !directory.mkdir(QStringLiteral("empty")))
    {
        qWarning() << "Gallery: could not create the sample folders";
        return;
    }

    const QDir documents(directory.filePath(QStringLiteral("documents")));
    const QStringList names = {
        QStringLiteral("README.md"),
        QStringLiteral("Release checklist.txt"),
        QStringLiteral("Keyboard navigation and document shortcuts.md"),
        QStringLiteral("Design brief.md"),
        QStringLiteral("Research notes.txt"),
        QStringLiteral("Roadmap.md"),
        QStringLiteral("Meeting agenda.txt"),
        QStringLiteral("Team guide.md"),
        QStringLiteral("Draft outline.txt"),
        QStringLiteral("Project summary.md"),
        QStringLiteral("Changelog.txt"),
        QStringLiteral("Archive index.md")
    };
    const QHash<QString, QByteArray> sampleContents = {
        {QStringLiteral("README.md"),
         "Document library\n\nUse the grid to browse documents, or switch to the detail view to compare names, modification dates, sizes, and file types."},
        {QStringLiteral("Release checklist.txt"),
         "Release checklist\n\nReview the updated guides. Check the document names. Share the release notes with the project team."},
        {QStringLiteral("Keyboard navigation and document shortcuts.md"),
         "Keyboard navigation\n\nUse the arrow keys to move between documents. Press Enter or Space to open the document menu, then choose Open to read the document."}
    };
    for (const auto& name : names)
    {
        QFile file(documents.filePath(name));
        const QByteArray content = sampleContents.value(name,
            name.toUtf8() + "\n\nA sample document in the Gallery library.");
        if (!file.open(QIODevice::WriteOnly) || file.write(content) != content.size())
        {
            qWarning() << "Gallery: could not write sample document" << name;
            return;
        }
    }

    mFolderUrl = QUrl::fromLocalFile(documents.absolutePath());
    mEmptyFolderUrl = QUrl::fromLocalFile(directory.filePath(QStringLiteral("empty")));
}

bool GalleryFixtures::configureMessageDialog(QObject* dialog, const QString& preset)
{
    auto* controller = dialog
        ? qobject_cast<UTMessageDialogController*>(dialog->property("controller").value<QObject*>())
        : nullptr;
    if (!controller)
    {
        qWarning() << "Gallery: message dialog has no UTMessageDialogController";
        return false;
    }

    UTMessageOptions options;
    if (preset == QStringLiteral("info"))
    {
        options = UTMessagePresets::info(tr("Report ready"), tr("Your sample report is ready."),
                                        tr("The report is available in the project workspace."));
    }
    else if (preset == QStringLiteral("confirm"))
    {
        options = UTMessagePresets::okCancel(tr("Change notification preference?"),
                                            tr("Apply the new notification preference?"),
                                            tr("This changes the sample preference shown in the Gallery."));
    }
    else if (preset == QStringLiteral("reset"))
    {
        options = UTMessagePresets::destructiveConfirm(tr("Reset notification preference?"),
                                                       tr("Restore the sample notification preference?"),
                                                       tr("Reset"), tr("Notifications will be turned off."));
    }
    else
    {
        qWarning() << "Gallery: unknown message preset" << preset;
        return false;
    }

    // The Case creates a new window for each use, matching the controller's
    // configure-once lifecycle. Keep the result accessible to QML without
    // changing the production dialog's typed UTMessageResult API.
    controller->setOptions(options);
    connect(controller, &UTMessageDialogController::closed, this,
            [this, guardedDialog = QPointer<QObject>(dialog)](const UTMessageResult& result) {
                if (guardedDialog)
                {
                    const bool accepted = result.buttonIndex >= 0
                        && (result.role == UTButtonRole::Accept || result.role == UTButtonRole::Destructive);
                    emit messageClosed(guardedDialog.data(), result.buttonIndex, result.buttonText, accepted);
                }
            });
    return true;
}
