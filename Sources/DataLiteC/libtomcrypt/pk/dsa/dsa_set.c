/* LibTomCrypt, modular cryptographic library -- Tom St Denis */
/* SPDX-License-Identifier: Unlicense */
#include "tomcrypt_private.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wshorten-64-to-32"

#ifdef LTC_MDSA

/**
  Import DSA's p, q & g from raw numbers
  @param p       DSA's p  in binary representation
  @param plen    The length of p
  @param q       DSA's q  in binary representation
  @param qlen    The length of q
  @param g       DSA's g  in binary representation
  @param glen    The length of g
  @param key     [out] the destination for the imported key
  @return CRYPT_OK if successful.
*/
int dsa_set_pqg(const unsigned char *p,  unsigned long plen,
                const unsigned char *q,  unsigned long qlen,
                const unsigned char *g,  unsigned long glen,
                dsa_key *key)
{
   int err, stat;

   LTC_ARGCHK(p           != NULL);
   LTC_ARGCHK(q           != NULL);
   LTC_ARGCHK(g           != NULL);

   /* init key */
   if ((err = dsa_int_init(key)) != CRYPT_OK) return err;

   if ((err = ltc_mp_read_unsigned_bin(key->p, p , plen)) != CRYPT_OK)              { goto LBL_ERR; }
   if ((err = ltc_mp_read_unsigned_bin(key->g, g , glen)) != CRYPT_OK)              { goto LBL_ERR; }
   if ((err = ltc_mp_read_unsigned_bin(key->q, q , qlen)) != CRYPT_OK)              { goto LBL_ERR; }

   key->qord = ltc_mp_unsigned_bin_size(key->q);

   /* do only a quick validation, without primality testing */
   if ((err = dsa_int_validate_pqg(key, &stat)) != CRYPT_OK)                        { goto LBL_ERR; }
   if (stat == 0) {
      err = CRYPT_INVALID_PACKET;
      goto LBL_ERR;
   }

   return CRYPT_OK;

LBL_ERR:
   dsa_free(key);
   return err;
}

/**
  Import DSA public or private key-part from raw numbers

     NB: The p, q & g parts must be set beforehand

  @param in      The key-part to import, either public or private.
  @param inlen   The key-part's length
  @param type    Which type of key (PK_PRIVATE or PK_PUBLIC)
  @param key     [out] the destination for the imported key
  @return CRYPT_OK if successful.
*/
int dsa_set_key(const unsigned char *in, unsigned long inlen, int type, dsa_key *key)
{
   int err, stat = 0;

   LTC_ARGCHK(key         != NULL);
   LTC_ARGCHK(key->x      != NULL);
   LTC_ARGCHK(key->y      != NULL);
   LTC_ARGCHK(key->p      != NULL);
   LTC_ARGCHK(key->g      != NULL);
   LTC_ARGCHK(key->q      != NULL);
   LTC_ARGCHK(ltc_mp.name != NULL);

   if (type == PK_PRIVATE) {
      key->type = PK_PRIVATE;
      if ((err = ltc_mp_read_unsigned_bin(key->x, in, inlen)) != CRYPT_OK)              { goto LBL_ERR; }
      if ((err = ltc_mp_exptmod(key->g, key->x, key->p, key->y)) != CRYPT_OK)           { goto LBL_ERR; }
   }
   else {
      key->type = PK_PUBLIC;
      if ((err = ltc_mp_read_unsigned_bin(key->y, in, inlen)) != CRYPT_OK)              { goto LBL_ERR; }
   }

   if ((err = dsa_int_validate_xy(key, &stat)) != CRYPT_OK)                             { goto LBL_ERR; }
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
