#include <iostream>
#include <iomanip>
#include "stdlib.h"
#include "stdio.h"

#include <iostream>
#include <cstring>

#include "cryptopp/modes.h"
#include "cryptopp/aes.h"
#include "cryptopp/filters.h"

int main(int argc, char* argv[]) {

   //
   // Key and IV setup
   //AES encryption uses a secret key of a variable length (128-bit, 196-bit or 256-   
   //bit). This key is secretly exchanged between two parties before communication   
   //begins. DEFAULT_KEYLENGTH= 16 bytes
   byte key[ CryptoPP::AES::DEFAULT_KEYLENGTH ], iv[ CryptoPP::AES::BLOCKSIZE ];
   memset( key, 0x00, CryptoPP::AES::DEFAULT_KEYLENGTH );
   memset( iv, 0x00, CryptoPP::AES::BLOCKSIZE );

   //
   // String and Sink setup
   //
   unsigned char * plainbytes = (unsigned char *) malloc( 64 );
   unsigned char * ciphertextbytes = (unsigned char *) malloc( 64 );
   unsigned char * decryptedbytes = (unsigned char *) malloc( 64 );
   plainbytes[0] = 'h';
   plainbytes[1] = 'e';
   plainbytes[2] = 'l';
   plainbytes[3] = 'l';
   plainbytes[4] = 'o';
   plainbytes[5] = ' ';
   plainbytes[6] = 't';
   plainbytes[7] = 'h';
   plainbytes[8] = 'e';
   plainbytes[9] = 'r';
   plainbytes[10] = 'e';
   plainbytes[11] = ' ';
   plainbytes[12] = 'b';
   plainbytes[13] = 'o';
   plainbytes[14] = 's';
   plainbytes[15] = 's';
   plainbytes[62] = '!';
   plainbytes[63] = '\0';

   printf("Plaintext:\n");
   for (int i = 0; i < 64; i++)
      printf("%c", plainbytes[i]); //std::cout << ciphertext << std::endl;
   printf("\n");

   //std::string plaintext = "Now is the time for all good men to come to the aide...";
   std::string ciphertext;

   //
   // Encryption
   //

   CryptoPP::AES::Encryption aesEncryption(key, CryptoPP::AES::DEFAULT_KEYLENGTH);
   CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption( aesEncryption, iv );

   CryptoPP::StreamTransformationFilter stfEncryptor(cbcEncryption, new CryptoPP::StringSink( ciphertext ) );
   stfEncryptor.Put( plainbytes, 64 );
   stfEncryptor.MessageEnd();

   //
   // Translation to and from string
   //

   std::cout << ciphertext << std::endl;

   char * cstr = new char [64];
   memcpy (cstr, ciphertext.c_str(), 64);

   std::string finstring(reinterpret_cast<const char *>(cstr));

   //
   // Decryption
   //

   CryptoPP::AES::Decryption aesDecryption(key, CryptoPP::AES::DEFAULT_KEYLENGTH);
   CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption( aesDecryption, iv );

   CryptoPP::StreamTransformationFilter stfDecryptor(cbcDecryption, new CryptoPP::ArraySink( decryptedbytes, 64 ) );
   stfDecryptor.Put( reinterpret_cast<const unsigned char*>( ciphertext.c_str() ), ciphertext.size() );
   stfDecryptor.MessageEnd();

   printf("Decrypted bytes:\n");
   for (int i = 0; i < 64; i++)
      printf("%c", decryptedbytes[i]); //std::cout << ciphertext << std::endl;
   printf("\n\n");
   
   return 0;
}
