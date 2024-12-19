#pragma once

#include <QObject>
#include <QtQml>
#include <QFont>

#include <include/UIComponentBaseExport.h>

class UIComponentBase_EXPORT FontConstants: public QObject
{
Q_OBJECT
    Q_PROPERTY(QFont bodySecondary READ getBodySecondary CONSTANT)
QML_ELEMENT
public:
    explicit FontConstants(QObject* parent = nullptr);
    QFont getBodySecondary() const;
};