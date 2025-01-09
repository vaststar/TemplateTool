#pragma once 

#include <QObject>
#include <QColor>
#include <QString>
#include <QtQml>
#include <UIDataStruct/UIDataStructExport.h>

namespace UIData{


struct UIDataStruct_EXPORT UIColorSet: public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("only created in c++ code")
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(QColor normal MEMBER normal)
    Q_PROPERTY(QColor hovered MEMBER hovered)
    Q_PROPERTY(TESTFont font MEMBER font)

public:

    enum class TESTFont{
        AAA
    };
    Q_ENUM(TESTFont)
public:
    UIColorSet(QObject *parent = nullptr);
    UIColorSet(const QString& name, const QColor& normal);
    UIColorSet(const QString& name, const QColor& normal, const QString& hovered);
private:
    QString name;
    QColor normal;
    QColor hovered;
    TESTFont font{TESTFont::AAA};
};
}