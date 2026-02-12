#pragma once

#include <QObject>
#include <QtQml>

#include <UTComponent/UTComponentExport.h>

class UTCOMPONENT_EXPORT UTQRCodeLevel : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    enum Level {
        Low = 0,
        Medium = 1,
        Quartile = 2,
        High = 3
    };
    Q_ENUM(Level)

    explicit UTQRCodeLevel(QObject* parent = nullptr);
};
