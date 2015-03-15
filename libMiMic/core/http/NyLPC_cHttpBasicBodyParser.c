#include "NyLPC_cHttpBasicBodyParser.h"
#include <ctype.h>

void NyLPC_cHttpBasicBodyParser_initialize(NyLPC_TcHttpBasicBodyParser_t* i_inst,struct NyLPC_TcHttpBasicBodyParser_Handler* i_handler)
{
    i_inst->_handler=i_handler;
    i_inst->_status=NyLPC_TcHttpBasicBodyParser_ST_NULL;
}
#define NyLPC_cHttpBasicBodyParser_finalize(i_inst)

/**
 * パーサの開始処理をします。
 * 関数は、parseInit->parseChar[n回]->(parseStream)->parseFinishの順でコールします。
 * parseChar、又はparseStreamでエラーが発生した場合は、後続の関数を呼び出すことは出来ません。
 * parseCharでEOHに達した場合、parseCharまたはparseStreamを続けて呼ぶことは出来ません。
 * parseFinishはparseCharまたはparseStreamでEOHに達した場合のみ呼び出すことが出来ます。
 */
void NyLPC_cHttpBasicBodyParser_parseInit(NyLPC_TcHttpBasicBodyParser_t* i_inst,const struct NyLPC_THttpBasicHeader* i_info)
{
    switch(i_info->transfer_encoding)
    {
    case NyLPC_THttpMessgeHeader_TransferEncoding_CHUNKED:
        i_inst->_encode_type=NyLPC_THttpMessgeHeader_TransferEncoding_CHUNKED;
        i_inst->_status=NyLPC_TcHttpBasicBodyParser_ST_CHUNK_HEADER_START;
        i_inst->_data.chunked.recv_len=0;
        break;
    default:
        i_inst->_encode_type=NyLPC_THttpMessgeHeader_TransferEncoding_NONE;
        i_inst->_data.normal.content_length=i_info->content_length;
        i_inst->_status=NyLPC_TcHttpBasicBodyParser_ST_BODY;
        break;
    }
}

/**
 * パーサの処理を閉じます。
 * @return
 * パース処理が正常に終了したかの真偽値
 */
NyLPC_TBool NyLPC_cHttpBasicBodyParser_parseFinish(NyLPC_TcHttpBasicBodyParser_t* i_inst)
{
    NyLPC_TBool ret=(i_inst->_status==NyLPC_TcHttpBasicBodyParser_ST_EOB);
    i_inst->_status=NyLPC_TcHttpBasicBodyParser_ST_NULL;
    return ret;
}
#define HTTP_CR 0x0D
#define HTTP_LF 0x0A
#define HTTP_SP 0x20
/**
 * HTTPストリームをパースします。
 * @return
 * 処理した文字列の長さ。-1の場合エラーである。
 * それ以外の場合ステータスをチェックすること。
 * NyLPC_TcHttpBasicBodyParser_ST_ERROR
 */
NyLPC_TInt32 NyLPC_cHttpBasicBodyParser_parseChar(NyLPC_TcHttpBasicBodyParser_t* i_inst,const NyLPC_TChar* i_c,NyLPC_TInt32 i_size)
{
    NyLPC_TInt32 i;
    NyLPC_TChar c;
    switch(i_inst->_encode_type){
    case NyLPC_THttpMessgeHeader_TransferEncoding_CHUNKED:
        for(i=0;i<i_size;)
        {
            c=*(i_c+i);
            //[:START:][:SP:][:EXT:][:BODY:][:END:]
            switch(i_inst->_status)
            {
            case NyLPC_TcHttpBasicBodyParser_ST_CHUNK_BODY:
                //OnRecv
                if(!i_inst->_handler->bodyHandler(i_inst,c)){
                    i_inst->_data.chunked.recv_len--;
                    //中断
                    if(i_inst->_data.chunked.recv_len==0){
                        //content length分だけ読み取った
                        i_inst->_status=NyLPC_TcHttpBasicBodyParser_ST_CHUNK_FOOTER;
                    }
                    return i+1;
                }
                i_inst->_data.chunked.recv_len--;
                if(i_inst->_data.chunked.recv_len==0){
                    //content length分だけ読み取った
                    i_inst->_status=NyLPC_TcHttpBasicBodyParser_ST_CHUNK_FOOTER;
                }
                i++;//次の文字へ
                break;
            // HEX
            case NyLPC_TcHttpBasicBodyParser_ST_CHUNK_HEADER_START:
                if(isxdigit((int)c)){
                    i_inst->_data.chunked.recv_len=i_inst->_data.chunked.recv_len*16+NyLPC_ctox(c);
                    //一応最大チャンクサイズは決めておこうか。
                    if(i_inst->_data.chunked.recv_len>0x0fffffff){
                        NyLPC_OnErrorGoto(ERROR);
                    }
                    i++;//次の文字へ
                }else if(c==HTTP_SP){
                    i_inst->_status=NyLPC_TcHttpBasicBodyParser_ST_CHUNK_HEADER_SP;
                    i++;//次の文字へ
                }else if(c==HTTP_CR || c==HTTP_LF){
                    i_inst->_status=NyLPC_TcHttpBasicBodyParser_ST_CHUNK_HEADER_EXT;
                    //読取位置を変化させずにEXTへ
                }else{
                    NyLPC_OnErrorGoto(ERROR);
                }
                break;
            case NyLPC_TcHttpBasicBodyParser_ST_CHUNK_HEADER_SP:
                if(c==HTTP_SP){
                    i++;//次の文字へ
                }else{
                    //ext
                    i_inst->_status=NyLPC_TcHttpBasicBodyParser_ST_CHUNK_HEADER_EXT;
                }
                break;
            case NyLPC_TcHttpBasicBodyParser_ST_CHUNK_HEADER_EXT:
                //EXTの内容は読まない。
                if(c==HTTP_LF){
                    if(i_inst->_data.chunked.recv_len==0){
                        //chunksize=0でCRLFを検出したらend-chunk
                        i_inst->_status=NyLPC_TcHttpBasicBodyParser_ST_CHUNK_END;
                    }else{
                        //chunksize>0でCRLFを検出したらBODY検出
                        i_inst->_status=NyLPC_TcHttpBasicBodyParser_ST_CHUNK_BODY;
                    }
                }else{
                    //nothing to do
                }
                i++;//次の文字へ
                break;
            case NyLPC_TcHttpBasicBodyParser_ST_CHUNK_FOOTER:
                //CRLF待ち
                if(c==HTTP_CR){
                    //無視
                }else if(c==HTTP_LF){
                    //確定
                    i_inst->_status=NyLPC_TcHttpBasicBodyParser_ST_CHUNK_HEADER_START;
                }else{
                    NyLPC_OnErrorGoto(ERROR);
                }
                i++;
                break;
            case NyLPC_TcHttpBasicBodyParser_ST_CHUNK_END:
                //CRLF待ち
                if(c==HTTP_CR){
                    //無視
                }else if(c==HTTP_LF){
                    //確定
                    i_inst->_status=NyLPC_TcHttpBasicBodyParser_ST_EOB;
                    return i+1;
                }else{
                    NyLPC_OnErrorGoto(ERROR);
                }
                i++;
                break;
            default:
                NyLPC_OnErrorGoto(ERROR);
            }
        }
        return i_size;
    case NyLPC_THttpMessgeHeader_TransferEncoding_NONE:
        if(i_inst->_status!=NyLPC_TcHttpBasicBodyParser_ST_BODY){
            NyLPC_OnErrorGoto(ERROR);
        }
        for(i=0;i<i_size;i++)
        {
            if(i_inst->_data.normal.content_length>0){
                //OnRecv
                if(!i_inst->_handler->bodyHandler(i_inst,*(i_c+i))){
                    i++;
                    i_inst->_data.normal.content_length--;
                    break;//中断(遷移無し)
                }
                i_inst->_data.normal.content_length--;
            }else{
                //content-length==0;全て受信
                break;
            }
        }
        if(i_inst->_data.normal.content_length==0){
            //content length分だけ読み取った
            i_inst->_status=NyLPC_TcHttpBasicBodyParser_ST_EOB;
            return i;
        }
        return i;
    }
ERROR:
    //ERROR
    i_inst->_status=NyLPC_TcHttpBasicBodyParser_ST_ERROR;
    return -1;
}


