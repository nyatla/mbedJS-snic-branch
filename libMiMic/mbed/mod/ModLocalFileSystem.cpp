#include "ModLocalFileSystem.h"
#include "HttpdConnection.h"
#include "UrlReader.h"
#include "Http.h"
#include "Httpd.h"
#include "NyLPC_net.h"
#include <stdio.h>
#include <stdlib.h>
#include <typeinfo>
#include "mbed.h"
#include "FATDirHandle.h"


using namespace MiMic;
static void retDirJson(UrlReader& url,char* buf,HttpdConnection& i_connection,unsigned char i_fs_type)
{
   //assert(HEAD or GET)
   //directory-list json
    if(!NyLPC_cHttpdUtils_sendJsonHeader((i_connection._ref_inst))){
        return;
    }
    if(!i_connection.isMethodType(Http::MT_GET)){
        return;
    }
    const char* t;
    int l;
    url.getPath(t,l);
    buf[l]='\0';//split path
    //remove '/'
    if(buf[l-1]=='/'){
        buf[l-1]='\0';
    }        
    DIR* d=opendir(buf);
    if ( d == NULL )
    {
        i_connection.sendBodyF("{\"dir\":\"%s\",\"status\":404,\"list\":[]}",buf);
        return;
    }
    if(!i_connection.isMethodType(Http::MT_GET)){
        //nothing to do
    }else{
        i_connection.sendBodyF("{\"dir\":\"%s\",\"status\":200,\"list\":[",buf);
        switch(i_fs_type){
        case ModLocalFileSystem::FST_DEFAULT:
            for(struct dirent *p= readdir(d);;)
            {
                i_connection.sendBodyF("{\"name\":\"%s\",\"mtype\":\"%s\",\"size\":undefined}",
                p->d_name,NyLPC_cMiMeType_getFileName2MimeType(p->d_name));
                p = readdir(d);
                if(p==NULL){
                    break;
                }
                i_connection.sendBodyF(",");                        
            }
            break;
        case ModLocalFileSystem::FST_SDFATFS:
            for(struct dirent *p= readdir(d);;)
            {
                bool isdir=(((struct direntFAT*)(p))->fattrib & AM_DIR)!=0;
                i_connection.sendBodyF("{\"name\":\"%s\",\"mtype\":\"%s\",\"size\":%u}",
                p->d_name,isdir?"directory":NyLPC_cMiMeType_getFileName2MimeType(p->d_name),
                isdir?0:((struct direntFAT*)(p))->fsize);
                p = readdir(d);
                if(p==NULL){
                    break;
                }
                i_connection.sendBodyF(",");                        
            }
            break;
        default:
            break;
        }
        i_connection.sendBodyF("]}");
    }
    closedir(d);
}
static void retDirHtml(UrlReader& url,char* buf,HttpdConnection& i_connection,unsigned char i_fs_type)
{
    //assert(HEAD or GET)
    buf[strlen(buf)-1]='\0';//convert to dir path
    DIR* d=opendir(buf);
    if(d==NULL){
        i_connection.sendError(403);
        if(!i_connection.isMethodType(Http::MT_GET)){
            return;
        }
        i_connection.sendBodyF("<!DOCTYPE html><html><body><h1>403 Forbidden</h1><hr/>'%s'</body></html>",buf);
        return;
    }        
    if(!i_connection.sendHeader(200,"text/html",NULL)){
        //nothing to do
    }else{
        if(!i_connection.isMethodType(Http::MT_GET)){
            //nothing to do.
        }else{
            i_connection.sendBodyF(
                "<!DOCTYPE html><html><body><h1>Index of %s</h1><hr/>\n"
                "<ul>\n"
                ,buf);
            switch(i_fs_type){
            case ModLocalFileSystem::FST_DEFAULT:
                for(struct dirent *p = readdir(d);p!=NULL;p = readdir(d))
                {
                    i_connection.sendBodyF("<li><a href=\"./%s\">%s</a></li>\n",
                    p->d_name,p->d_name);
                }
                break;
            case ModLocalFileSystem::FST_SDFATFS:
                for(struct dirent *p = readdir(d);p!=NULL;p = readdir(d))
                {
                    if((((struct direntFAT*)(p))->fattrib & AM_DIR)!=0){
                        //dir
                        i_connection.sendBodyF("<li><a href=\"./%s/\">[DIR]%s</a></li>\n",p->d_name,p->d_name);
                    }else{
                        //file
                        i_connection.sendBodyF("<li><a href=\"./%s\">%s</a></li>\n",p->d_name,p->d_name);
                    }
                }
                break;
            default:
                break;
            }
            i_connection.sendBodyF("</ul></body></html>",buf);
        }
    }
    closedir(d);
}
static void retFile(UrlReader& url,char* buf,HttpdConnection& i_connection)
{
    //file contents
    {//split URL path and query
        const char* t;
        int l;
        url.getPath(t,l);
        buf[l]='\0';
    }
    //return content
    FILE *fp;
    size_t sz;
    //size
    fp = fopen(buf, "r"); 
    if(fp==NULL){
        i_connection.sendError(404);
        if(!i_connection.isMethodType(Http::MT_GET)){
            return;
        }
        i_connection.sendBodyF("<!DOCTYPE html><html><body>'%s' not found.</body></html>",buf);
        return;
    }
    
    fseek(fp, 0, SEEK_END); // seek to end of file
    sz = ftell(fp);       // get current file pointer
    fseek(fp, 0, SEEK_SET); // seek back to beginning of file
    if(i_connection.sendHeader(200,NyLPC_cMiMeType_getFileName2MimeType(buf),NULL,sz)){
        if(!i_connection.isMethodType(Http::MT_GET)){
            //nothing to do
        }else{
            Timer t;
            t.start();
            for(;;){
                sz=fread(buf,1,Httpd::SIZE_OF_HTTP_BUF,fp);
                if(sz<1){
                    break;
                }
                if(!i_connection.sendBody(buf,sz)){
                    break;
                }
                //switch other session
                if(t.read_ms()>500){
                    //switch transport thread
                    i_connection.unlockHttpd();
                    NyLPC_cThread_sleep(50);
                    i_connection.lockHttpd();
                    t.reset();
                }
            }
        }
    }
    fclose(fp);
}    


namespace MiMic
{
    ModLocalFileSystem::ModLocalFileSystem(const char* i_path,unsigned char i_fs_type):ModBaseClass(i_path)
    {
        this->_fs_type=i_fs_type;
    }
    ModLocalFileSystem::ModLocalFileSystem():ModBaseClass()
    {
    }
    ModLocalFileSystem::~ModLocalFileSystem()
    {
    }
    void ModLocalFileSystem::setParam(const char* i_path,unsigned char i_fs_type)
    {
        ModBaseClass::setParam(i_path);
        this->_fs_type=i_fs_type;
    }
  
    bool ModLocalFileSystem::execute(HttpdConnection& i_connection)
    {
        //check platform
        //<write here! />
        
        //check prefix
        if(!this->canHandle(i_connection)){
            return false;
        }
        
        //check Method type
        {
            int mt=i_connection.getMethodType();
            if(mt!=Http::MT_GET && mt!=Http::MT_HEAD){
                //method not allowed.
                i_connection.sendError(405);
                return true;
            }
        }
        //Httpd lock
        i_connection.lockHttpd();
        char* buf=Httpd::_shared_buf;
        
        //set file path
        {
            //call ModUrl
            NyLPC_TcModUrl_t mod;
            NyLPC_cModUrl_initialize(&mod);
            if(!NyLPC_cModUrl_execute2(&mod,i_connection._ref_inst,buf,Httpd::SIZE_OF_HTTP_BUF,0,NyLPC_cModUrl_ParseMode_ALL)){
                NyLPC_cModUrl_finalize(&mod);
                i_connection.unlockHttpd();
                return true;
            }
            NyLPC_cModUrl_finalize(&mod);
        }
        UrlReader url(buf);
        if(url.hasQueryKey("list")){
            // if path has '/?list' query key,return directory information
            retDirJson(url,buf,i_connection,this->_fs_type);
        }else if(strchr(buf,'?')==NULL && strchr(buf,'#')==NULL && buf[strlen(buf)-1]=='/'){
            //return directory html when URL has not bookmark and URL query and terminated by '/'.
            retDirHtml(url,buf,i_connection,this->_fs_type);
        }else{
            retFile(url,buf,i_connection);
        }
        //Httpd unlock
        i_connection.unlockHttpd();
        return true;
        
    }
}
