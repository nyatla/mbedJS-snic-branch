#include "ModUrl.h"
#include "HttpdConnection.h"

namespace MiMic
{
    ModUrl::ModUrl()
    {
    }
    ModUrl::~ModUrl()
    {
    }
    bool ModUrl::execute(HttpdConnection& i_connection,char* o_url_buf,int i_buf_len,int* o_method_type)
    {
        NyLPC_TcModUrl_t mod;
        if(i_buf_len<1){
            return false;
        }
        NyLPC_cModUrl_initialize(&mod);
        if(!NyLPC_cModUrl_execute(&mod,i_connection._ref_inst,o_url_buf,i_buf_len)){
            *o_url_buf='\0';
        }
        if(o_method_type!=NULL){
            *o_method_type=NyLPC_cModUrl_getMethod(&mod);
        }
        NyLPC_cModUrl_finalize(&mod);
        return true;
    }

}