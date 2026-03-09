#pragma once

#include <QObject>
#include <QtQml>
#include "UIViewBase/include/UIViewController.h"

namespace commonHead::viewModels {
    class IToolsViewModel;
}

class UuidToolController : public UIViewController
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString generatedUuid READ getGeneratedUuid NOTIFY generatedUuidChanged)
    Q_PROPERTY(QString validateInput READ getValidateInput WRITE setValidateInput NOTIFY validateInputChanged)
    Q_PROPERTY(QString validateResult READ getValidateResult NOTIFY validateResultChanged)
    Q_PROPERTY(QStringList uuidHistory READ getUuidHistory NOTIFY uuidHistoryChanged)

public:
    explicit UuidToolController(QObject* parent = nullptr);

    QString getGeneratedUuid() const;
    QString getValidateInput() const;
    void setValidateInput(const QString& text);
    QString getValidateResult() const;
    QStringList getUuidHistory() const;

    Q_INVOKABLE void generate();
    Q_INVOKABLE void generateMultiple(int count);
    Q_INVOKABLE void validate();
    Q_INVOKABLE void copyUuid();
    Q_INVOKABLE void copyHistory();
    Q_INVOKABLE void clearHistory();

protected:
    void init() override;

signals:
    void generatedUuidChanged();
    void validateInputChanged();
    void validateResultChanged();
    void uuidHistoryChanged();

private:
    std::shared_ptr<commonHead::viewModels::IToolsViewModel> m_toolsViewModel;
    QString m_generatedUuid;
    QString m_validateInput;
    QString m_validateResult;
    QStringList m_uuidHistory;
};
