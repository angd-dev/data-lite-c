/* LibTomCrypt, modular cryptographic library -- Tom St Denis */
/* SPDX-License-Identifier: Unlicense */
#include "tomcrypt_private.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wshorten-64-to-32"

/**
  @file omac_init.c
  OMAC1 support, initialize state, by Tom St Denis
*/


#ifdef LTC_OMAC

/**
   Initialize an OMAC state
   @param omac    The OMAC state to initialize
   @param cipher  The index of the desired cipher
   @param key     The secret key
   @param keylen  The length of the secret key (octets)
   @return CRYPT_OK if successful
*/
int omac_init(omac_state *omac, int cipher, const unsigned char *key, unsigned long keylen)
{
   int err, x, y, mask, msb, len;

   LTC_ARGCHK(omac != NULL);
   LTC_ARGCHK(key  != NULL);

   /* schedule the key */
   if ((err = cipher_is_valid(cipher)) != CRYPT_OK) {
      return err;
   }

#ifdef LTC_FAST
   if (cipher_descriptor[cipher].block_length % sizeof(LTC_FAST_TYPE)) {
       return CRYPT_INVALID_ARG;
   }
#endif

   /* now setup the system */
   switch (cipher_descriptor[cipher].block_length) {
       case 8:  mask = 0x1B;
                len  = 8;
                break;
       case 16: mask = 0x87;
                len  = 16;
                break;
       default: return CRYPT_INVALID_ARG;
   }

   if ((err = cipher_descriptor[cipher].setup(key, keylen, 0, &omac->key)) != CRYPT_OK) {
      return err;
   }

   /* ok now we need Lu and Lu^2 [calc one from the other] */

   /* first calc L which is Ek(0) */
   zeromem(omac->Lu[0], cipher_descriptor[cipher].block_length);
   if ((err = cipher_descriptor[cipher].ecb_encrypt(omac->Lu[0], omac->Lu[0], &omac->key)) != CRYPT_OK) {
      return err;
   }

   /* now do the mults, whoopy! */
   for (x = 0; x < 2; x++) {
       /* if msb(L * u^(x+1)) = 0 then just shift, otherwise shift and xor constant mask */
       msb = omac->Lu[x][0] >> 7;

       /* shift left */
       for (y = 0; y < (len - 1); y++) {
           omac->Lu[x][y] = ((omac->Lu[x][y] << 1) | (omac->Lu[x][y+1] >> 7)) & 255;
       }
       omac->Lu[x][len - 1] = ((omac->Lu[x][len - 1] << 1) ^ (msb ? mask : 0)) & 255;

       /* copy up as require */
       if (x == 0) {
          XMEMCPY(omac->Lu[1], omac->Lu[0], sizeof(omac->Lu[0]));
       }
   }

   /* setup state */
   omac->cipher_idx = cipher;
   omac->buflen     = 0;
   omac->blklen     = len;
   zeromem(omac->prev,  sizeof(omac->prev));
   zeromem(omac->block, sizeof(omac->block));

   return CRYPT_OK;
}

#endif

#pragma clang diagnostic pop
