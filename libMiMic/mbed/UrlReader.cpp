#include "NyLPC_http.h"
#include "NyLPC_stdlib.h"
#include "mbed.h"
#include "UrlReader.h"
namespace MiMic
{
    UrlReader::UrlReader(const char* i_ref_url)
    {
        this->_ref_str=i_ref_url;
    }
    bool UrlReader::getPath(const char* &path,int &l)
    {
        return NyLPC_cUrlReader_getPath(this->_ref_str,&path,(NyLPC_TInt32*)&l);
    }
    bool UrlReader::isPathEqual(const char* path)
    {
        const char* p;
        int l;
        if(!NyLPC_cUrlReader_getPath(this->_ref_str,&p,(NyLPC_TInt32*)&l)){
            return false;
        }
        return ((l==strlen(path)) && (strncmp(p,path,l)==0));
    }
    /**
     * @param i_ref_str
     *  URL text for read.
     *  This is referenced pointer. Must hold it until an instance is closed.
     */
    void UrlReader::setUrl(const char* i_ref_url)
    {
        this->_ref_str=i_ref_url;
    }
    //bool hasHost(const char* key);
    //bool getHost(const char* i_ref_url);
    //bool hasPath(const char* i_ref_url);
    //bool getPath(const char* i_ref_url);
    /**
     * This function confirms URL has a query key.
     * @param key
     * key name.
     */
    bool UrlReader::hasQueryKey(const char* key)
    {
        return NyLPC_cUrlReader_findKeyValue(this->_ref_str,key)!=NULL;
    }
    bool UrlReader::getQueryStr(const char* key,const char* &o_ref_val,int &o_val_len)
    {
        return NyLPC_cUrlReader_getStr(this->_ref_str,key,&o_ref_val,(NyLPC_TInt32*)&o_val_len)==NyLPC_TBool_TRUE;
    }
    bool UrlReader::getQueryUInt(const char* key,unsigned int &v)
    {
        return NyLPC_cUrlReader_getUInt(this->_ref_str,key,(NyLPC_TUInt32*)&v)==NyLPC_TBool_TRUE;
    }
    bool UrlReader::getQueryInt(const char* key,int &v)
    {
        return NyLPC_cUrlReader_getInt(this->_ref_str,key,(NyLPC_TInt32*)&v)==NyLPC_TBool_TRUE;
    }
    bool UrlReader::isQueryEqualStr(const char* key,const char* v)
    {
        const char* kv;
        NyLPC_TInt32 l;
        if(NyLPC_cUrlReader_getStr(this->_ref_str,key,&kv,&l)){
            return strncmp(v,kv,l)==0;
        }
        return false;
    }
    bool UrlReader::isQueryEqualUInt(const char* key,unsigned int v)
    {
        NyLPC_TUInt32 l;
        if(NyLPC_cUrlReader_getUInt(this->_ref_str,key,&l)){
            return l==v;
        }
        return false;
    }
    bool UrlReader::isQueryEqualInt(const char* key,int v)
    {
        NyLPC_TInt32 l;
        if(NyLPC_cUrlReader_getInt(this->_ref_str,key,&l)){
            return l==v;
        }
        return false;
    }

}