#ifndef cOnchipFlashWriter_h
#define cOnchipFlashWriter_h


#include "NyLPC_stdlib.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



/**
 * アドレスi_destに、i_srcの内容を書き込みます。
 * 書き込みは、FlashROMがイレース済なものとして実行します。書込み範囲以外のデータは、変更されません。
 * 既に書込み済みのデータがある場合(0xFFFFFFFF以外)は、期待した結果が得られないので、注意してください。
 * この関数は、IAPインタフェイス経由で256バイト単位でデータを書き込みます。
 * IAPは0x1000000から32バイトをワークエリアとして使用します。
 * 関数はワークエリアの待避と復帰を行いますが、安全の為、使用前にRTOSを一旦停止させてください。
 * この関数はリエントラントではありません。
 */
NyLPC_TBool NyLPC_cOnchipFlashWriter_write(const void* i_dest,const void* i_src,NyLPC_TUInt32 i_size);
/**
 * この関数はリエントラントではありません。
 */
NyLPC_TBool NyLPC_cOnchipFlashWriter_writeSector(NyLPC_TUInt16 i_sector,NyLPC_TUInt32 i_offset,const void* i_src,NyLPC_TUInt32 i_size);
NyLPC_TBool NyLPC_cOnchipFlashWriter_elase(NyLPC_TUInt16 i_sector_s,NyLPC_TUInt16 i_sector_e);
NyLPC_TBool NyLPC_cOnchipFlashWriter_isOnchipFlash(const void* i_addr);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
