/* LibTomCrypt, modular cryptographic library -- Tom St Denis */
/* SPDX-License-Identifier: Unlicense */

#include "tomcrypt_private.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wshorten-64-to-32"

/**
  @file ecc_sizes.c
  ECC Crypto, Tom St Denis
*/

#ifdef LTC_MECC

void ecc_sizes(int *low, int *high)
{
  int i, size;
  void *prime;

  LTC_ARGCHKVD(low  != NULL);
  LTC_ARGCHKVD(high != NULL);

  *low = INT_MAX;
  *high = 0;

  if (ltc_mp_init(&prime) == CRYPT_OK) {
    for (i = 0; ltc_ecc_curves[i].prime != NULL; i++) {
       if (ltc_mp_read_radix(prime, ltc_ecc_curves[i].prime, 16) == CRYPT_OK) {
         size = ltc_mp_unsigned_bin_size(prime);
         if (size < *low)  *low  = size;
         if (size > *high) *high = size;
       }
    }
    ltc_mp_clear(prime);
  }
}

#endif

#pragma clang diagnostic pop
