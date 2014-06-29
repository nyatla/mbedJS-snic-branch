#include <stdio.h>
#include "NyLPC_cMimeType.h"
struct TMimeTypeTable{
    const char* ext;
    const char* mimetype;
};
const static struct TMimeTypeTable table[]=
{
    {"zip" ,"application/zip"},
    {"js"  ,"application/x-javascript"},
    {"txt" ,"text/plain"},
    {"html","text/html"},
    {"htm","text/html"},    
    {"css" ,"text/css"},
    {"jpeg","image/jpeg"},
    {"jpg" ,"image/jpeg"},
    {"png" ,"image/png"},
    {"gif" ,"image/gif"},
    {NULL,NULL}
};
const static char* default_mimetype="application/octet-stream";

const char* NyLPC_cMiMeType_getFileName2MimeType(const char* i_file_name)
{
    int i;
    const char* p=strrchr(i_file_name,'.');
    if(p==NULL){
        return default_mimetype;
    }
    for(i=0;table[i].ext!=NULL;i++){
        if(NyLPC_stricmp(table[i].ext,p+1)==0){
            return table[i].mimetype;
        }
    }
    return default_mimetype;
}
