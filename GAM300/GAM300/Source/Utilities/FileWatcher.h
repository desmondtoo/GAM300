#include "Core/SystemInterface.h"

#ifndef FILEWATCHER_H
#define FILEWATCHER_H

SINGLETON(FileWatcher)
{
public:
    FileWatcher();

    void ThreadWork();

    ~FileWatcher();

private:
    void* hDir;
};

#endif