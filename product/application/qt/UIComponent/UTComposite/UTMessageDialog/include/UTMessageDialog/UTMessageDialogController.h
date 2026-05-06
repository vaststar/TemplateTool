#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QVariantList>
#include <QtQml/qqmlregistration.h>

#include <UTComposite/UTCompositeExport.h>
#include <UTMessageDialog/UTMessageOptions.h>

// QML-facing controller for UTMessageDialog.qml.
//
// Two-phase lifecycle:
//   1. Default-constructed by QML inside the dialog (so the QML side fully
//      owns its lifetime; the controller dies with the dialog window).
//   2. C++ side fetches it via UIViewHelper::controllerOf<>() and calls
//      setOptions() exactly once before showing the window.
//
// QML reads the dialog content via the NOTIFY-backed Q_PROPERTYs; bindings
// refresh automatically when setOptions() fires optionsChanged().
class UTCOMPOSITE_EXPORT UTMessageDialogController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString      titleText    READ titleText    NOTIFY optionsChanged)
    Q_PROPERTY(QString      messageText  READ messageText  NOTIFY optionsChanged)
    Q_PROPERTY(QString      detailText   READ detailText   NOTIFY optionsChanged)
    Q_PROPERTY(int          iconKind     READ iconKind     NOTIFY optionsChanged)
    Q_PROPERTY(QVariantList buttonsModel READ buttonsModel NOTIFY optionsChanged)
    Q_PROPERTY(int          cancelIndex  READ cancelIndex  NOTIFY optionsChanged)
public:
    explicit UTMessageDialogController(QObject* parent = nullptr);
    ~UTMessageDialogController() override = default;

    // Inject options. Validates buttons (at most one isDefault/isCancel) and
    // builds the QML-facing button model. Subsequent calls are ignored with a
    // warning — the dialog is meant to be one-shot.
    void setOptions(const UTMessageOptions& options);

    QString      titleText()    const { return mOptions.title; }
    QString      messageText()  const { return mOptions.message; }
    QString      detailText()   const { return mOptions.detail; }
    int          iconKind()     const { return static_cast<int>(mOptions.icon); }
    QVariantList buttonsModel() const { return mButtonsModel; }
    int          cancelIndex()  const { return mCancelIndex; }

    // Called by QML on button click, Esc, or window close. `index` is the
    // 0-based index into buttonsModel, or -1 when no cancel button exists and
    // the window is dismissed externally. Idempotent: only the first call
    // emits closed().
    Q_INVOKABLE void accept(int index);

signals:
    void optionsChanged();
    void closed(const UTMessageResult& result);

private:
    UTMessageOptions mOptions;
    QVariantList     mButtonsModel;
    int              mCancelIndex = -1;
    bool             mOptionsSet  = false;
    bool             mFired       = false;
};
