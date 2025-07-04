/* LibTomCrypt, modular cryptographic library -- Tom St Denis */
/* SPDX-License-Identifier: Unlicense */

#include "tomcrypt_private.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wshorten-64-to-32"

#if defined(LTC_MECC) && defined(LTC_ECC_SHAMIR)

/**
  @file ecc_recover_key.c
  ECC Crypto, Russ Williams
*/

/**
   Recover ECC public key from signature and hash
   @param sig         The signature to verify
   @param siglen      The length of the signature (octets)
   @param hash        The hash (message digest) that was signed
   @param hashlen     The length of the hash (octets)
   @param recid       The recovery ID ("v"), can be -1 if signature contains it
   @param sigformat   The format of the signature (ecc_signature_type)
   @param key         The recovered public ECC key
   @return CRYPT_OK if successful (even if the signature is not valid)
*/
int ecc_recover_key(const unsigned char *sig,  unsigned long siglen,
                    const unsigned char *hash, unsigned long hashlen,
                    int recid, ecc_signature_type sigformat, ecc_key *key)
{
   ecc_point     *mG = NULL, *mQ = NULL, *mR = NULL;
   void          *p, *m, *a, *b;
   void          *r, *s, *v, *w, *t1, *t2, *u1, *u2, *v1, *v2, *e, *x, *y, *a_plus3;
   void          *mu = NULL, *ma = NULL;
   void          *mp = NULL;
   int           err;
   unsigned long pbits, pbytes, i, shift_right;
   unsigned char ch, buf[MAXBLOCKSIZE];

   LTC_ARGCHK(sig  != NULL);
   LTC_ARGCHK(hash != NULL);
   LTC_ARGCHK(key  != NULL);

   /* BEWARE: requires sqrtmod_prime */
   if (ltc_mp.sqrtmod_prime == NULL) {
      return CRYPT_ERROR;
   }

   /* allocate ints */
   if ((err = ltc_mp_init_multi(&r, &s, &v, &w, &t1, &t2, &u1, &u2, &v1, &v2, &e, &x, &y, &a_plus3, LTC_NULL)) != CRYPT_OK) {
      return err;
   }

   p = key->dp.order;
   m = key->dp.prime;
   a = key->dp.A;
   b = key->dp.B;
   if ((err = ltc_mp_add_d(a, 3, a_plus3)) != CRYPT_OK) {
      goto error;
   }

   /* allocate points */
   mG = ltc_ecc_new_point();
   mQ = ltc_ecc_new_point();
   mR = ltc_ecc_new_point();
   if (mR == NULL || mQ  == NULL || mG == NULL) {
      err = CRYPT_MEM;
      goto error;
   }

   if (sigformat == LTC_ECCSIG_RFC7518) {
      /* RFC7518 format - raw (r,s) */
      i = ltc_mp_unsigned_bin_size(key->dp.order);
      if (siglen != (2*i)) {
         err = CRYPT_INVALID_PACKET;
         goto error;
      }
      if ((err = ltc_mp_read_unsigned_bin(r, sig,   i)) != CRYPT_OK)                                    { goto error; }
      if ((err = ltc_mp_read_unsigned_bin(s, sig+i, i)) != CRYPT_OK)                                    { goto error; }
   }
   else if (sigformat == LTC_ECCSIG_ETH27) {
      /* Ethereum (v,r,s) format */
      if (pk_oid_cmp_with_ulong("1.3.132.0.10", key->dp.oid, key->dp.oidlen) != CRYPT_OK) {
         /* Only valid for secp256k1 - OID 1.3.132.0.10 */
         err = CRYPT_ERROR; goto error;
      }
      if (siglen != 65) { /* Only secp256k1 curves use this format, so must be 65 bytes long */
         err = CRYPT_INVALID_PACKET;
         goto error;
      }
      i = (unsigned long)sig[64];
      if ((i>=27) && (i<31)) i -= 27; /* Ethereum adds 27 to recovery ID */
      if (recid >= 0 && ((unsigned long)recid != i)) {
         /* Recovery ID specified, but doesn't match signature */
         err = CRYPT_INVALID_PACKET;
         goto error;
      }
      recid = i;
      if ((err = ltc_mp_read_unsigned_bin(r, sig,  32)) != CRYPT_OK)                                    { goto error; }
      if ((err = ltc_mp_read_unsigned_bin(s, sig+32, 32)) != CRYPT_OK)                                  { goto error; }
   }
#ifdef LTC_DER
   else if (sigformat == LTC_ECCSIG_ANSIX962) {
         /* ANSI X9.62 format - ASN.1 encoded SEQUENCE{ INTEGER(r), INTEGER(s) }  */
         if ((err = der_decode_sequence_multi_ex(sig, siglen, LTC_DER_SEQ_SEQUENCE | LTC_DER_SEQ_STRICT,
                                                 LTC_ASN1_INTEGER, 1UL, r,
                                                 LTC_ASN1_INTEGER, 1UL, s,
                                                 LTC_ASN1_EOL, 0UL, LTC_NULL)) != CRYPT_OK)             { goto error; }
   }
#endif
#ifdef LTC_SSH
   else if (sigformat == LTC_ECCSIG_RFC5656) {
      char name[64], name2[64];
      unsigned long namelen = sizeof(name);
      unsigned long name2len = sizeof(name2);

      /* Decode as SSH data sequence, per RFC4251 */
      if ((err = ssh_decode_sequence_multi(sig, &siglen,
                                           LTC_SSHDATA_STRING, name, &namelen,
                                           LTC_SSHDATA_MPINT,  r,
                                           LTC_SSHDATA_MPINT,  s,
                                           LTC_SSHDATA_EOL,    LTC_NULL)) != CRYPT_OK)                  { goto error; }


      /* Check curve matches identifier string */
      if ((err = ecc_ssh_ecdsa_encode_name(name2, &name2len, key)) != CRYPT_OK)                         { goto error; }
      if ((namelen != name2len) || (XSTRCMP(name, name2) != 0)) {
         err = CRYPT_INVALID_ARG;
         goto error;
      }
   }
#endif
   else {
      /* Unknown signature format */
      err = CRYPT_ERROR;
      goto error;
   }

   if (recid < 0 || (unsigned long)recid >= 2*(key->dp.cofactor+1)) {
      /* Recovery ID is out of range, reject it */
      err = CRYPT_INVALID_ARG;
      goto error;
   }

   /* check for zero */
   if (ltc_mp_cmp_d(r, 0) != LTC_MP_GT || ltc_mp_cmp_d(s, 0) != LTC_MP_GT ||
       ltc_mp_cmp(r, p) != LTC_MP_LT || ltc_mp_cmp(s, p) != LTC_MP_LT) {
      err = CRYPT_INVALID_PACKET;
      goto error;
   }

   /* read hash - truncate if needed */
   pbits = ltc_mp_count_bits(p);
   pbytes = (pbits+7) >> 3;
   if (pbits > hashlen*8) {
      if ((err = ltc_mp_read_unsigned_bin(e, hash, hashlen)) != CRYPT_OK)                                   { goto error; }
   }
   else if (pbits % 8 == 0) {
      if ((err = ltc_mp_read_unsigned_bin(e, hash, pbytes)) != CRYPT_OK)                                    { goto error; }
   }
   else {
      shift_right = 8 - pbits % 8;
      for (i=0, ch=0; i<pbytes; i++) {
        buf[i] = ch;
        ch = (hash[i] << (8-shift_right));
        buf[i] = buf[i] ^ (hash[i] >> shift_right);
      }
      if ((err = ltc_mp_read_unsigned_bin(e, buf, pbytes)) != CRYPT_OK)                                     { goto error; }
   }

   /* decompress point from r=(x mod p) - BEWARE: requires sqrtmod_prime */
   /* x = r + p*(recid/2) */
   if ((err = ltc_mp_set(x, recid/2)) != CRYPT_OK)                                                          { goto error; }
   if ((err = ltc_mp_mulmod(p, x, m, x)) != CRYPT_OK)                                                       { goto error; }
   if ((err = ltc_mp_add(x, r, x)) != CRYPT_OK)                                                             { goto error; }
   /* compute x^3 */
   if ((err = ltc_mp_sqr(x, t1)) != CRYPT_OK)                                                               { goto error; }
   if ((err = ltc_mp_mulmod(t1, x, m, t1)) != CRYPT_OK)                                                     { goto error; }
   /* compute x^3 + a*x */
   if ((err = ltc_mp_mulmod(a, x, m, t2)) != CRYPT_OK)                                                      { goto error; }
   if ((err = ltc_mp_add(t1, t2, t1)) != CRYPT_OK)                                                          { goto error; }
   /* compute x^3 + a*x + b */
   if ((err = ltc_mp_add(t1, b, t1)) != CRYPT_OK)                                                           { goto error; }
   /* compute sqrt(x^3 + a*x + b) */
   if ((err = ltc_mp_sqrtmod_prime(t1, m, t2)) != CRYPT_OK)                                                 { goto error; }

   /* fill in mR */
   if ((err = ltc_mp_copy(x, mR->x)) != CRYPT_OK)                                                           { goto error; }
   if ((ltc_mp_isodd(t2) && (recid%2)) || (!ltc_mp_isodd(t2) && !(recid%2))) {
      if ((err = ltc_mp_mod(t2, m, mR->y)) != CRYPT_OK)                                                     { goto error; }
   }
   else {
      if ((err = ltc_mp_submod(m, t2, m, mR->y)) != CRYPT_OK)                                               { goto error; }
   }
   if ((err = ltc_mp_set(mR->z, 1)) != CRYPT_OK)                                                            { goto error; }

   /*  w  = r^-1 mod n */
   if ((err = ltc_mp_invmod(r, p, w)) != CRYPT_OK)                                                          { goto error; }
   /* v1 = sw */
   if ((err = ltc_mp_mulmod(s, w, p, v1)) != CRYPT_OK)                                                      { goto error; }
   /* v2 = -ew */
   if ((err = ltc_mp_mulmod(e, w, p, v2)) != CRYPT_OK)                                                      { goto error; }
   if ((err = ltc_mp_submod(p, v2, p, v2)) != CRYPT_OK)                                                     { goto error; }

   /*  w  = s^-1 mod n */
   if ((err = ltc_mp_invmod(s, p, w)) != CRYPT_OK)                                                          { goto error; }
   /* u1 = ew */
   if ((err = ltc_mp_mulmod(e, w, p, u1)) != CRYPT_OK)                                                      { goto error; }
   /* u2 = rw */
   if ((err = ltc_mp_mulmod(r, w, p, u2)) != CRYPT_OK)                                                      { goto error; }

   /* find mG */
   if ((err = ltc_ecc_copy_point(&key->dp.base, mG)) != CRYPT_OK)                                           { goto error; }

   /* find the montgomery mp */
   if ((err = ltc_mp_montgomery_setup(m, &mp)) != CRYPT_OK)                                                 { goto error; }

   /* for curves with a == -3 keep ma == NULL */
   if (ltc_mp_cmp(a_plus3, m) != LTC_MP_EQ) {
      if ((err = ltc_mp_init_multi(&mu, &ma, LTC_NULL)) != CRYPT_OK)                                        { goto error; }
      if ((err = ltc_mp_montgomery_normalization(mu, m)) != CRYPT_OK)                                       { goto error; }
      if ((err = ltc_mp_mulmod(a, mu, m, ma)) != CRYPT_OK)                                                  { goto error; }
   }

   /* recover mQ from mR */
   /* compute v1*mR + v2*mG = mQ using Shamir's trick */
   if ((err = ltc_mp.ecc_mul2add(mR, v1, mG, v2, mQ, ma, m)) != CRYPT_OK)                                   { goto error; }

   /* compute u1*mG + u2*mQ = mG using Shamir's trick */
   if ((err = ltc_mp.ecc_mul2add(mG, u1, mQ, u2, mG, ma, m)) != CRYPT_OK)                                   { goto error; }

   /* v = X_x1 mod n */
   if ((err = ltc_mp_mod(mG->x, p, v)) != CRYPT_OK)                                                         { goto error; }

   /* does v == r */
   if (ltc_mp_cmp(v, r) == LTC_MP_EQ) {
      /* found public key which verifies signature */
      if ((err = ltc_ecc_copy_point(mQ, &key->pubkey)) != CRYPT_OK)                                         { goto error; }
      /* point on the curve + other checks */
      if ((err = ltc_ecc_verify_key(key)) != CRYPT_OK)                                                      { goto error; }

      key->type = PK_PUBLIC;

      err = CRYPT_OK;
   }
   else {
      /* not found - recid is wrong or we're unable to calculate public key for some other reason */
      err = CRYPT_INVALID_ARG;
   }

error:
   if (ma != NULL) ltc_mp_clear(ma);
   if (mu != NULL) ltc_mp_clear(mu);
   if (mp != NULL) ltc_mp_montgomery_free(mp);
   if (mR != NULL) ltc_ecc_del_point(mR);
   if (mQ != NULL) ltc_ecc_del_point(mQ);
   if (mG != NULL) ltc_ecc_del_point(mG);
   ltc_mp_deinit_multi(a_plus3, y, x, e, v2, v1, u2, u1, t2, t1, w, v, s, r, LTC_NULL);
   return err;
}

#endif

#pragma clang diagnostic pop
