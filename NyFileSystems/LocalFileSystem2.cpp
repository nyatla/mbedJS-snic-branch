#include "LocalFileSystem2.h"
#include "utils/PlatformInfo.h" 
namespace MiMic
{
#ifdef TARGET_LPC1768
    /**
     * This module is LocalFileSystem class which is not stopped on LPCXpresso.
     * It uses instead of LocalFileSystem. 
     */
    LocalFileSystem2::LocalFileSystem2(const char* n) : LocalFileSystem(n)
    {
        this->_is_enable=(PlatformInfo::getPlatformType()==PlatformInfo::PF_MBED);
    }
    FileHandle *LocalFileSystem2::open(const char* name, int flags)
    {
        return this->_is_enable?LocalFileSystem::open(name,flags):NULL;
    }
    int LocalFileSystem2::remove(const char *filename)
    {
        return this->_is_enable?LocalFileSystem::remove(filename):-1;
    }
    DirHandle *LocalFileSystem2::opendir(const char *name)
    {
        return this->_is_enable?LocalFileSystem::opendir(name):NULL;
    }
#else

#endif  
}
