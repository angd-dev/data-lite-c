/* LibTomCrypt, modular cryptographic library -- Tom St Denis */
/* SPDX-License-Identifier: Unlicense */
#include "tomcrypt_private.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wshorten-64-to-32"

/**
  @file der_encode_sequence_multi.c
  ASN.1 DER, encode a SEQUENCE, Tom St Denis
*/

#ifdef LTC_DER

/**
  Encode a SEQUENCE type using a VA list
  @param out    [out] Destination for data
  @param outlen [in/out] Length of buffer and resulting length of output
  @remark <...> is of the form <type, size, data> (int, unsigned long, void*)
  @return CRYPT_OK on success
*/
int der_encode_sequence_multi(unsigned char *out, unsigned long *outlen, ...)
{
   int           err;
   ltc_asn1_type type;
   unsigned long size, x;
   void          *data;
   va_list       args;
   ltc_asn1_list *list;

   LTC_ARGCHK(out    != NULL);
   LTC_ARGCHK(outlen != NULL);

   /* get size of output that will be required */
   va_start(args, outlen);
   x = 0;
   for (;;) {
       type = (ltc_asn1_type)va_arg(args, int);

       if (type == LTC_ASN1_EOL) {
          break;
       }

       size = va_arg(args, unsigned long);
       data = va_arg(args, void*);
       LTC_UNUSED_PARAM(size);
       LTC_UNUSED_PARAM(data);

       switch (type) {
           case LTC_ASN1_BOOLEAN:
           case LTC_ASN1_INTEGER:
           case LTC_ASN1_SHORT_INTEGER:
           case LTC_ASN1_BIT_STRING:
           case LTC_ASN1_OCTET_STRING:
           case LTC_ASN1_NULL:
           case LTC_ASN1_OBJECT_IDENTIFIER:
           case LTC_ASN1_IA5_STRING:
           case LTC_ASN1_PRINTABLE_STRING:
           case LTC_ASN1_UTF8_STRING:
           case LTC_ASN1_UTCTIME:
           case LTC_ASN1_SEQUENCE:
           case LTC_ASN1_SET:
           case LTC_ASN1_SETOF:
           case LTC_ASN1_RAW_BIT_STRING:
           case LTC_ASN1_GENERALIZEDTIME:
                ++x;
                break;

           case LTC_ASN1_CHOICE:
           case LTC_ASN1_CUSTOM_TYPE:
           case LTC_ASN1_EOL:
           case LTC_ASN1_TELETEX_STRING:
               va_end(args);
               return CRYPT_INVALID_ARG;
       }
   }
   va_end(args);

   /* allocate structure for x elements */
   if (x == 0) {
      return CRYPT_NOP;
   }

   list = XCALLOC(x, sizeof(*list));
   if (list == NULL) {
      return CRYPT_MEM;
   }

   /* fill in the structure */
   va_start(args, outlen);
   x = 0;
   for (;;) {
       type = (ltc_asn1_type)va_arg(args, int);

       if (type == LTC_ASN1_EOL) {
          break;
       }

       size = va_arg(args, unsigned long);
       data = va_arg(args, void*);

       switch (type) {
           case LTC_ASN1_BOOLEAN:
           case LTC_ASN1_INTEGER:
           case LTC_ASN1_SHORT_INTEGER:
           case LTC_ASN1_BIT_STRING:
           case LTC_ASN1_OCTET_STRING:
           case LTC_ASN1_NULL:
           case LTC_ASN1_OBJECT_IDENTIFIER:
           case LTC_ASN1_IA5_STRING:
           case LTC_ASN1_PRINTABLE_STRING:
           case LTC_ASN1_UTF8_STRING:
           case LTC_ASN1_UTCTIME:
           case LTC_ASN1_SEQUENCE:
           case LTC_ASN1_SET:
           case LTC_ASN1_SETOF:
           case LTC_ASN1_RAW_BIT_STRING:
           case LTC_ASN1_GENERALIZEDTIME:
                LTC_SET_ASN1(list, x++, type, data, size);
                break;

           case LTC_ASN1_CHOICE:
           case LTC_ASN1_CUSTOM_TYPE:
           case LTC_ASN1_EOL:
           case LTC_ASN1_TELETEX_STRING:
               va_end(args);
               err = CRYPT_INVALID_ARG;
               goto LBL_ERR;
       }
   }
   va_end(args);

   err = der_encode_sequence(list, x, out, outlen);
LBL_ERR:
   XFREE(list);
   return err;
}

#endif

#pragma clang diagnostic pop
