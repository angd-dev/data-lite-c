/* LibTomCrypt, modular cryptographic library -- Tom St Denis */
/* SPDX-License-Identifier: Unlicense */
#include "tomcrypt_private.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wshorten-64-to-32"

#ifdef LTC_MDSA

/**
  Import DSA's p, q & g from dsaparam

      dsaparam data: openssl dsaparam -outform DER -out dsaparam.der 2048

  @param dsaparam    The DSA param DER encoded data
  @param dsaparamlen The length of dhparam data
  @param key         [out] the destination for the imported key
  @return CRYPT_OK if successful.
*/
int dsa_set_pqg_dsaparam(const unsigned char *dsaparam, unsigned long dsaparamlen,
                         dsa_key *key)
{
   int err, stat;

   LTC_ARGCHK(dsaparam    != NULL);
   LTC_ARGCHK(key         != NULL);
   LTC_ARGCHK(ltc_mp.name != NULL);

   /* init key */
   if ((err = dsa_int_init(key)) != CRYPT_OK) return err;

   if ((err = der_decode_sequence_multi(dsaparam, dsaparamlen,
                                        LTC_ASN1_INTEGER, 1UL, key->p,
                                        LTC_ASN1_INTEGER, 1UL, key->q,
                                        LTC_ASN1_INTEGER, 1UL, key->g,
                                        LTC_ASN1_EOL,     0UL, NULL)) != CRYPT_OK) {
      goto LBL_ERR;
   }

   key->qord = ltc_mp_unsigned_bin_size(key->q);

   /* quick p, q, g validation, without primality testing */
   if ((err = dsa_int_validate_pqg(key, &stat)) != CRYPT_OK) {
      goto LBL_ERR;
   }
   if (stat == 0) {
      err = CRYPT_INVALID_PACKET;
      goto LBL_ERR;
   }

   return CRYPT_OK;

LBL_ERR:
   dsa_free(key);
   return err;
}

#endif

#pragma clang diagnostic pop
