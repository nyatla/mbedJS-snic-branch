/*
 * NyLPC_TcIdPtrTable.c
 *
 *  Created on: 2014/11/23
 *      Author: nyatla
 */
#include "NyLPC_cIdPtrTable.h"

static struct NyLPC_TcIdPtrTableItem* id2Item(NyLPC_TcIdPtrTable_t* i_inst,NyLPC_TUInt8 i_id)
{
	int i;
	for(i=0;i<NyLPC_TcIdPtrTable_NUMBER_OF_ITEM;i++){
		if(i_inst->items[i].id==i_id){
			return &i_inst->items[i];
		}
	}
	return NULL;
}
static struct NyLPC_TcIdPtrTableItem* obj2Item(NyLPC_TcIdPtrTable_t* i_inst,const void* i_object)
{
	int i;
	for(i=0;i<NyLPC_TcIdPtrTable_NUMBER_OF_ITEM;i++){
		if(i_inst->items[i].obj==i_object){
			return &i_inst->items[i];
		}
	}
	return NULL;
}


NyLPC_TBool NyLPC_cIdPtrTable_initialize(NyLPC_TcIdPtrTable_t* i_inst)
{
	int i;
	for(i=0;i<NyLPC_TcIdPtrTable_NUMBER_OF_ITEM;i++){
		i_inst->items[i].obj=NULL;
	}
	if(!NyLPC_cMutex_initialize(&i_inst->mux)){
		return NyLPC_TBool_FALSE;
	}
	return NyLPC_TBool_TRUE;
}
void NyLPC_cIdPtrTable_finalize(NyLPC_TcIdPtrTable_t* i_inst)
{
	NyLPC_cMutex_finalize(&i_inst->mux);
}







NyLPC_TUInt8 NyLPC_cIdPtrTable_addItem(NyLPC_TcIdPtrTable_t* i_inst,void* i_object)
{
	struct NyLPC_TcIdPtrTableItem* item;
	NyLPC_cMutex_lock(&i_inst->mux);
	for(;;){
		item=obj2Item(i_inst,NULL);
		if(item==NULL){
			NyLPC_cMutex_unlock(&i_inst->mux);
			NyLPC_cThread_yield();//セマフォによる最適化対象
			NyLPC_cMutex_lock(&i_inst->mux);
			continue;
		}
		item->id=(i_inst->_counter)+1;
		item->obj=i_object;
		item->locked=NyLPC_TUInt8_FALSE;
		i_inst->_counter=(i_inst->_counter+1)%255;
		NyLPC_cMutex_unlock(&i_inst->mux);
		return item->id;
	}
}
void NyLPC_cIdPtrTable_removeItem(NyLPC_TcIdPtrTable_t* i_inst,const void* i_object)
{
	volatile struct NyLPC_TcIdPtrTableItem* item;
	NyLPC_cMutex_lock(&i_inst->mux);
	item=obj2Item(i_inst,i_object);
	while(item->locked){
		NyLPC_cMutex_unlock(&i_inst->mux);
		NyLPC_cThread_yield();
		NyLPC_cMutex_lock(&i_inst->mux);
	}
	item->obj=NULL;
	NyLPC_cMutex_unlock(&i_inst->mux);
	return;
}

void* NyLPC_cIdPtrTable_getPtrByIdWithLock(NyLPC_TcIdPtrTable_t* i_inst,NyLPC_TUInt8 i_id)
{
	struct NyLPC_TcIdPtrTableItem* item;
	NyLPC_cMutex_lock(&i_inst->mux);
	item=id2Item(i_inst,i_id);
	if(item!=NULL){
		item->locked=NyLPC_TUInt8_TRUE;
		NyLPC_cMutex_unlock(&i_inst->mux);
		return item->obj;
	}
	NyLPC_cMutex_unlock(&i_inst->mux);
	return NULL;
}
void NyLPC_cIdPtTable_unLockPtr(NyLPC_TcIdPtrTable_t* i_inst,const void* i_object)
{
	struct NyLPC_TcIdPtrTableItem* item;
	NyLPC_cMutex_lock(&i_inst->mux);
	item=obj2Item(i_inst,i_object);
	if(item!=NULL){
		item->locked=NyLPC_TUInt8_FALSE;
	}
	NyLPC_cMutex_unlock(&i_inst->mux);
	return;
}
