#pragma once
#include <QApplication>
#include <UICore/UICoreExport.h>

class UICore_EXPORT CoreApplication: public QApplication
{
Q_OBJECT
public:
    CoreApplication(int& argc, char** argv);
};