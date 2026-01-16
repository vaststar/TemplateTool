#pragma once

#include <QObject>
#include <QString>
#include <QtQml>

class QRCodeGenerator : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(int errorLevel READ errorLevel WRITE setErrorLevel NOTIFY errorLevelChanged)
    Q_PROPERTY(int border READ border WRITE setBorder NOTIFY borderChanged)
    Q_PROPERTY(QString darkColor READ darkColor WRITE setDarkColor NOTIFY darkColorChanged)
    Q_PROPERTY(QString lightColor READ lightColor WRITE setLightColor NOTIFY lightColorChanged)
    Q_PROPERTY(QString svgData READ svgData NOTIFY svgDataChanged)
    Q_PROPERTY(bool valid READ valid NOTIFY validChanged)

public:
    enum ErrorLevel
    {
        Low = 0,
        Medium = 1,
        Quartile = 2,
        High = 3
    };
    Q_ENUM(ErrorLevel)

    explicit QRCodeGenerator(QObject* parent = nullptr);

    QString text() const;
    void setText(const QString& text);

    int errorLevel() const;
    void setErrorLevel(int level);

    int border() const;
    void setBorder(int border);

    QString darkColor() const;
    void setDarkColor(const QString& color);

    QString lightColor() const;
    void setLightColor(const QString& color);

    QString svgData() const;
    bool valid() const;

signals:
    void textChanged();
    void errorLevelChanged();
    void borderChanged();
    void darkColorChanged();
    void lightColorChanged();
    void svgDataChanged();
    void validChanged();

private:
    void regenerate();

    QString m_text;
    int m_errorLevel;
    int m_border;
    QString m_darkColor;
    QString m_lightColor;
    QString m_svgData;
    bool m_valid;
};
