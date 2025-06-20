/* LibTomCrypt, modular cryptographic library -- Tom St Denis */
/* SPDX-License-Identifier: Unlicense */

/**
   @file ocb3_decrypt.c
   OCB implementation, decrypt data, by Tom St Denis
*/
#include "tomcrypt_private.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wshorten-64-to-32"

#ifdef LTC_OCB3_MODE

/**
   Decrypt blocks of ciphertext with OCB
   @param ocb     The OCB state
   @param ct      The ciphertext (length multiple of the block size of the block cipher)
   @param ctlen   The length of the input (octets)
   @param pt      [out] The plaintext (length of ct)
   @return CRYPT_OK if successful
*/
int ocb3_decrypt(ocb3_state *ocb, const unsigned char *ct, unsigned long ctlen, unsigned char *pt)
{
   unsigned char tmp[MAXBLOCKSIZE];
   int err, i, full_blocks;
   unsigned char *pt_b, *ct_b;

   LTC_ARGCHK(ocb != NULL);
   if (ctlen == 0) return CRYPT_OK; /* no data, nothing to do */
   LTC_ARGCHK(ct != NULL);
   LTC_ARGCHK(pt != NULL);

   if ((err = cipher_is_valid(ocb->cipher)) != CRYPT_OK) {
      return err;
   }
   if (ocb->block_len != cipher_descriptor[ocb->cipher].block_length) {
      return CRYPT_INVALID_ARG;
   }

   if (ctlen % ocb->block_len) { /* ctlen has to bu multiple of block_len */
      return CRYPT_INVALID_ARG;
   }

   full_blocks = ctlen/ocb->block_len;
   for(i=0; i<full_blocks; i++) {
     pt_b = (unsigned char *)pt+i*ocb->block_len;
     ct_b = (unsigned char *)ct+i*ocb->block_len;

     /* ocb->Offset_current[] = ocb->Offset_current[] ^ Offset_{ntz(block_index)} */
     ocb3_int_xor_blocks(ocb->Offset_current, ocb->Offset_current, ocb->L_[ocb3_int_ntz(ocb->block_index)], ocb->block_len);

     /* tmp[] = ct[] XOR ocb->Offset_current[] */
     ocb3_int_xor_blocks(tmp, ct_b, ocb->Offset_current, ocb->block_len);

     /* decrypt */
     if ((err = cipher_descriptor[ocb->cipher].ecb_decrypt(tmp, tmp, &ocb->key)) != CRYPT_OK) {
        goto LBL_ERR;
     }

     /* pt[] = tmp[] XOR ocb->Offset_current[] */
     ocb3_int_xor_blocks(pt_b, tmp, ocb->Offset_current, ocb->block_len);

     /* ocb->checksum[] = ocb->checksum[] XOR pt[] */
     ocb3_int_xor_blocks(ocb->checksum, ocb->checksum, pt_b, ocb->block_len);

     ocb->block_index++;
   }

   err = CRYPT_OK;

LBL_ERR:
#ifdef LTC_CLEAN_STACK
   zeromem(tmp, sizeof(tmp));
#endif
   return err;
}

#endif

#pragma clang diagnostic pop
