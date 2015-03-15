/*
 * INetIf.h
 *
 *  Created on: 2014/12/12
 *      Author: nyatla
 */

#pragma once
#ifndef INETIF_H_
#define INETIF_H_

#include "NyLPC_net.h"
#include "NyLPC_netif.h"

namespace MiMic
{
    class INetif
    {
    public:
    	virtual const NyLPC_TiNetInterface_Interface* getInterface()=0;
    };
}



#endif /* INETIF_H_ */
