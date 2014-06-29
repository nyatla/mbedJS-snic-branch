#pragma once
////////////////////////////////////////////////////////////////////////////////
// ModRomFiles.h
////////////////////////////////////////////////////////////////////////////////

#include "NyLPC_net.h"
#include "ModBaseClass.h"


namespace MiMic
{
    class HttpdConnection;

    /**
     * This class is httpd module.
     * The class provide files on mbed LocalFile System.
     * The class provide 2 services.
     * <ul>
     * <li>lfile content provider.</li>
     * This is response of local file path. for example "/local/file.txt". full content of file.
     * <li>file list provider</li>
     * This is array of json response. for example, "/local/"
     * [{name:"name",size:"size",type:"type"}]
     * </ul>
     */
    class ModLocalFileSystem:ModBaseClass
    {
    private:
        /** file system type*/
        unsigned char _fs_type;
    public:
        const static unsigned char FST_DEFAULT=0x00;
        const static unsigned char FST_SDFATFS=0x01;
    public:
        /**
         * @param i_fs_type
         * Filesystem type.
         * This value should match the file system type of mount point.
         * <ul>
         * <li>FST_DEFAULT - default filesystem(eg. mbed local.)</li>         
         * <li>FST_SDFATFS - for SD filesystem</li>
         * </ul>
         */
        ModLocalFileSystem(const char* i_path,unsigned char i_fs_type=FST_DEFAULT);
        ModLocalFileSystem();
        virtual ~ModLocalFileSystem();
        void setParam(const char* i_path,unsigned char i_fs_type=FST_DEFAULT);
        bool execute(HttpdConnection& i_connection);
    };

}