#pragma once

#include <QObject>
#include <QtQml>
#include "UIViewBase/include/UIViewController.h"

namespace commonHead::viewModels {
    class IToolsViewModel;
}

class Base64ToolController : public UIViewController
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString inputText READ getInputText WRITE setInputText NOTIFY inputTextChanged)
    Q_PROPERTY(QString outputText READ getOutputText NOTIFY outputTextChanged)
    Q_PROPERTY(QString errorMessage READ getErrorMessage NOTIFY errorMessageChanged)
    Q_PROPERTY(bool urlSafe READ isUrlSafe WRITE setUrlSafe NOTIFY urlSafeChanged)

public:
    explicit Base64ToolController(QObject* parent = nullptr);

    QString getInputText() const;
    void setInputText(const QString& text);
    QString getOutputText() const;
    QString getErrorMessage() const;
    bool isUrlSafe() const;
    void setUrlSafe(bool urlSafe);

    Q_INVOKABLE void encode();
    Q_INVOKABLE void decode();
    Q_INVOKABLE void swapInputOutput();
    Q_INVOKABLE void copyOutput();
    Q_INVOKABLE void clearAll();

protected:
    void init() override;

signals:
    void inputTextChanged();
    void outputTextChanged();
    void errorMessageChanged();
    void urlSafeChanged();

private:
    std::shared_ptr<commonHead::viewModels::IToolsViewModel> m_toolsViewModel;
    QString m_inputText;
    QString m_outputText;
    QString m_errorMessage;
    bool m_urlSafe = false;
};
