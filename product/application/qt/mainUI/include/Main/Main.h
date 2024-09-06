#pragma once

#include <memory>
#include "MainExport.h"
class Main_EXPORT Main final
{
public:
    Main();
    ~Main();
    int runMain(int argc, char *argv[]);
public:
    Main(const Main&) = delete;
    Main(Main&&) = delete;
    Main& operator=(const Main&) = delete;
    Main& operator=(Main&&) = delete;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};