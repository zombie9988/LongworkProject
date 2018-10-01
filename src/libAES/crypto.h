#include "aes.h"
#include "aes.c"

void encryptCbc(char* in, const uint8_t* key, const uint8_t* iv, size_t bufSize, uint8_t* buffer)
{
	for (size_t i = 0; i < bufSize; i++)
	{
		buffer[i] = in[i];
	}

	struct AES_ctx ctx;
	AES_init_ctx_iv(&ctx, key, iv);
	AES_CBC_encrypt_buffer(&ctx, buffer, bufSize);

	for (size_t i = 0; i < bufSize; i++)
	{
		in[i] = buffer[i];
	}
}


void decryptCbc(char* in, const uint8_t* key, const uint8_t* iv, size_t bufSize, uint8_t* buffer)
{
	for (size_t i = 0; i < bufSize; i++)
	{
		buffer[i] = in[i];
	}

	struct AES_ctx ctx;
	AES_init_ctx_iv(&ctx, key, iv);
	AES_CBC_decrypt_buffer(&ctx, buffer, bufSize);

	for (size_t i = 0; i < bufSize; i++)
	{
		in[i] = buffer[i];
	}
}
