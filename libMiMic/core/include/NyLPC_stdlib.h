/*********************************************************************************
 * PROJECT: MiMic
 * --------------------------------------------------------------------------------
 *
 * This file is part of MiMic
 * Copyright (C)2011 Ryo Iizuka
 *
 * MiMic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by　the Free Software Foundation, either version 3 of the　License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information please contact.
 *  http://nyatla.jp/
 *  <airmail(at)ebony.plala.or.jp> or <nyatla(at)nyatla.jp>
 *
 *********************************************************************************/
/**
 * @file
 * このファイルは、ルートモジュールにあるヘッダファイルを集積します。
 * 基本的な型の定義も行います。
 */
#ifndef NyLPC_stdlib_h
#define NyLPC_stdlib_h
#include "NyLPC_config.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**********************************************************************
 *
 * ASSERT/ABORT
 *
 **********************************************************************/
#define MIMIC_DEBUG 1
#ifdef MIMIC_DEBUG

    /**
     * 引数チェック専用のASSERT構文です。
     * デバック時のみ有効です。
     */
    #define NyLPC_ArgAssert(e) if(!(e)){NyLPC_assertHook(NULL,-1);};

    /**
     * ASSERTマクロです。
     * デバック時のみ有効です。
     */
    #define NyLPC_Assert(e) if(!(e)){NyLPC_assertHook(NULL,-1);};

    /**
     * デバック用のフックマクロです。
     * デバック時のみ有効です。
     */
    #define NyAR_DebugHook() {NyLPC_debugHook(__FILE__,__LINE__);};

    /**
     * Abortマクロです。プログラムを異常終了するときのエントリポイントになります。
     * デバック時/リリース時のどちらでも有効です。
     */
    #define NyLPC_Abort() {NyLPC_abortHook(__FILE__,__LINE__);};    //もうだめ

    /**
     * Abortマクロです。eが偽の時に、異常終了します。
     * デバック時/リリース時のどちらでも有効です。
     * @param e
     * 評価式です.
     */
    #define NyLPC_AbortIfNot(e) if(!(e)){NyLPC_abortHook(__FILE__,__LINE__);};

    /**
     * 警告表示用のマクロです.デバックに使います.
     * デバック時のみ有効です.
     */
    #define NyLPC_Warning() {NyLPC_debugHook(__FILE__,__LINE__);};

    /**
     * 警告表示用のマクロです.eが偽の時に、警告を出します.
     * デバック時のみ有効です.
     * @param e
     * 評価式です。
     */
    #define NyLPC_WarningIfNot(e) if(!(e)){NyLPC_debugHook(__FILE__,__LINE__);};

    /*
     * トレースマクロです。デバックに使います。
     * 内部変数に、最後にコールされたファイル名と、行番号を保存します。
     * デバック時のみ有効です.
     */
    #define NyLPC_Trace() {NyLPC_debugHook(__FILE__,__LINE__);};
#else
    #define NyLPC_ArgAssert(e) if(!(e)){NyLPC_assertHook(NULL,-1);};
    #define NyLPC_Assert(e) if(!(e)){NyLPC_assertHook(NULL,-1);};
    #define NyAR_DebugHook() {NyLPC_debugHook(__FILE__,__LINE__);};
    #define NyLPC_Abort() {NyLPC_abortHook(__FILE__,__LINE__);};    //もうだめ
    #define NyLPC_AbortIfNot(e) if(!(e)){NyLPC_abortHook(__FILE__,__LINE__);};
    #define NyLPC_Warning() {NyLPC_debugHook(__FILE__,__LINE__);};
    #define NyLPC_WarningIfNot(e) if(!(e)){NyLPC_debugHook(__FILE__,__LINE__);};
    #define NyLPC_Trace() {NyLPC_debugHook(__FILE__,__LINE__);};
#endif

/**
 * アサートマクロが使う関数です。
 * @param m
 * __FILE__を指定します。
 * @param l
 * __LINE__を指定します。
 */
void NyLPC_assertHook(const char* m,int l);
 /**
  * Abortマクロが使う関数です。
  * @param m
  * __FILE__を指定します。
  * @param l
  * __LINE__を指定します。
  */
void NyLPC_abortHook(const char* m,int l);
/**
 * Debugマクロが使う関数です。
 * @param m
 * __FILE__を指定します。
 * @param l
 * __LINE__を指定します。
 */
void NyLPC_debugHook(const char* m,int l);

/**
 * デバック用のステータス変数です。
 * アサートを検出した回数を保存します。
 */
extern unsigned int NyLPC_assert_counter;
/**
 * デバック用のステータス変数です。
 * アサートを検出した回数を保存します。
 */
extern unsigned int NyLPC_abort_counter;
/**
 * デバック用のステータス変数です。
 * アサートを検出した回数を保存します。
 */
extern unsigned int NyLPC_debug_counter;

/**
 * GOTO方式のエラーハンドラを記述するためのマクロです。
 * @param l
 * 指定ラベルへジャンプします。
 */
#define NyLPC_OnErrorGoto(l) goto l



/**********************************************************************
 *
 * Basic type
 *
 **********************************************************************/

/**
 * 真偽値型です。
 */
typedef long NyLPC_TBool;

/**
 * TRUEを表します。この値は、ifで評価したときに真を返すことを保障します。
 */
#define NyLPC_TBool_TRUE (0==0)
/**
 * FALSEを表します。この値は、ifで評価したときに偽を返すことを保障します。
 */
#define NyLPC_TBool_FALSE (0!=0)

/**
 * ｳｪｰｲを表します。この値は、NyLPC_TBool_FALSEと同じです。
 */
#define NyLPC_TBool_VEII NyLPC_TBool_FALSE



/**
 * 8bit長のバイト文字の型です。
 */
typedef char NyLPC_TChar;
/**
 * 符号有り8bit型です。
 */
typedef signed char NyLPC_TInt8;
/**
 * 符号有り16bit型です。
 */
typedef signed short NyLPC_TInt16;
/**
 * 符号有り32bit型です。
 */
typedef signed long NyLPC_TInt32;

//----------------------------------------------------------------------
//　NyLPC_TUInt8
//----------------------------------------------------------------------

/**
 * 符号無し8bit型です。
 */
typedef unsigned char NyLPC_TUInt8;

/**
 * ビット操作関数です。bfのビット番号bに、1をセットします。
 * @bf
 * 操作対象の変数です。
 * @b
 * 操作するビットパターンです。
 */
#define NyLPC_TUInt8_setBit(bf,b) NyLPC_TUInt32_setBit(bf,b)
/**
 * ビット操作関数です。bfのビット番号bに、0をセットします。
 * @bf
 * 操作対象の変数です。
 * @b
 * 操作するビットパターンです。
 */
#define NyLPC_TUInt8_unsetBit(bf,b) NyLPC_TUInt32_unsetBit(bf,b)
/**
 * ビット判定関数です。bfのビット番号bが1であるかを確認します。
 * @bf
 * 判定する変数です。
 * @b
 * 判定するビットパターンです。
 * @return
 * ビットが一致するなら真を返します。
 */
#define NyLPC_TUInt8_isBitOn(bf,b) NyLPC_TUInt32_isBitOn(bf,b)

/**
 * 8bit長のTRUE値です。
 */
#define NyLPC_TUInt8_TRUE NyLPC_TBool_TRUE
/**
 * 8bit長のFALSE値です。
 */
#define NyLPC_TUInt8_FALSE NyLPC_TBool_FALSE
/**
 * 8bit長のFALSEをNyLPC_TBoolへ変換します。
 * @param a
 * 変換する変数です。
 * @return
 * 変換した値です。
 */
#define NyLPC_TUInt8_castto_TBool(a) ((a)?NyLPC_TBool_TRUE:NyLPC_TBool_FALSE)

//----------------------------------------------------------------------
// NyLPC_TUInt16
//----------------------------------------------------------------------

/**
 * 符号無し16bit型です。
 */
typedef unsigned short NyLPC_TUInt16;

/**
 * INTMAX
 */
#define NyLPC_TUInt16_MAX 0xFFFF

/**
 * ビット操作関数です。bfのビット番号bに、1をセットします。
 * @bf
 * 操作対象の変数です。
 * @b
 * 操作するビット番号です。
 */
#define NyLPC_TUInt16_setBit(bf,b) NyLPC_TUInt32_setBit(bf,b)
/**
 * ビット操作関数です。bfのビット番号bに、0をセットします。
 * @bf
 * 操作対象の変数です。
 * @b
 * 操作するビット番号です。
 */
#define NyLPC_TUInt16_unsetBit(bf,b) NyLPC_TUInt32_unsetBit(bf,b)
/**
 * ビット判定関数です。bfのビット番号bが1であるかを確認します。
 * @bf
 * 判定する変数です。
 * @b
 * 判定するビット番号です。
 * @return
 * ビットが一致するなら真を返します。
 */
#define NyLPC_TUInt16_isBitOn(bf,b) NyLPC_TUInt32_isBitOn(bf,b)

/**
 * バイトオーダーを入れ替えます。
 * @param n
 * 変換もとの変数です。
 * @return
 * 入れ替えた16ビット値を返します。
 *
 */
#define NyLPC_TUInt16_BSWAP(n) (((((NyLPC_TUInt16)(n))<< 8)&0xff00)|((((NyLPC_TUInt16)(n))>> 8)&0x00ff))



/**
 * バイトオーダーを入れ替えます。
 * NyLPC_TUInt16_BSWAPとの違いは、関数であることです。
 * @param n
 * 変換もとの変数です。
 * @return
 * 入れ替えた16ビット値を返します。
 */
NyLPC_TUInt16 NyLPC_TUInt16_bswap(NyLPC_TUInt16 n);
/**
 * 16bit長のTRUE値です。
 */
#define NyLPC_TUInt16_TRUE NyLPC_TBool_TRUE
/**
 * 16bit長のFALSE値です。
 */
#define NyLPC_TUInt16_FALSE NyLPC_TBool_FALSE
/**
 * 16bit長のFALSEをNyLPC_TBoolへ変換します。
 * @param a
 * 変換する変数です。
 * @return
 * 変換した値です。
 */
#define NyLPC_TUInt16_castto_TBool(a) ((a)?NyLPC_TBool_TRUE:NyLPC_TBool_FALSE)

//----------------------------------------------------------------------

/**
 * 符号無し32bit型です。
 */
typedef unsigned long NyLPC_TUInt32;
/**
 * ビット操作関数です。bfのビットパターンbに、1をセットします。
 * @bf
 * 操作対象の変数です。
 * @b
 * 操作するビットパターンです。
 */
#define NyLPC_TUInt32_setBit(bf,b) bf=(bf|(1<<b))
/**
 * ビット操作関数です。bfのビットパターンbに、0をセットします。
 * @bf
 * 操作対象の変数です。
 * @b
 * 操作するビットパターンです。
 */
#define NyLPC_TUInt32_unsetBit(bf,b) bf=(bf&(~(1<<b)))
/**
 * ビット判定関数です。bfのビットパターンbが1であるかを確認します。
 * @bf
 * 判定する変数です。
 * @b
 * 判定するビットパターンです。
 * @return
 * ビットが一致するなら真を返します。
 */
#define NyLPC_TUInt32_isBitOn(bf,b) ((bf&(1<<b))!=0)

/**
 * バイトオーダーを入れ替えます。
 * @param n
 * 変換もとの変数です。
 * @return
 * 入れ替えた32ビット値を返します。
 *
 */
#define NyLPC_TUInt32_BSWAP(n) (((((NyLPC_TUInt32)(n))<<24)&0xff000000)|((((NyLPC_TUInt32)(n))<< 8)&0x00ff0000)|((((NyLPC_TUInt32)(n))>> 8)&0x0000ff00)|((((NyLPC_TUInt32)(n))>>24)&0x000000ff))
/**
 * バイトオーダーを入れ替えます。
 * NyLPC_TUInt32_BSWAPとの違いは、関数であることです。
 * @param n
 * 変換もとの変数です。
 * @return
 * 入れ替えた32ビット値を返します。
 */
NyLPC_TUInt32 NyLPC_TUInt32_bswap(NyLPC_TUInt32 n);

/**
 * 32bit長のTRUE値です。
 */
#define NyLPC_TUInt32_TRUE NyLPC_TBool_TRUE
/**
 * 32bit長のFALSE値です。
 */
#define NyLPC_TUInt32_FALSE NyLPC_TBool_FALSE
/**
 * 32bit長のFALSEをNyLPC_TBoolへ変換します。
 * @param a
 * 変換する変数です。
 * @return
 * 変換した値です。
 */
#define NyLPC_TUInt32_castto_TBool(a) ((a)?NyLPC_TBool_TRUE:NyLPC_TBool_FALSE)


//----------------------------------------------------------------------

/**
 * 長さ付TChar配列の構造体です。
 *
 */
struct NyLPC_TCharArrayPtr
{
    /** 配列のポインタ */
    NyLPC_TChar* ptr;
    /**　配列の長さ */
    NyLPC_TUInt16 len;
};
/**
 * ptrの位置をi_seekだけ進行します。
 * @param i_struct
 * 操作する構造体
 * @param i_seek
 * シークするバイト長
 */
NyLPC_TBool NyLPC_TCharArrayPtr_seek(struct NyLPC_TCharArrayPtr* i_struct,NyLPC_TUInt16 i_seek);


/**
 * 長さ付TUInt32配列の構造体です。
 *
 */
struct NyLPC_TUInt32ArrayPtr
{
    /** 配列のポインタ */
    NyLPC_TUInt32* ptr;
    /**　要素数 */
    NyLPC_TUInt16 len;
};
/**
 * ptrの位置をi_seekだけ進行します。
 * 進行すると、len要素がi_seekだけ減少します。
 * @param i_struct
 * 操作する構造体
 * @param i_seek
 * シークする要素長
 */
NyLPC_TBool NyLPC_TUInt32ArrayPtr_seek(struct NyLPC_TUInt32ArrayPtr* i_struct,NyLPC_TUInt16 i_seek);
/**
 * 構造体に、参照するバッファの初期位置とサイズをセットします。
 * セットしたバッファは、前方シークの可能な書き込みバッファとして使用できます。
 */
void NyLPC_TUInt32ArrayPtr_setBuf(struct NyLPC_TUInt32ArrayPtr* i_struct,NyLPC_TUInt32* i_ptr,NyLPC_TUInt16 i_len);


/**
 * TextとIDのテーブルです。
 */
struct NyLPC_TTextIdTbl{
    const char* n;
    NyLPC_TUInt8 id;
};

/**
 * テーブルから文字列に一致するIDを返します。
 * 大文字と小文字の区別をしません。
 * @return
 * 一致する文字列のID
 * 一致するものがない場合、テーブルの終端の値を返す。
 */
NyLPC_TUInt8 NyLPC_TTextIdTbl_getMatchId(const NyLPC_TChar* i_str,const struct NyLPC_TTextIdTbl i_tbl[]);
/**
 * テーブルから文字列に一致するIDを返します。
 * 大文字と小文字の区別をしません。
 * @return
 * 一致する文字列のID
 * 一致するものがない場合、テーブルの終端の値を返す。
 */
NyLPC_TUInt8 NyLPC_TTextIdTbl_getMatchIdIgnoreCase(const NyLPC_TChar* i_str,const struct NyLPC_TTextIdTbl i_tbl[]);
/**
 * テーブルからIDに一致する文字列を返す.
 * @return
 * IDに一致する文字列.
 * 存在しなければNULL
 */
const NyLPC_TChar* NyLPC_TTextIdTbl_getTextById(NyLPC_TUInt8 i_id,const struct NyLPC_TTextIdTbl i_tbl[]);

/*********************************************************************************
 * standard function
 *********************************************************************************/
/**
 * Same as tolower
 */
#define NyLPC_tolower(c) (((c) >= 'A' && (c) <= 'Z' )?((c)+'a' - 'A'):(c))

/**
 * @return
 * 書き込んだ文字列の長さ
 */
NyLPC_TInt8 NyLPC_itoa(int i_n,char* o_out,NyLPC_TInt8 i_base);


/**
 * @return
 * 書き込んだ文字列の長さ
 */
NyLPC_TInt8 NyLPC_uitoa(unsigned int i_n,char* o_out,NyLPC_TInt8 i_base);
/**
 * 桁数の指定できるuitoaです。
 */
NyLPC_TInt8 NyLPC_uitoa2(unsigned int i_n,char* o_out,NyLPC_TInt8 i_base,NyLPC_TInt8 i_digit);


/**
 * Same as reverse
 */
void NyLPC_reverse(char* s);

/**
 * Same as stricmp
 */
int NyLPC_stricmp(const char *i_s1, const char *i_s2);

/**
 * Same as strnicmp
 */
int NyLPC_strnicmp(const char *i_s1, const char *i_s2,int n);

/**
 * Convert a charactor to integer.
 */
int NyLPC_ctoi(char i);

/**
 * Convert a character to 16 digit integer.
 */
int NyLPC_ctox(char i);

/**
 * va_copyがない場合の対処だお
 */
#ifndef va_copy
#    define NyLPC_va_copy(dest, src) ((dest) = (src))
#else
#    define NyLPC_va_copy(dest, src) va_copy((dest),(src))
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */



/*********************************************************************************
 *
 *********************************************************************************/

#include "../NyLPC_cMiMicEnv.h"
#include "../NyLPC_cRingBuffer.h"
#include "../NyLPC_cPtrTbl.h"
#include "../NyLPC_cFifoBuffer.h"
#include "../NyLPC_cPtrStream.h"
#include "../NyLPC_cStr.h"





#endif
