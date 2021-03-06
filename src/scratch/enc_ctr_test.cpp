// g++ -g3 -ggdb -O0 -DDEBUG -I/usr/include/cryptopp Driver.cpp -o Driver.exe -lcryptopp -lpthread
// g++ -g -O2 -DNDEBUG -I/usr/include/cryptopp Driver.cpp -o Driver.exe -lcryptopp -lpthread

#include "cryptopp/osrng.h"
using CryptoPP::AutoSeededRandomPool;

#include "stdio.h"

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <string>
using std::string;

#include <cstdlib>
using std::exit;

#include "cryptopp/cryptlib.h"
using CryptoPP::Exception;

#include "cryptopp/hex.h"
using CryptoPP::HexEncoder;
using CryptoPP::HexDecoder;

#include "cryptopp/filters.h"
using CryptoPP::StringSink;
using CryptoPP::StringSource;
using CryptoPP::StreamTransformationFilter;

#include "cryptopp/aes.h"
using CryptoPP::AES;

#include "cryptopp/ccm.h"
using CryptoPP::CTR_Mode;

#include "cryptopp/secblock.h"
using CryptoPP::SecByteBlock;

#include "assert.h"

SecByteBlock HexDecodeString(const char *hex)
{
	StringSource ss(hex, true, new HexDecoder);
	SecByteBlock result((size_t)ss.MaxRetrievable());
	ss.Get(result, result.size());
	return result;
}

int main(int argc, char* argv[])
{
	AutoSeededRandomPool prng;

/*
	byte key[AES::DEFAULT_KEYLENGTH];
	prng.GenerateBlock(key, sizeof(key));
 memset(key, 0, AES::DEFAULT_KEYLENGTH);
*/

//	byte iv[AES::BLOCKSIZE];
//	prng.GenerateBlock(iv, sizeof(iv));
// memset(iv, 0, AES::BLOCKSIZE);

//Key          : AE 68 52 F8 12 10 67 CC 4B F7 A5 76 55 77 F3 9E
//Plaintext        : 53 69 6E 67 6C 65 20 62 6C 6F 63 6B 20 6D 73 67
//IV: 00 00 00 30 00 00 00 00 00 00 00 00 00 00 00 01
//Ciphertext       : E4 09 5D 4F B7 A7 B3 79 2D 61 75 A3 26 13 11 B8

   SecByteBlock key = HexDecodeString("AE 68 52 F8 12 10 67 CC 4B F7 A5 76 55 77 F3 9E");
	SecByteBlock iv = HexDecodeString("00 00 00 30 00 00 00 00 00 00 00 00 00 00 00 01");

   cout << AES::DEFAULT_KEYLENGTH << " " << AES::BLOCKSIZE << endl;

	SecByteBlock plain = HexDecodeString("53 69 6E 67 6C 65 20 62 6C 6F 63 6B 20 6D 73 67");
//string plain = "CTR Mode Test";
	string cipher, encoded, recovered;

   int BLOCK_LEN = 16;

	/*********************************\
	\*********************************/

	// Pretty print key
	encoded.clear();
	StringSource(key, BLOCK_LEN, true,
		new HexEncoder(
			new StringSink(encoded)
		) // HexEncoder
	); // StringSource
	cout << "key: " << encoded << endl;

	// Pretty print iv
	encoded.clear();
	StringSource(iv, BLOCK_LEN, true,
		new HexEncoder(
			new StringSink(encoded)
		) // HexEncoder
	); // StringSource
	cout << "iv: " << encoded << endl;

	/*********************************\
	\*********************************/

   // Pretty print plain text
	encoded.clear();
	StringSource(plain, BLOCK_LEN, true,
		new HexEncoder(
			new StringSink(encoded)
		) // HexEncoder
	); // StringSource
	cout << "plaintext: " << encoded << endl;

	try
	{
		CTR_Mode< AES >::Encryption e;
		e.SetKeyWithIV(key, sizeof(key), iv);

		// The StreamTransformationFilter adds padding
		//  as required. ECB and CBC Mode must be padded
		//  to the block size of the cipher.
		StringSource(plain, true, 
			new StreamTransformationFilter(e,
				new StringSink(cipher)
			) // StreamTransformationFilter      
		); // StringSource
	}
	catch(const CryptoPP::Exception& e)
	{
		cerr << e.what() << endl;
		exit(1);
	}

	/*********************************\
	\*********************************/

	// Pretty print
	encoded.clear();
	StringSource(cipher, true,
		new HexEncoder(
			new StringSink(encoded)
		) // HexEncoder
	); // StringSource
	cout << "cipher text: " << encoded << endl;

	/*********************************\
	\*********************************/

	try
	{
		CTR_Mode< AES >::Decryption d;
		d.SetKeyWithIV(key, sizeof(key), iv);

		// The StreamTransformationFilter removes
		//  padding as required.
		StringSource s(cipher, true, 
			new StreamTransformationFilter(d,
				new StringSink(recovered)
			) // StreamTransformationFilter
		); // StringSource

		//cout << "recovered text: " << recovered << endl;

      const char * c = recovered.c_str();
      for (int i = 0; i < BLOCK_LEN; i++) {
         printf("%x ", c[i]);
      }
      printf("\n");
      // Pretty print plain text
/*	   encoded.clear();
	   StringSource(recoveredSecByteBlock, BLOCK_LEN, true,
		   new HexEncoder(
			   new StringSink(encoded)
		   ) // HexEncoder
	   ); // StringSource*/
	   cout << "recovered: " << recovered << endl;

	}
	catch(const CryptoPP::Exception& e)
	{
		cerr << e.what() << endl;
		exit(1);
	}

	/*********************************\
	\*********************************/

	return 0;
}

