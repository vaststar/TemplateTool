#pragma once
#include <QApplication>
#include <UICore/UICoreExport.h>

namespace UICore{
class UICore_EXPORT CoreApplication: public QApplication
{
Q_OBJECT
public:
    CoreApplication(int& argc, char** argv);
};
}