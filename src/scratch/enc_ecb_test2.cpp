
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

#include "cryptopp/modes.h"
using CryptoPP::ECB_Mode;

using CryptoPP::ArraySource;
using CryptoPP::ArraySink;

int main(int argc, char* argv[])
{
	AutoSeededRandomPool prng;

	byte key[AES::DEFAULT_KEYLENGTH];
	prng.GenerateBlock(key, sizeof(key));

   memset(key, 0, AES::DEFAULT_KEYLENGTH);

   int BLOCK_LEN = 64;
   byte plainbytes[] = "1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcde";
   byte cipherbytes[BLOCK_LEN];
   byte recoveredbytes[BLOCK_LEN];

   for (int i = 0; i < sizeof(plainbytes); i++)
      printf("%d <%c>\n", i, plainbytes[i]);
   printf("\nSize was: %d\n", sizeof(plainbytes));

	string cipher, encoded, recovered;

	/*********************************\
	\*********************************/

/*
	// Pretty print key
	encoded.clear();
	ArraySource(key, sizeof(key), true,
		new HexEncoder(
			new StringSink(encoded)
		) // HexEncoder
	); // StringSource
	cout << "key: " << encoded << endl;
*/

	/*********************************\
	\*********************************/

	try
	{
		//cout << "plain text: " << plain << endl;

		ECB_Mode< AES >::Encryption e;
		e.SetKey(key, sizeof(key));

		// The StreamTransformationFilter adds padding
		//  as required. ECB and CBC Mode must be padded
		//  to the block size of the cipher.
		ArraySource(plainbytes, sizeof(plainbytes) - 1, true,
			new StreamTransformationFilter(e,
				new ArraySink(cipherbytes, BLOCK_LEN)
			) // StreamTransformationFilter      
		); // StringSource
	}
	catch(const CryptoPP::Exception& e)
	{
		cerr << e.what() << endl;
		exit(1);
	}

   cout << "Cipher bytes: " << endl;
   for (int i = 0; i < sizeof(cipherbytes); i++)
      printf("%d <%c>\n", i, cipherbytes[i]);
   printf("\n");

   //cout << "Cipher: " << cipher << endl;

/*
	// Pretty print
	encoded.clear();
	StringSource(cipher, true,
		new HexEncoder(
			new StringSink(encoded)
		) // HexEncoder
	); // StringSource
	cout << "cipher text: " << encoded << " ; length: " << ((float)encoded.size())/2.0 << endl;
*/

	try
	{
		ECB_Mode< AES >::Decryption d;
		d.SetKey(key, sizeof(key));

		// The StreamTransformationFilter removes
		//  padding as required.
		ArraySource s(cipherbytes, sizeof(cipherbytes), true,
			new StreamTransformationFilter(d,
				new StringSink(recovered)//bytes, BLOCK_LEN)
			) // StreamTransformationFilter
		); // StringSource

		cout << "recovered text: " << recovered << endl;
	}
	catch(const CryptoPP::Exception& e)
	{
		cerr << e.what() << endl;
		exit(1);
	}

	return 0;
}

