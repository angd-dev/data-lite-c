/* LibTomCrypt, modular cryptographic library -- Tom St Denis */
/* SPDX-License-Identifier: Unlicense */

#include "tomcrypt_private.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wshorten-64-to-32"

#ifdef LTC_MECC

static int s_ecc_cmp_hex_bn(const char *left_hex, void *right_bn, void *tmp_bn)
{
   if (ltc_mp_read_radix(tmp_bn, left_hex, 16) != CRYPT_OK) return 0;
   if (ltc_mp_cmp(tmp_bn, right_bn) != LTC_MP_EQ)           return 0;
   return 1;
}

static void s_ecc_oid_lookup(ecc_key *key)
{
   void *bn;
   const ltc_ecc_curve *curve;

   key->dp.oidlen = 0;
   if (ltc_mp_init(&bn) != CRYPT_OK) return;
   for (curve = ltc_ecc_curves; curve->prime != NULL; curve++) {
      if (s_ecc_cmp_hex_bn(curve->prime, key->dp.prime,  bn) != 1) continue;
      if (s_ecc_cmp_hex_bn(curve->order, key->dp.order,  bn) != 1) continue;
      if (s_ecc_cmp_hex_bn(curve->A,     key->dp.A,      bn) != 1) continue;
      if (s_ecc_cmp_hex_bn(curve->B,     key->dp.B,      bn) != 1) continue;
      if (s_ecc_cmp_hex_bn(curve->Gx,    key->dp.base.x, bn) != 1) continue;
      if (s_ecc_cmp_hex_bn(curve->Gy,    key->dp.base.y, bn) != 1) continue;
      if (key->dp.cofactor != curve->cofactor)                    continue;
      break; /* found */
   }
   ltc_mp_clear(bn);
   if (curve->prime && curve->OID) {
      key->dp.oidlen = 16; /* size of key->dp.oid */
      pk_oid_str_to_num(curve->OID, key->dp.oid, &key->dp.oidlen);
   }
}

int ecc_copy_curve(const ecc_key *srckey, ecc_key *key)
{
   unsigned long i;
   int err;

   LTC_ARGCHK(key != NULL);
   LTC_ARGCHK(srckey != NULL);

   if ((err = ltc_mp_init_multi(&key->dp.prime, &key->dp.order, &key->dp.A, &key->dp.B,
                            &key->dp.base.x, &key->dp.base.y, &key->dp.base.z,
                            &key->pubkey.x, &key->pubkey.y, &key->pubkey.z, &key->k,
                            NULL)) != CRYPT_OK) {
      return err;
   }

   /* A, B, order, prime, Gx, Gy */
   if ((err = ltc_mp_copy(srckey->dp.prime,  key->dp.prime )) != CRYPT_OK) { goto error; }
   if ((err = ltc_mp_copy(srckey->dp.order,  key->dp.order )) != CRYPT_OK) { goto error; }
   if ((err = ltc_mp_copy(srckey->dp.A,      key->dp.A     )) != CRYPT_OK) { goto error; }
   if ((err = ltc_mp_copy(srckey->dp.B,      key->dp.B     )) != CRYPT_OK) { goto error; }
   if ((err = ltc_ecc_copy_point(&srckey->dp.base, &key->dp.base)) != CRYPT_OK) { goto error; }
   /* cofactor & size */
   key->dp.cofactor = srckey->dp.cofactor;
   key->dp.size     = srckey->dp.size;
   /* OID */
   if (srckey->dp.oidlen > 0) {
     key->dp.oidlen = srckey->dp.oidlen;
     for (i = 0; i < key->dp.oidlen; i++) key->dp.oid[i] = srckey->dp.oid[i];
   }
   else {
     s_ecc_oid_lookup(key); /* try to find OID in ltc_ecc_curves */
   }
   /* success */
   return CRYPT_OK;

error:
   ecc_free(key);
   return err;
}

int ecc_set_curve_from_mpis(void *a, void *b, void *prime, void *order, void *gx, void *gy, unsigned long cofactor, ecc_key *key)
{
   int err;

   LTC_ARGCHK(key   != NULL);
   LTC_ARGCHK(a     != NULL);
   LTC_ARGCHK(b     != NULL);
   LTC_ARGCHK(prime != NULL);
   LTC_ARGCHK(order != NULL);
   LTC_ARGCHK(gx    != NULL);
   LTC_ARGCHK(gy    != NULL);

   if ((err = ltc_mp_init_multi(&key->dp.prime, &key->dp.order, &key->dp.A, &key->dp.B,
                            &key->dp.base.x, &key->dp.base.y, &key->dp.base.z,
                            &key->pubkey.x, &key->pubkey.y, &key->pubkey.z, &key->k,
                            NULL)) != CRYPT_OK) {
      return err;
   }

   /* A, B, order, prime, Gx, Gy */
   if ((err = ltc_mp_copy(prime, key->dp.prime )) != CRYPT_OK) { goto error; }
   if ((err = ltc_mp_copy(order, key->dp.order )) != CRYPT_OK) { goto error; }
   if ((err = ltc_mp_copy(a,     key->dp.A     )) != CRYPT_OK) { goto error; }
   if ((err = ltc_mp_copy(b,     key->dp.B     )) != CRYPT_OK) { goto error; }
   if ((err = ltc_mp_copy(gx,    key->dp.base.x)) != CRYPT_OK) { goto error; }
   if ((err = ltc_mp_copy(gy,    key->dp.base.y)) != CRYPT_OK) { goto error; }
   if ((err = ltc_mp_set(key->dp.base.z, 1)) != CRYPT_OK)      { goto error; }
   /* cofactor & size */
   key->dp.cofactor = cofactor;
   key->dp.size = ltc_mp_unsigned_bin_size(prime);
   /* try to find OID in ltc_ecc_curves */
   s_ecc_oid_lookup(key);
   /* success */
   return CRYPT_OK;

error:
   ecc_free(key);
   return err;
}

#endif

#pragma clang diagnostic pop
