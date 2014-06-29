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
     * This class is a module for Httpd.
     * The class sends file image which is stored on ROM.
     * The class is wrapper of NyLPC_tcModRomFiles class.
     */
    class ModRomFiles:ModBaseClass
    {
    private:
        const NyLPC_TRomFileData* _ref_fsdata;
        unsigned short _num;
    public:
        /**
         * Constructor with parameter initialization.
         */
        ModRomFiles(const char* i_path,const NyLPC_TRomFileData* i_ref_fsdata,unsigned short i_num);
        /**
         * Default constructor.
         * Must be call {@link setParam} function after constructed.
         */
        ModRomFiles();
        virtual ~ModRomFiles();
        /**
         * @param i_path
         * target path
         * <pre>
         * ex.setParam("setup")
         * </pre>
         */
        void setParam(const char* i_path,const NyLPC_TRomFileData* i_ref_fsdata,unsigned short i_num);
        /**
          * This function processes a request. 
          * The function checks whether a connection has a target request.
          * If necessary, it will transmit a response.
          * @return
          * TRUE if request was processed. otherwise FALSE.
          */
        bool execute(HttpdConnection& i_connection);
    };

}