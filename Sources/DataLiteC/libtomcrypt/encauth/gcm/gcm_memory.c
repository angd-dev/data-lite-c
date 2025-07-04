/* LibTomCrypt, modular cryptographic library -- Tom St Denis */
/* SPDX-License-Identifier: Unlicense */

/**
   @file gcm_memory.c
   GCM implementation, process a packet, by Tom St Denis
*/
#include "tomcrypt_private.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wshorten-64-to-32"

#ifdef LTC_GCM_MODE

/**
  Process an entire GCM packet in one call.
  @param cipher            Index of cipher to use
  @param key               The secret key
  @param keylen            The length of the secret key
  @param IV                The initialization vector
  @param IVlen             The length of the initialization vector
  @param adata             The additional authentication data (header)
  @param adatalen          The length of the adata
  @param pt                The plaintext
  @param ptlen             The length of the plaintext (ciphertext length is the same)
  @param ct                The ciphertext
  @param tag               [out] The MAC tag
  @param taglen            [in/out] The MAC tag length
  @param direction         Encrypt or Decrypt mode (GCM_ENCRYPT or GCM_DECRYPT)
  @return CRYPT_OK on success
 */
int gcm_memory(      int           cipher,
               const unsigned char *key,    unsigned long keylen,
               const unsigned char *IV,     unsigned long IVlen,
               const unsigned char *adata,  unsigned long adatalen,
                     unsigned char *pt,     unsigned long ptlen,
                     unsigned char *ct,
                     unsigned char *tag,    unsigned long *taglen,
                               int direction)
{
    void      *orig;
    gcm_state *gcm;
    int        err;

    if ((err = cipher_is_valid(cipher)) != CRYPT_OK) {
       return err;
    }

    if (cipher_descriptor[cipher].accel_gcm_memory != NULL) {
       return cipher_descriptor[cipher].accel_gcm_memory
                                          (key,   keylen,
                                           IV,    IVlen,
                                           adata, adatalen,
                                           pt,    ptlen,
                                           ct,
                                           tag,   taglen,
                                           direction);
    }



#ifndef LTC_GCM_TABLES_SSE2
    orig = gcm = XMALLOC(sizeof(*gcm));
#else
    orig = gcm = XMALLOC(sizeof(*gcm) + 16);
#endif
    if (gcm == NULL) {
        return CRYPT_MEM;
    }

   /* Force GCM to be on a multiple of 16 so we can use 128-bit aligned operations
    * note that we only modify gcm and keep orig intact.  This code is not portable
    * but again it's only for SSE2 anyways, so who cares?
    */
#ifdef LTC_GCM_TABLES_SSE2
    gcm = LTC_ALIGN_BUF(gcm, 16);
#endif

    if ((err = gcm_init(gcm, cipher, key, keylen)) != CRYPT_OK) {
       goto LTC_ERR;
    }
    if ((err = gcm_add_iv(gcm, IV, IVlen)) != CRYPT_OK) {
       goto LTC_ERR;
    }
    if ((err = gcm_add_aad(gcm, adata, adatalen)) != CRYPT_OK) {
       goto LTC_ERR;
    }
    if ((err = gcm_process(gcm, pt, ptlen, ct, direction)) != CRYPT_OK) {
       goto LTC_ERR;
    }
    if (direction == GCM_ENCRYPT) {
      if ((err = gcm_done(gcm, tag, taglen)) != CRYPT_OK) {
         goto LTC_ERR;
      }
    }
    else if (direction == GCM_DECRYPT) {
       unsigned char buf[MAXBLOCKSIZE];
       unsigned long buflen = sizeof(buf);
       if ((err = gcm_done(gcm, buf, &buflen)) != CRYPT_OK) {
          goto LTC_ERR;
       }
       if (buflen != *taglen || XMEM_NEQ(buf, tag, buflen) != 0) {
          err = CRYPT_ERROR;
       }
    }
    else {
       err = CRYPT_INVALID_ARG;
    }
LTC_ERR:
    gcm_reset(gcm);
    XFREE(orig);
    return err;
}
#endif

#pragma clang diagnostic pop
