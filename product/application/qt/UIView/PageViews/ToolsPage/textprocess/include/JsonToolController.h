#pragma once

#include <QObject>
#include <QtQml>
#include "UIViewBase/include/UIViewController.h"

namespace commonHead::viewModels {
    class IToolsViewModel;
}

class JsonToolController : public UIViewController
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString inputText READ getInputText WRITE setInputText NOTIFY inputTextChanged)
    Q_PROPERTY(QString outputText READ getOutputText NOTIFY outputTextChanged)
    Q_PROPERTY(QString errorMessage READ getErrorMessage NOTIFY errorMessageChanged)
    Q_PROPERTY(int indentSize READ getIndentSize WRITE setIndentSize NOTIFY indentSizeChanged)

public:
    explicit JsonToolController(QObject* parent = nullptr);

    QString getInputText() const;
    void setInputText(const QString& text);
    QString getOutputText() const;
    QString getErrorMessage() const;
    int getIndentSize() const;
    void setIndentSize(int indent);

    Q_INVOKABLE void format();
    Q_INVOKABLE void minify();
    Q_INVOKABLE void validate();
    Q_INVOKABLE void copyOutput();
    Q_INVOKABLE void clearAll();

protected:
    void init() override;

signals:
    void inputTextChanged();
    void outputTextChanged();
    void errorMessageChanged();
    void indentSizeChanged();

private:
    std::shared_ptr<commonHead::viewModels::IToolsViewModel> m_toolsViewModel;
    QString m_inputText;
    QString m_outputText;
    QString m_errorMessage;
    int m_indentSize = 2;
};
