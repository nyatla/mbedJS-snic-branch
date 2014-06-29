/* sha1.h */
#include "NyLPC_stdlib.h"
/* If OpenSSL is in use, then use that version of SHA-1 */
#ifdef OPENSSL
#include <t_sha.h>
#define __SHA1_INCLUDE_
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef __SHA1_INCLUDE_

#ifndef SHA1_SIGNATURE_SIZE
#ifdef SHA_DIGESTSIZE
#define SHA1_SIGNATURE_SIZE SHA_DIGESTSIZE
#else
#define SHA1_SIGNATURE_SIZE 20
#endif
#endif


typedef struct {
    NyLPC_TUInt32 state[5];
    NyLPC_TUInt32 count[2];
    unsigned char buffer[64];
}SHA1_CTX;

void SHA1Init(SHA1_CTX* context);
void SHA1Update(SHA1_CTX* context,const void* data, unsigned int len);
void SHA1Final(unsigned char digest[20], SHA1_CTX* context);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#define __SHA1_INCLUDE_
#endif /* __SHA1_INCLUDE_ */

