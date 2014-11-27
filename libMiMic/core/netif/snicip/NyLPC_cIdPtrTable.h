/*
 * NyLPC_cIdPtrTable.h
 *
 *  Created on: 2014/11/23
 *      Author: nyatla
 */

#ifndef NYLPC_CIDPTRTABLE_H_
#define NYLPC_CIDPTRTABLE_H_

#include "NyLPC_stdlib.h"
#include "NyLPC_os.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * このクラスはid値とポインタのペアテーブルです。
 * ついでにポインタのロック機構を提供します。
 */
typedef struct NyLPC_TcIdPtrTable NyLPC_TcIdPtrTable_t;

#define NyLPC_TcIdPtrTable_NUMBER_OF_ITEM 10
struct NyLPC_TcIdPtrTableItem
{
	volatile NyLPC_TUInt8 locked;
	NyLPC_TUInt8 id;
	void* obj;
};
struct NyLPC_TcIdPtrTable
{
	NyLPC_TUInt8 _counter;
	NyLPC_TcMutex_t mux;
	struct NyLPC_TcIdPtrTableItem items[NyLPC_TcIdPtrTable_NUMBER_OF_ITEM];
};

NyLPC_TBool NyLPC_cIdPtrTable_initialize(NyLPC_TcIdPtrTable_t* i_inst);
void NyLPC_cIdPtrTable_finalize(NyLPC_TcIdPtrTable_t* i_inst);

/**
 * オブジェクトを登録してidを返します。
 */
NyLPC_TUInt8 NyLPC_cIdPtrTable_addItem(NyLPC_TcIdPtrTable_t* i_inst,void* i_object);
/**
 * オブジェクトを解放します。
 * オブジェクトがロックされている場合、アンロックされるまでブロックします。
 */
void NyLPC_cIdPtrTable_removeItem(NyLPC_TcIdPtrTable_t* i_inst,const void* i_object);
/**
 * idでオブジェクトをロックして返します。
 * 多重ロックは検出出来ません。
 */
void* NyLPC_cIdPtrTable_getPtrByIdWithLock(NyLPC_TcIdPtrTable_t* i_inst,NyLPC_TUInt8 i_id);
void NyLPC_cIdPtTable_unLockPtr(NyLPC_TcIdPtrTable_t* i_inst,const void* i_object);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CIDPTRTABLE_H_ */
