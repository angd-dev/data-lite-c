/* LibTomCrypt, modular cryptographic library -- Tom St Denis */
/* SPDX-License-Identifier: Unlicense */
#include "tomcrypt_private.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wshorten-64-to-32"

/**
  @file crypt_find_hash_any.c
  Find a hash, Tom St Denis
*/

/**
   Find a hash flexibly.  First by name then if not present by digest size
   @param name        The name of the hash desired
   @param digestlen   The minimum length of the digest size (octets)
   @return >= 0 if found, -1 if not present
*/int find_hash_any(const char *name, int digestlen)
{
   int x, y, z;
   LTC_ARGCHK(name != NULL);

   x = find_hash(name);
   if (x != -1) return x;

   LTC_MUTEX_LOCK(&ltc_hash_mutex);
   y = MAXBLOCKSIZE+1;
   z = -1;
   for (x = 0; x < TAB_SIZE; x++) {
       if (hash_descriptor[x].name == NULL) {
          continue;
       }
       if ((int)hash_descriptor[x].hashsize >= digestlen && (int)hash_descriptor[x].hashsize < y) {
          z = x;
          y = hash_descriptor[x].hashsize;
       }
   }
   LTC_MUTEX_UNLOCK(&ltc_hash_mutex);
   return z;
}

#pragma clang diagnostic pop
