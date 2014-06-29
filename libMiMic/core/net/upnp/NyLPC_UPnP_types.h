#ifndef NyLPC_TCUPnP_types_H
#define NyLPC_TCUPnP_types_H
#include "NyLPC_stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct NyLPC_TUPnPDevDescIcon{
	const NyLPC_TChar* mimetype;
	NyLPC_TInt16 width;
	NyLPC_TInt16 height;
	NyLPC_TInt32 depth;
	const NyLPC_TChar* url;
};

/**
 * UPnP ServiceRecord
 */
struct NyLPC_TUPnPDevDescService
{
	/** Required*/
	const NyLPC_TChar* scpd_url;
	/** Required*/
	const NyLPC_TChar* service_type;
	/** Required*/
	const NyLPC_TChar* service_id;
};

struct NyLPC_TUPnPDevDescDevice
{
	/** Required*/
	const NyLPC_TChar* device_type;
	/** Required*/
	const NyLPC_TChar* frendly_name;
	/** Required*/
	const NyLPC_TChar* manufacturer;
	/** Optional*/
	const NyLPC_TChar* manufacturer_url;
	/** Recommended*/
	const NyLPC_TChar* model_descriprion;
	/** Required*/
	const NyLPC_TChar* model_name;
	/** Recommended*/
	const NyLPC_TChar* model_number;
	/** Optional*/
	const NyLPC_TChar* model_url;
	/** Recommended*/
	const NyLPC_TChar* serial_number;
	/** Required*/
	const NyLPC_TChar* udn;
	/** Optional*/
	const NyLPC_TChar* upc;
	/** Recommended*/
	const NyLPC_TChar* presentation_url;
	NyLPC_TInt8 number_of_devices;
	NyLPC_TInt8 number_of_service;
	NyLPC_TInt8 number_of_icon;
	NyLPC_TInt8 _padding;
	/** Required if number_of_devices>0
	 * UPnPデバイス構造体のポインタへの配列
	 */
	const struct NyLPC_TUPnPDevDescDevice** devices;
	/**
	 * Optional
	 * UPnPサービス構造体のポインタへの配列
	 */
	const struct NyLPC_TUPnPDevDescService* services;
	/** Required if number_of_icon>0*/
	const struct NyLPC_TUPnPDevDescIcon* icons;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */



#endif
