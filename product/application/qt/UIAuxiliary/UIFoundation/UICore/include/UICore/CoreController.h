#pragma once

#include <QObject>

#include <UICore/UICoreExport.h>

namespace UICore{
class UICore_EXPORT CoreController: public QObject
{
    Q_OBJECT
public:
    CoreController(QObject *parent = nullptr);
};
}