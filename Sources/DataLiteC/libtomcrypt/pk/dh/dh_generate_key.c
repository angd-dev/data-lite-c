/* LibTomCrypt, modular cryptographic library -- Tom St Denis */
/* SPDX-License-Identifier: Unlicense */

#include "tomcrypt_private.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wshorten-64-to-32"

#ifdef LTC_MDH

static int s_dh_groupsize_to_keysize(int groupsize)
{
   /* The strength estimates from https://tools.ietf.org/html/rfc3526#section-8
    * We use "Estimate 2" to get an appropriate private key (exponent) size.
    */
   if (groupsize <= 0) {
      return 0;
   }
   if (groupsize <= 192) {
      return 30;     /* 1536-bit => key size 240-bit */
   }
   if (groupsize <= 256) {
      return 40;     /* 2048-bit => key size 320-bit */
   }
   if (groupsize <= 384) {
      return 52;     /* 3072-bit => key size 416-bit */
   }
   if (groupsize <= 512) {
      return 60;     /* 4096-bit => key size 480-bit */
   }
   if (groupsize <= 768) {
      return 67;     /* 6144-bit => key size 536-bit */
   }
   if (groupsize <= 1024) {
      return 77;     /* 8192-bit => key size 616-bit */
   }
   return 0;
}

int dh_generate_key(prng_state *prng, int wprng, dh_key *key)
{
   unsigned char *buf;
   unsigned long keysize;
   int err, max_iterations = LTC_PK_MAX_RETRIES;

   LTC_ARGCHK(key         != NULL);
   LTC_ARGCHK(ltc_mp.name != NULL);

   /* good prng? */
   if ((err = prng_is_valid(wprng)) != CRYPT_OK) {
      return err;
   }

   keysize = s_dh_groupsize_to_keysize(ltc_mp_unsigned_bin_size(key->prime));
   if (keysize == 0) {
      err = CRYPT_INVALID_KEYSIZE;
      goto freemp;
   }

   /* allocate buffer */
   buf = XMALLOC(keysize);
   if (buf == NULL) {
      err = CRYPT_MEM;
      goto freemp;
   }

   key->type = PK_PRIVATE;
   do {
      /* make up random buf */
      if (prng_descriptor[wprng].read(buf, keysize, prng) != keysize) {
         err = CRYPT_ERROR_READPRNG;
         goto freebuf;
      }
      /* load the x value - private key */
      if ((err = ltc_mp_read_unsigned_bin(key->x, buf, keysize)) != CRYPT_OK) {
         goto freebuf;
      }
      /* compute the y value - public key */
      if ((err = ltc_mp_exptmod(key->base, key->x, key->prime, key->y)) != CRYPT_OK) {
         goto freebuf;
      }
      err = dh_check_pubkey(key);
   } while (err != CRYPT_OK && max_iterations-- > 0);

freebuf:
   zeromem(buf, keysize);
   XFREE(buf);
freemp:
   if (err != CRYPT_OK) dh_free(key);
   return err;
}

#endif /* LTC_MDH */

#pragma clang diagnostic pop
