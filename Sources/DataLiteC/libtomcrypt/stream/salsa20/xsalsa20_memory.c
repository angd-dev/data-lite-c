/* LibTomCrypt, modular cryptographic library -- Tom St Denis */
/* SPDX-License-Identifier: Unlicense */

#include "tomcrypt_private.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wshorten-64-to-32"

#ifdef LTC_XSALSA20

/**
   Encrypt (or decrypt) bytes of ciphertext (or plaintext) with XSalsa20
   @param key      The key
   @param keylen   The key length
   @param nonce    The initial vector
   @param noncelen The initial vector length
   @param datain   The plaintext (or ciphertext)
   @param datalen  The length of the input and output (octets)
   @param rounds   The number of rounds
   @param dataout  [out] The ciphertext (or plaintext)
   @return CRYPT_OK if successful
*/
int xsalsa20_memory(const unsigned char *key,    unsigned long keylen,   unsigned long rounds,
                    const unsigned char *nonce,  unsigned long noncelen,
                    const unsigned char *datain, unsigned long datalen,  unsigned char *dataout)
{
   salsa20_state st;
   int err;

   if ((err = xsalsa20_setup(&st, key, keylen, nonce, noncelen, rounds)) != CRYPT_OK) goto WIPE_KEY;
   err = salsa20_crypt(&st, datain, datalen, dataout);
WIPE_KEY:
   salsa20_done(&st);
   return err;
}

#endif /* LTC_XSALSA20 */

#pragma clang diagnostic pop
