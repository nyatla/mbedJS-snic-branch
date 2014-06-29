#pragma once

 
#include "mbed.h"
#include "FATFileSystem.h"
namespace MiMic
{
    /**
     * This module is LocalFileSystem class which is not stopped on LPCXpresso.
     * It uses instead of LocalFileSystem. 
     */
#ifdef TARGET_LPC1768
    class LocalFileSystem2 : public LocalFileSystem
    {
    private:
        bool _is_enable;
    public:
        LocalFileSystem2(const char* n);
        virtual FileHandle *open(const char* name, int flags);
        virtual int remove(const char *filename);
        virtual DirHandle *opendir(const char *name);
    };
#else
    class LocalFileSystem2 : public FileSystemLike
    {
    public:
        LocalFileSystem2(const char* n):FileSystemLike(n){}
        virtual FileHandle *open(const char *filename, int flags){return NULL;}
    };
#endif
}

