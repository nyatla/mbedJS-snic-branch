#ifndef NYLPC_CMODWEBSOCKET_PROTECTED_H_
#define NYLPC_CMODWEBSOCKET_PROTECTED_H_

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
 *	http://nyatla.jp/
 *	<airmail(at)ebony.plala.or.jp> or <nyatla(at)nyatla.jp>
 *
 *********************************************************************************/
#include "NyLPC_cModWebSocket.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * Payloadヘッダを書く。
 */
NyLPC_TBool NyLPC_cModWebSocket_writePayloadHeader(NyLPC_TcModWebSocket_t* i_inst,NyLPC_TInt16 i_len);

void NyLPC_cModWebSocket_update(NyLPC_TcModWebSocket_t* i_inst,NyLPC_TUInt32 i_time_out);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NYLPC_CMODWEBSOCKET_H_ */
