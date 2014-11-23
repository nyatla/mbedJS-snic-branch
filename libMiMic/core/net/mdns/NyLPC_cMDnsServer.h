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
#ifndef NYLPC_CMDNSSERVER_H_
#define NYLPC_CMDNSSERVER_H_
#include "NyLPC_netif.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * NyLPC_TDnsRecordで使用するサービスレコード
 */
struct NyLPC_TMDnsServiceRecord
{
    const char* protocol;
    NyLPC_TUInt16 port;
};
/**
 * DNSレコード
 */
struct NyLPC_TDnsRecord
{
    /** Service name*/
    const NyLPC_TChar* name;
    /** Host name recommended name[:emac:]*/
    const NyLPC_TChar* a;
    /**
     * 配列の数
     */
    NyLPC_TUInt16 num_of_srv;
    const struct NyLPC_TMDnsServiceRecord* srv;
};



/**
 * MDNSサーバクラス
 */
typedef struct NyLPC_TcMDnsServer NyLPC_TcMDnsServer_t;



struct NyLPC_TcMDnsServer
{
    /** マルチキャストのUDPソケット*/
    NyLPC_TiUdpSocket_t* _socket;
    /**周期実行タイマ*/
    NyLPC_TcStopwatch_t _periodic_sw;
    /** 動作モード(private)*/
    NyLPC_TUInt8 _state;
    NyLPC_TUInt8 _state_val;
    NyLPC_TUInt8 _padding[2];
    /** DNSレコードの参照情報*/
    const struct NyLPC_TDnsRecord* _ref_record;
};

/**
 * スケッチシステムの場合、この関数はsetup時に実行してください。
 * @param i_ref_record
 * DNSレコードの参照値。インスタンス
 */
NyLPC_TBool NyLPC_cMDnsServer_initialize(
    NyLPC_TcMDnsServer_t* i_inst,const struct NyLPC_TDnsRecord* i_ref_record);

void NyLPC_cMDnsServer_finalize(
    NyLPC_TcMDnsServer_t* i_inst);

void NyLPC_cMDnsServer_periodicRecvProc(NyLPC_TcMDnsServer_t* i_inst);
/**
 * mDNSサービスを開始します。
 * 関数はstop関数をコールするまでの間ブロックします。
 */
void NyLPC_cMDnsServer_start(NyLPC_TcMDnsServer_t* i_inst);
/**
 * mDNSサービスを停止します。
 * 関数はサービスが停止するまでブロックします。
 */
void NyLPC_cMDnsServer_stop(NyLPC_TcMDnsServer_t* i_inst);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
