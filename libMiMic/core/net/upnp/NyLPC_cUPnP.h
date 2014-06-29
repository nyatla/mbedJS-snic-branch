#ifndef NyLPC_TCUPnP_H
#define NyLPC_TCUPnP_H
#include "NyLPC_cSsdpSocket.h"
#include "NyLPC_UPnP_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct NyLPC_TcUPnP NyLPC_TcUPnP_t;

struct NyLPC_TcUPnP
{
	/**
	 * 階層構造のルートデバイス
	 */
	const struct NyLPC_TUPnPDevDescDevice* ref_root_device;
	const char* _ref_root_path;
	NyLPC_TcSsdpSocket_t _ssdp;
};



/**
 * UPnPサービスのインスタンスを構築します。
 * nyardinoシステムの場合はsetup関数で実行してください。
 * 関数はソケットコンストラクタを使用します。UIPサービスを停止中に実行してください。
 * @param i_inst
 * インスタンスのポインタ
 * @param i_http_port
 * HTTPサービスのポート番号
 * @param i_path
 * HTTPサービスのルートパス
 * @param i_ref_description
 * DeviceDescription構造体のポインタ
 */
void NyLPC_cUPnP_initialize(NyLPC_TcUPnP_t* i_inst,NyLPC_TUInt16 i_http_port,const NyLPC_TChar* i_path,const struct NyLPC_TUPnPDevDescDevice* i_ref_description);
void NyLPC_cUPnP_finalize(NyLPC_TcUPnP_t* i_inst);
void NyLPC_cUPnP_start(NyLPC_TcUPnP_t* i_inst);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif
