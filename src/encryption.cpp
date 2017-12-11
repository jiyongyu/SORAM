
#include "encryption.hpp"

#include "cryptopp/secblock.h"
using CryptoPP::FixedSizeAlignedSecBlock;

#include <emmintrin.h>

// -----------------------------------------------------------------------------
// Intrinsics
// -----------------------------------------------------------------------------

typedef unsigned int word32;

__inline int __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_extract_epi32 (__m128i a, const int i)
{
   int r;
   asm ("pextrd %2, %1, %0" : "=rm"(r) : "x"(a), "i"(i));
   return r;
}
__inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_insert_epi32 (__m128i a, int b, const int i)
{
   asm ("pinsrd %2, %1, %0" : "+x"(a) : "rm"(b), "i"(i));
   return a;
}

__inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_clmulepi64_si128 (__m128i a, __m128i b, const int i)
{
   asm ("pclmulqdq %2, %1, %0" : "+x"(a) : "xm"(b), "i"(i));
   return a;
}
__inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_aeskeygenassist_si128 (__m128i a, const int i)
{
   __m128i r;
   asm ("aeskeygenassist %2, %1, %0" : "=x"(r) : "xm"(a), "i"(i));
   return r;
}
__inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_aesimc_si128 (__m128i a)
{
   __m128i r;
   asm ("aesimc %1, %0" : "=x"(r) : "xm"(a));
   return r;
}
__inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_aesenc_si128 (__m128i a, __m128i b)
{
   asm ("aesenc %1, %0" : "+x"(a) : "xm"(b));
   return a;
}
__inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_aesenclast_si128 (__m128i a, __m128i b)
{
   asm ("aesenclast %1, %0" : "+x"(a) : "xm"(b));
   return a;
}
__inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_aesdec_si128 (__m128i a, __m128i b)
{
   asm ("aesdec %1, %0" : "+x"(a) : "xm"(b));
   return a;
}
__inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_aesdeclast_si128 (__m128i a, __m128i b)
{
   asm ("aesdeclast %1, %0" : "+x"(a) : "xm"(b));
   return a;
}

// -----------------------------------------------------------------------------
// KeyGen
// -----------------------------------------------------------------------------

void KeyGen(FixedSizeAlignedSecBlock<word32, 4*15> * m_key, 
            const byte *userKey, 
            unsigned int keylen, 
            bool_t is_encryption)
{
   int m_rounds = keylen/4 + 6;

   m_key->New(4*(m_rounds+1));

   word32 *rk = *m_key;

   static const word32 rcLE[] = {
      0x01, 0x02, 0x04, 0x08,
      0x10, 0x20, 0x40, 0x80,
      0x1B, 0x36, /* for 128-bit blocks, Rijndael never uses more than 10 rcon values */
   };
   const word32 *rc = rcLE;

   __m128i temp = _mm_loadu_si128((__m128i *)(userKey+keylen-16));
   memcpy(rk, userKey, keylen);

   while (true)
   {
      rk[keylen/4] = rk[0] ^ _mm_extract_epi32(_mm_aeskeygenassist_si128(temp, 0), 3) ^ *(rc++);
      rk[keylen/4+1] = rk[1] ^ rk[keylen/4];
      rk[keylen/4+2] = rk[2] ^ rk[keylen/4+1];
      rk[keylen/4+3] = rk[3] ^ rk[keylen/4+2];

      if (rk + keylen/4 + 4 == m_key->end())
         break;

      if (keylen == 24)
      {
         rk[10] = rk[ 4] ^ rk[ 9];
         rk[11] = rk[ 5] ^ rk[10];
         temp = _mm_insert_epi32(temp, rk[11], 3);
      }
      else if (keylen == 32)
      {
         temp = _mm_insert_epi32(temp, rk[11], 3);
         rk[12] = rk[ 4] ^ _mm_extract_epi32(_mm_aeskeygenassist_si128(temp, 0), 2);
         rk[13] = rk[ 5] ^ rk[12];
         rk[14] = rk[ 6] ^ rk[13];
         rk[15] = rk[ 7] ^ rk[14];
         temp = _mm_insert_epi32(temp, rk[15], 3);
      }
      else
         temp = _mm_insert_epi32(temp, rk[7], 3);

      rk += keylen/4;
   }

   if (!is_encryption)
   {
      rk = *m_key;
      unsigned int i, j;

      std::swap(*(__m128i *)(rk), *(__m128i *)(rk+4*m_rounds));

      for (i = 4, j = 4*m_rounds-4; i < j; i += 4, j -= 4)
      {
         temp = _mm_aesimc_si128(*(__m128i *)(rk+i));
         *(__m128i *)(rk+i) = _mm_aesimc_si128(*(__m128i *)(rk+j));
         *(__m128i *)(rk+j) = temp;
      }

      *(__m128i *)(rk+i) = _mm_aesimc_si128(*(__m128i *)(rk+i));
   }
   return;
}

// -----------------------------------------------------------------------------
// AES Helpers
// -----------------------------------------------------------------------------

#define AES_ROUNDS 10

inline void AESNI_Enc_4_Blocks(__m128i &block0, __m128i &block1, __m128i &block2, __m128i &block3, const __m128i *subkeys)
{
   __m128i rk = subkeys[0];
   block0 = _mm_xor_si128(block0, rk);
   block1 = _mm_xor_si128(block1, rk);
   block2 = _mm_xor_si128(block2, rk);
   block3 = _mm_xor_si128(block3, rk);
   for (unsigned int i=1; i<AES_ROUNDS; i++)
   {
      rk = subkeys[i];
      block0 = _mm_aesenc_si128(block0, rk);
      block1 = _mm_aesenc_si128(block1, rk);
      block2 = _mm_aesenc_si128(block2, rk);
      block3 = _mm_aesenc_si128(block3, rk);
   }
   rk = subkeys[AES_ROUNDS];
   block0 = _mm_aesenclast_si128(block0, rk);
   block1 = _mm_aesenclast_si128(block1, rk);
   block2 = _mm_aesenclast_si128(block2, rk);
   block3 = _mm_aesenclast_si128(block3, rk);
}

inline void AESNI_Dec_4_Blocks(__m128i &block0, __m128i &block1, __m128i &block2, __m128i &block3, const __m128i *subkeys)
{
   __m128i rk = subkeys[0];
   block0 = _mm_xor_si128(block0, rk);
   block1 = _mm_xor_si128(block1, rk);
   block2 = _mm_xor_si128(block2, rk);
   block3 = _mm_xor_si128(block3, rk);
   for (unsigned int i=1; i<AES_ROUNDS; i++)
   {
      rk = subkeys[i];
      block0 = _mm_aesdec_si128(block0, rk);
      block1 = _mm_aesdec_si128(block1, rk);
      block2 = _mm_aesdec_si128(block2, rk);
      block3 = _mm_aesdec_si128(block3, rk);
   }
   rk = subkeys[AES_ROUNDS];
   block0 = _mm_aesdeclast_si128(block0, rk);
   block1 = _mm_aesdeclast_si128(block1, rk);
   block2 = _mm_aesdeclast_si128(block2, rk);
   block3 = _mm_aesdeclast_si128(block3, rk);
}

template <typename F4>
inline void AESNI_AdvancedProcessBlocks(F4 func4, const __m128i *subkeys, const byte *inBlocks, byte *outBlocks)
{
   __m128i block0 = _mm_loadu_si128((const __m128i *)inBlocks), block1, block2, block3;
   block1 = _mm_loadu_si128((const __m128i *)(inBlocks + 16));
   block2 = _mm_loadu_si128((const __m128i *)(inBlocks + 32));
   block3 = _mm_loadu_si128((const __m128i *)(inBlocks + 48));

   func4(block0, block1, block2, block3, subkeys);

   _mm_storeu_si128((__m128i *)outBlocks, block0);
   _mm_storeu_si128((__m128i *)(outBlocks + 16), block1);
   _mm_storeu_si128((__m128i *)(outBlocks + 32), block2);
   _mm_storeu_si128((__m128i *)(outBlocks + 48), block3);
}

// -----------------------------------------------------------------------------
// Interface
// -----------------------------------------------------------------------------

// TODO point these to some sw sets
// TODO initialize using prng
__m128i enc_subkey[AES_ROUNDS+1];
__m128i dec_subkey[AES_ROUNDS+1];

void crypto_initialize() 
{
   byte key[AES::DEFAULT_KEYLENGTH];

   memset(key, 0, AES::DEFAULT_KEYLENGTH);

   FixedSizeAlignedSecBlock<word32, 4*15> enc_key;
   FixedSizeAlignedSecBlock<word32, 4*15> dec_key;

// ... CacheGrind does not currently support AES instrinsics
// (to check: run CacheGrind with --db-attach=yes and -O0 and attach the 
// debugger when it crashes
#if !CACHE_GRIND
   KeyGen(&enc_key, key, sizeof(key), TRUE);
   KeyGen(&dec_key, key, sizeof(key), FALSE);
#endif

   memcpy(enc_subkey, enc_key.begin(), (AES_ROUNDS+1) * AES::DEFAULT_KEYLENGTH);
   memcpy(dec_subkey, dec_key.begin(), (AES_ROUNDS+1) * AES::DEFAULT_KEYLENGTH);
}

void encrypt(cache_line_t * line) 
{
#if !CACHE_GRIND
   AESNI_AdvancedProcessBlocks(  AESNI_Enc_4_Blocks,
                                 enc_subkey,
                                 (byte *) line->data, 
                                 (byte *) line->data);
#endif
}

void decrypt(cache_line_t * line) 
{
#if !CACHE_GRIND
   AESNI_AdvancedProcessBlocks(  AESNI_Dec_4_Blocks,
                                 dec_subkey,
                                 (byte *) line->data, 
                                 (byte *) line->data);
#endif
}

// -----------------------------------------------------------------------------
// Debugging interface
// -----------------------------------------------------------------------------

/*
#include "stdio.h"
int main() 
{
   byte testvector[] = "1111222233334444555566667777888899990000aaaabbbbccccddddeeeeffff";
   ASSERT(sizeof(testvector) == LINE_SIZE + 1); // 64 bytes of characters + \0

   printf("Before: ");
   for (int i = 0; i < LINE_SIZE; i++) printf("%c", testvector[i]); printf("\n");

   crypto_initialize();

   cache_line_t tmp_line;
   memcpy(tmp_line.data, testvector, LINE_SIZE);
   encrypt(&tmp_line);

   printf("Ciphertext: ");
   for (int i = 0; i < LINE_SIZE; i++) printf("%c", tmp_line.data[i]); printf("\n");

   decrypt(&tmp_line);

   printf("After:  ");
   for (int i = 0; i < LINE_SIZE; i++) printf("%c", tmp_line.data[i]); printf("\n");

   return 0;
}
*/
