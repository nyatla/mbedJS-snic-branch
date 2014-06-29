#ifndef NYLPC_CIPV4_TYPEDEF_H_
#define NYLPC_CIPV4_TYPEDEF_H_

/**
 * クラス型を定義します。
 * NyLPC_cIPv4クラスは、NyLPC_cUipServiceクラスの一部として働きます。
 * 通常ユーザが操作することはありません。
 * IPv4における、ソケットクラス(NyLPC_cTcpSocketとNyLPC_cTcpListener)の管理を担当します。
 * クラスのインスタンスは、NyLPC_cUipServiceのインスタンスにより生成されます。
 * インスタンスは2つのポインタリストをもち、ここにこれらのインスタンスを登録します。
 * インスタンスは、NyLPC_cUipServiceから送られてきた受信パケットを、登録されているソケットクラスに
 * ディスパッチする機能を持ちます。
 */
typedef struct NyLPC_TcIPv4 NyLPC_TcIPv4_t;

#endif /* NYLPC_CIPV4_PUBLIC_H_ */

