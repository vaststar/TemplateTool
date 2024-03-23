#pragma once

#ifdef MainWindow_DLL
#ifdef MainWindow_LIB
#define MainWindow_EXPORT _declspec(dllexport)
#else
#define MainWindow_EXPORT _declspec(dllimport)
#endif
#else
#define MainWindow_EXPORT 
#endif

class MainWindow_EXPORT MainWindow
{
public:
    int runMainWindow(int argc, char *argv[]);
};