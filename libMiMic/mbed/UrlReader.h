#pragma once

namespace MiMic
{
    /**
     * This class is NULL terminated URL text reader. 
     * (Currentry, The class can read only absolute URL.)
     */
    class UrlReader
    {
        protected:
            const char* _ref_str;
        public:
            /**
             * The constructor
             * This function creates reader interface for null terminated text.
             * @param i_ref_str
             *  URL text for read.
             *  This is referenced pointer. Must hold it until an instance is closed.
             *  If omited this, Should call setUrl function before call other functions.
             */
            UrlReader(const char* i_ref_url=NULL);
            /**
             * This function sets new text.
             * @param i_ref_str
             *  URL text for read.
             *  This is referenced pointer. Must hold it until an instance is closed.
             */
            void setUrl(const char* i_ref_url);
            //bool hasHost(const char* key);
            //bool getHost(const char* i_ref_url);
            //bool hasPath(const char* i_ref_url);
            //bool getPath(const char* i_ref_url);
            /**
             * This function confirms URL has a query key.
             * @param key
             * a query key name in text.
             * @return
             * TRUE if text has key name, otherwise FALSE.
             */
            bool hasQueryKey(const char* key);
            /**
             * This function gets a query key value.
             * @param key
             * a query key name.
             * @param o_ref_value
             * address of variable which store a pointer to key value.
             * It is part of URL which is not terminated by '\0'.
             * @param o_val_len
             * length of value text.
             * @return
             * true if got value. otherwise false.
             */
            bool getQueryStr(const char* key,const char* &o_ref_val,int &o_val_len);
            /**
             * This function gets a converted query key value in unsigned integer.
             * @param key
             * a query key name.
             * @param v
             * address of variable which store a pointer to key value.
             * @return
             * true if got value. otherwise false.
             */
            bool getQueryUInt(const char* key,unsigned int &v);
            /**
             * This function gets a converted query key value in integer.
             * @param key
             * a query key name.
             * @param v
             * address of variable which store a pointer to key value.
             * @return
             * true if got value. otherwise false.
             */
            bool getQueryInt(const char* key,int &v);

            /**
             * This function gets a query key value and check it is equal.
             * @param key
             * key name.
             * @param v
             * string to check 
             * @return
             * true if got value that is same as v. otherwise false.
             */            
            bool isQueryEqualStr(const char* key,const char* v);
            /**
             * This function gets a query key value and check it is equal.
             * @param key
             * key name.
             * @param v
             * string to check 
             * @return
             * true if got value that is same as v. otherwise false.
             */  
            bool isQueryEqualUInt(const char* key,unsigned int v);
            /**
             * This function gets a query key value and check it is equal.
             * @param key
             * key name.
             * @param v
             * string to check 
             * @return
             * true if got value that is same as v. otherwise false.
             */
            bool isQueryEqualInt(const char* key,int v);
            /**
             * This function gets path part pointer and length from text.
             * @param i_path
             * address of variable which store a pointer to path part.
             * @param i_l
             * length of path part.
             * @return
             * true if got. otherwise false
             */
            bool getPath(const char* &path,int &l);
            
            /**
             * This function checks whether a path section in URL string is equal. 
             *
             */
            bool isPathEqual(const char* path);

       };
}