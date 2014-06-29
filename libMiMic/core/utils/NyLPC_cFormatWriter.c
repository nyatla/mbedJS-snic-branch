#include "NyLPC_cFormatWriter.h"


#define FTYPE_LENGTH 0x01
#define FTYPE_NOTHING 0x00

#define NUM_OF_WORK 16
NyLPC_TBool NyLPC_cFormatWriter_print(NyLPC_cFormatWriter_printHandler i_handler,void* i_inst,const NyLPC_TChar* i_fmt,va_list args)
{
    const char* rp=i_fmt;
    const char* sp;
    char wk[NUM_OF_WORK];
    NyLPC_TUInt8 ftype;
    NyLPC_TUInt32 ut;
    NyLPC_TInt16 ol=0;
    while(*rp!='\0'){
        if(*rp=='%'){
            ftype=FTYPE_NOTHING;
            rp++;
        FMT_NEXT:
            switch (*rp){
            case '.':
                //%.*(s)
                if(*(rp+1)=='*'){
                    //%.*
                    ftype=FTYPE_LENGTH;
                    rp+=2;
                    goto FMT_NEXT;
                }
                //その他
                wk[ol]=*rp;
                ol++;
                rp++;
                break;
            case 's':
                switch(ftype){
                case FTYPE_LENGTH:
                    //%.*sの場合
                    ut=va_arg(args,NyLPC_TUInt32);
                    break;
                default:
                    ut=0x7FFFFFFF;
                }
                sp=va_arg(args,const char*);
                while(*sp!=0 && ut>0){
                    wk[ol]=*sp;
                    ol++;
                    sp++;
                    //バッファフルなら書込み。
                    if(ol>=NUM_OF_WORK){
                        i_handler(i_inst,wk,NUM_OF_WORK);
                        ol=0;
                    }
                    ut--;
                }
                rp++;
                continue;
            case 'c':
                wk[ol]=(char)va_arg(args,int);
                rp++;
                ol++;
                break;
            case 'd':
                //ワークを空にする。
                if(ol>0){
                    i_handler(i_inst,wk,ol);
                    ol=0;
                }
                NyLPC_itoa((va_arg(args,int)),wk,10);
                //強制コミット
                i_handler(i_inst,wk,strlen(wk));
                rp++;
                continue;
            case 'u':
                //ワークを空にする。
                if (ol>0){
                    i_handler(i_inst, wk, ol);
                    ol = 0;
                }
                NyLPC_uitoa((va_arg(args, NyLPC_TUInt32)), wk, 10);
                //強制コミット
                i_handler(i_inst, wk, strlen(wk));
                rp++;
                continue;
            //16進数
            case 'x':
                //ワークを空にする。
                if(ol>0){
                    i_handler(i_inst,wk,ol);
                    ol=0;
                }
                NyLPC_uitoa((va_arg(args,unsigned int)),wk,16);
                //強制コミット
                i_handler(i_inst,wk,strlen(wk));
                rp++;
                continue;
            //BYTE値のHEXエンコード文字列(XX形式のテキスト。%.*Bのみ)
            case 'B':
                switch(ftype){
                case FTYPE_LENGTH:
                    //%.*Bの場合
                    ut=va_arg(args,NyLPC_TUInt32);
                    break;
                default:
                    ut=0;
                }
                sp=va_arg(args,const char*);
                while(ut>0){
                    //2文字以上の空きがないなら書き込み
                    if (ol >= (NUM_OF_WORK - 2)){
                        i_handler(i_inst, wk, ol);
                        ol = 0;
                    }
                    NyLPC_uitoa2((int)(*sp), wk + ol, 16, 2);
                    ol += 2;
                    sp++;
                    ut--;
                }
                rp++;
                continue;
            case '%':
                wk[ol]='%';
                ol++;
                rp++;
                break;
            case '\0':
                //オワタ(ループ抜けるためにrpはそのまま。)
                break;
            default:
                wk[ol]=*rp;
                ol++;
            }
            //バッファフルなら書込み。
            if(ol>=NUM_OF_WORK){
                i_handler(i_inst,wk,NUM_OF_WORK);
                ol=0;
            }
        }else if(*rp==0){
            //オワタ
            break;
        }else{
            wk[ol]=*rp;
            ol++;
            rp++;
            if(ol>=NUM_OF_WORK){
                i_handler(i_inst,wk,NUM_OF_WORK);
                ol=0;
            }
        }
    }
    //どこかでエラーが起こってればFALSE返す。
    return i_handler(i_inst,wk,ol);
}

NyLPC_TInt16 NyLPC_cFormatWriter_length(const NyLPC_TChar* i_fmt,va_list args)
{
    const char* rp=i_fmt;
    const char* sp;
    char wk[NUM_OF_WORK];
    NyLPC_TUInt32 ut;
    NyLPC_TUInt8 ftype;
    NyLPC_TInt16 len=0;
    while(*rp!='\0'){
        if(*rp=='%'){
            ftype=FTYPE_NOTHING;
            rp++;
        FMT_NEXT:
            switch (*rp){
            case '.':
                //%.*(s)
                if(*(rp+1)=='*'){
                    //%.*
                    ftype=FTYPE_LENGTH;
                    rp+=2;
                    goto FMT_NEXT;
                }
                //その他
                len++;
                rp++;
                break;
            case 's':
                switch(ftype){
                case FTYPE_LENGTH:
                    //%.*sの場合
                    ut=va_arg(args,NyLPC_TUInt32);
                    break;
                default:
                    ut=0x7FFFFFFF;
                }
                sp=va_arg(args,const char*);
                while(*sp!=0 && ut>0){
                    len++;
                    sp++;
                    ut--;
                }
                rp++;
                continue;
            case 'c':
                va_arg(args,int);
                len++;
                rp++;
                break;
            case 'd':
                NyLPC_itoa((va_arg(args,int)),wk,10);
                //強制コミット
                len+=(NyLPC_TInt16)strlen(wk);
                rp++;
                continue;
            case 'u':
                //ワークを空にする。
                NyLPC_uitoa((va_arg(args, NyLPC_TUInt32)), wk, 10);
                //強制コミット
                len += (NyLPC_TInt16)strlen(wk);
                rp++;
                continue;
            case 'x':
                NyLPC_uitoa((va_arg(args,unsigned int)),wk,16);
                //強制コミット
                len+=(NyLPC_TInt16)strlen(wk);
                rp++;
                continue;
            //BYTE値のHEXエンコード文字列(XX形式のテキスト。%.*Bのみ)
            case 'B':
                switch(ftype){
                case FTYPE_LENGTH:
                    //%.*Bの場合
                    ut=va_arg(args,NyLPC_TUInt32);
                    break;
                default:
                    ut=0;
                }
                sp=va_arg(args,const char*);
                len += (NyLPC_TInt16)ut * 2;
                rp++;
                continue;
            case '%':
                len++;
                rp++;
                break;
            case '\0':
                //オワタ(ループ抜けるためにrpはそのまま。)
                break;
            default:
                len++;
            }
        }else if(*rp==0){
            //オワタ
            break;
        }else{
            len++;
            rp++;
        }
    }
    //どこかでエラーが起こってればFALSE返す。
    return len;
}
