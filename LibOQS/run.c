#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <oqs/oqs.h>

#include <Windows.h>
#include <psapi.h>

#include "run.h"

static methrics *M = NULL;
static int *security = NULL;

OQS_KEM *kem = NULL;
uint8_t *public_key = NULL;
uint8_t *secret_key = NULL;
uint8_t *ciphertext = NULL;
uint8_t *shared_secret_e = NULL;
uint8_t *shared_secret_d = NULL;

static int pk_size = 0;
static int sk_size = 0;
static int ct_size = 0;
static int ss_e_size = 0;
static int ss_d_size = 0;

// /* Cleaning up memory etc */
static void set_param(void)
{
	if (kem != NULL)
	{
		pk_size = (int)kem->length_public_key;
		sk_size = (int)kem->length_secret_key;
		ct_size = (int)kem->length_ciphertext;
		ss_e_size = (int)kem->length_shared_secret;
		ss_d_size = (int)kem->length_shared_secret;
	}
}

static void clean_ptr(uint8_t *ptr, size_t len)
{
	for (size_t i = 0; i < len; i++)
	{
		ptr[i] = 0;
	}
}

void create_space(int alg)
{
	if (kem == NULL)
	{
		if (alg == 1) kem = OQS_KEM_new(OQS_KEM_alg_classic_mceliece_348864);
		if (alg == 2) kem = OQS_KEM_new(OQS_KEM_alg_classic_mceliece_460896);
		if (alg == 3) kem = OQS_KEM_new(OQS_KEM_alg_classic_mceliece_6688128);
		if (kem == NULL)
		{
			printf("OQS_KEM_classic_mceliece was not enabled at "
				   "compile-time.\n");
			return;
		}
		else
		{
			set_param();
			public_key = malloc(kem->length_public_key);
			secret_key = malloc(kem->length_secret_key);
			ciphertext = malloc(kem->length_ciphertext);
			shared_secret_e = malloc(kem->length_shared_secret);
			shared_secret_d = malloc(kem->length_shared_secret);

			if ((public_key == NULL) || (secret_key == NULL) || (ciphertext == NULL) ||
				(shared_secret_e == NULL) || (shared_secret_d == NULL))
			{
				fprintf(stderr, "ERROR: malloc failed!\n");
				cleanup_heap();

				return;
			}
			printf("OQS_KEM space created completed.\n");
		}
	}
}

int get_using_memory(void)
{
    HANDLE hProcess = GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc));
    return pmc.WorkingSetSize;
}

void create_methrics(void)
{
	if (M != NULL) free(M);
	M = NULL;
	M = malloc(sizeof(methrics));
}

void methrics_keypair(void)
{
	M->using_memory = pk_size + sk_size;
	M->one_obj_size = pk_size;
	M->two_obj_size = sk_size;
}

void methrics_encaps(void)
{
	M->using_memory = pk_size + ss_e_size + ct_size;
	M->one_obj_size = ct_size;
	M->two_obj_size = abs(ct_size - ss_e_size);
}

void methrics_decaps(void)
{
	M->using_memory = sk_size + ss_d_size + ct_size;
	M->one_obj_size = ss_d_size;
	M->two_obj_size = abs(ct_size - ss_d_size);
}

void cpy_methrics(methrics *MOUT)
{
	if (M != NULL)
	{
		MOUT->time = M->time;
		MOUT->cycle = M->cycle;
		MOUT->using_memory = M->using_memory;
		MOUT->one_obj_size = M->one_obj_size;
		MOUT->two_obj_size = M->two_obj_size;
	}
}

void clear_methrics(void)
{
	if (M != NULL)
	{
		M->time = 0;
		M->cycle = 0;
		M->using_memory = 0;
		M->one_obj_size = 0;
		M->two_obj_size = 0;
	}
}

//---------------------------------------------------------------------------------
static OQS_STATUS mce_348864_keypair() { //вывод ключей в файл
	clear_methrics();
	clock_t start = clock();

	OQS_STATUS rc = OQS_KEM_keypair(kem, public_key, secret_key);
	
	clock_t end = clock();
	M->time = (double)(end - start) / (double)CLOCKS_PER_SEC;
	M->cycle = (double)(end - start);

	if (rc != OQS_SUCCESS) {
		fprintf(stderr, "ERROR: OQS_KEM_keypair failed!\n");
		cleanup_heap();

		return OQS_ERROR;
	}
	printf("OQS_KEM_classic_mceliece_348864 keypair completed.\n");
	return OQS_SUCCESS; // success
}

static OQS_STATUS mce_348864_encaps() { //поступает публичный ключ - выход шифротекст и строка
	clear_methrics();
	clock_t start = clock();
	
	OQS_STATUS rc = OQS_KEM_encaps(kem, ciphertext, shared_secret_e, public_key);
	
	clock_t end = clock();
	M->time = (double)(end - start) / (double)CLOCKS_PER_SEC;
	M->cycle = (double)(end - start);

	for (int i = 0; i < kem->length_shared_secret; i++) {
		printf("%02x ", shared_secret_e[i]);
	}
	printf("\n");
	if (rc != OQS_SUCCESS) {
		fprintf(stderr, "ERROR: OQS_KEM_encaps failed!\n");
		cleanup_heap();

		return OQS_ERROR;
	}
	printf("OQS_KEM_classic_mceliece_348864 encaps completed.\n");
	return OQS_SUCCESS; // success
}
static OQS_STATUS mce_348864_decaps() { //поступает шифротекст и секретный ключ - выход строка
	clear_methrics();
	clock_t start = clock();
	
	OQS_STATUS rc = OQS_KEM_decaps(kem, shared_secret_d, ciphertext, secret_key);
	
	clock_t end = clock();
	M->time = (double)(end - start) / (double)CLOCKS_PER_SEC;
	M->cycle = (double)(end - start);
	
	for (int i = 0; i < kem->length_shared_secret; i++) {
		printf("%02x ", shared_secret_d[i]);
	}
	printf("\n");
	if (rc != OQS_SUCCESS) {
		fprintf(stderr, "ERROR: OQS_KEM_decaps failed!\n");
		cleanup_heap();

		return OQS_ERROR;
	}

	printf("OQS_KEM_classic_mceliece_348864 decaps completed.\n");
	return OQS_SUCCESS; // success
}

int run_mce_348864(uint8_t call) {
	switch (call)
	{
	case 1:
		if (mce_348864_keypair() == OQS_SUCCESS) { return EXIT_SUCCESS; }
		else { return EXIT_FAILURE; }
		break;
	case 2:
		if (mce_348864_encaps() == OQS_SUCCESS) { return EXIT_SUCCESS; }
		else { return EXIT_FAILURE; }
		break;
	case 3:
		if (mce_348864_decaps() == OQS_SUCCESS) { return EXIT_SUCCESS; }
		else { return EXIT_FAILURE; }
		break;
	}
}

//---------------------------------------------------------------------------------
static OQS_STATUS mce_460896_keypair() { //вывод ключей в файл
	clear_methrics();
	clock_t start = clock();
	
	OQS_STATUS rc = OQS_KEM_keypair(kem, public_key, secret_key);
	
	clock_t end = clock();
	M->time = (double)(end - start) / (double)CLOCKS_PER_SEC;
	M->cycle = (double)(end - start);
	
	if (rc != OQS_SUCCESS) {
		fprintf(stderr, "ERROR: OQS_KEM_keypair failed!\n");
		cleanup_heap();

		return OQS_ERROR;
	}
	printf("OQS_KEM_classic_mceliece_460896 keypair completed.\n");
	return OQS_SUCCESS; // success
}

static OQS_STATUS mce_460896_encaps() { //поступает публичный ключ - выход шифротекст и строка
	clear_methrics();
	clock_t start = clock();
	
	OQS_STATUS rc = OQS_KEM_encaps(kem, ciphertext, shared_secret_e, public_key);
	
	clock_t end = clock();
	M->time = (double)(end - start) / (double)CLOCKS_PER_SEC;
	M->cycle = (double)(end - start);
	
	for (int i = 0; i < kem->length_shared_secret; i++) {
		printf("%02x ", shared_secret_e[i]);
	}
	printf("\n");
	if (rc != OQS_SUCCESS) {
		fprintf(stderr, "ERROR: OQS_KEM_encaps failed!\n");
		cleanup_heap();

		return OQS_ERROR;
	}
	printf("OQS_KEM_classic_mceliece_460896 encaps completed.\n");
	return OQS_SUCCESS; // success
}
static OQS_STATUS mce_460896_decaps() { //поступает шифротекст и секретный ключ - выход строка
	clear_methrics();
	clock_t start = clock();

	OQS_STATUS rc = OQS_KEM_decaps(kem, shared_secret_d, ciphertext, secret_key);
	
	clock_t end = clock();
	M->time = (double)(end - start) / (double)CLOCKS_PER_SEC;
	M->cycle = (double)(end - start);
	
	for (int i = 0; i < kem->length_shared_secret; i++) {
		printf("%02x ", shared_secret_d[i]);
	}
	printf("\n");
	if (rc != OQS_SUCCESS) {
		fprintf(stderr, "ERROR: OQS_KEM_decaps failed!\n");
		cleanup_heap();

		return OQS_ERROR;
	}

	printf("OQS_KEM_classic_mceliece_460896 decaps completed.\n");

	return OQS_SUCCESS; // success
}

int run_mce_460896(uint8_t call) {
	switch (call)
	{
	case 1:
		if (mce_460896_keypair() == OQS_SUCCESS) { return EXIT_SUCCESS; }
		else { return EXIT_FAILURE; }
		break;
	case 2:
		if (mce_460896_encaps() == OQS_SUCCESS) { return EXIT_SUCCESS; }
		else { return EXIT_FAILURE; }
		break;
	case 3:
		if (mce_460896_decaps() == OQS_SUCCESS) { return EXIT_SUCCESS; }
		else { return EXIT_FAILURE; }
		break;
	}
}

//---------------------------------------------------------------------------------
static OQS_STATUS mce_6688128_keypair() { //вывод ключей в файл
	clear_methrics();
	clock_t start = clock();
	
	OQS_STATUS rc = OQS_KEM_keypair(kem, public_key, secret_key);
	
	clock_t end = clock();
	M->time = (double)(end - start) / (double)CLOCKS_PER_SEC;
	M->cycle = (double)(end - start);
	
	if (rc != OQS_SUCCESS) {
		fprintf(stderr, "ERROR: OQS_KEM_keypair failed!\n");
		cleanup_heap();

		return OQS_ERROR;
	}
	printf("OQS_KEM_classic_mceliece_6688128 keypair completed.\n");
	return OQS_SUCCESS; // success
}

static OQS_STATUS mce_6688128_encaps() { //поступает публичный ключ - выход шифротекст и строка
	clear_methrics();
	clock_t start = clock();
	
	OQS_STATUS rc = OQS_KEM_encaps(kem, ciphertext, shared_secret_e, public_key);
	
	clock_t end = clock();
	M->time = (double)(end - start) / (double)CLOCKS_PER_SEC;
	M->cycle = (double)(end - start);
	
	for (int i = 0; i < kem->length_shared_secret; i++) {
		printf("%02x ", shared_secret_e[i]);
	}
	printf("\n");
	if (rc != OQS_SUCCESS) {
		fprintf(stderr, "ERROR: OQS_KEM_encaps failed!\n");
		cleanup_heap();

		return OQS_ERROR;
	}
	printf("OQS_KEM_classic_mceliece_6688128 encaps completed.\n");
	return OQS_SUCCESS; // success
}
static OQS_STATUS mce_6688128_decaps() { //поступает шифротекст и секретный ключ - выход строка
	clear_methrics();
	clock_t start = clock();
	
	OQS_STATUS rc = OQS_KEM_decaps(kem, shared_secret_d, ciphertext, secret_key);
	
	clock_t end = clock();
	M->time = (double)(end - start) / (double)CLOCKS_PER_SEC;
	M->cycle = (double)(end - start);
	
	for (int i = 0; i < kem->length_shared_secret; i++) {
		printf("%02x ", shared_secret_d[i]);
	}
	printf("\n");
	if (rc != OQS_SUCCESS) {
		fprintf(stderr, "ERROR: OQS_KEM_decaps failed!\n");
		cleanup_heap();

		return OQS_ERROR;
	}

	printf("OQS_KEM_classic_mceliece_6688128 decaps completed.\n");
	return OQS_SUCCESS; // success
}

int run_mce_6688128(uint8_t call) {
	switch (call)
	{
	case 1:
		if (mce_6688128_keypair() == OQS_SUCCESS) { return EXIT_SUCCESS; }
		else { return EXIT_FAILURE; }
		break;
	case 2:
		if (mce_6688128_encaps() == OQS_SUCCESS) { return EXIT_SUCCESS; }
		else { return EXIT_FAILURE; }
		break;
	case 3:
		if (mce_6688128_decaps() == OQS_SUCCESS) { return EXIT_SUCCESS; }
		else { return EXIT_FAILURE; }
		break;
	}
}



// ///////////////////
// //SERVICE FUNCTIONS
// ///////////////////
static size_t convert_hex_inv(char *dest, size_t count, const uint8_t *src)
{
	size_t i = 0;
	for (i = 0; i < count && sprintf(dest + i * 2, "%02x", src[i]) == 2; i++)
		;
	return i;
}

void get_text(char *text, uint8_t call)
{
	char text_hex[1024] = "";
	switch (call)
	{
	case 1:
		convert_hex_inv(text_hex, kem->length_shared_secret, shared_secret_e);
		strcat(text, text_hex);
		break;
	case 2:
		convert_hex_inv(text_hex, kem->length_shared_secret, shared_secret_d);
		strcat(text, text_hex);
		break;
	}
}

void get_security_level(const char *filename,int *security_level)
{
	FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Error opening file for reading\n");
        return;
    }
	fread(security_level, sizeof(int), 1, file);
	fseek(file, 0, SEEK_SET);
	fclose(file);
}

void save(char *filename, uint8_t call, int security_level)
{
	FILE *file = fopen(filename, "wb");
	if (file == NULL) {
		printf("Failed to open file for writing\n");
		return;
	}
	switch(call)
	{
	case 1:
		fwrite(&security_level, sizeof(int), 1, file);
		fwrite(public_key, sizeof(uint8_t), kem->length_public_key, file);
		clean_ptr(public_key, kem->length_public_key);
		free(public_key);
		break;
	case 2:
		fwrite(&security_level, sizeof(int), 1, file);
		fwrite(secret_key, sizeof(uint8_t), kem->length_secret_key, file);
		clean_ptr(secret_key, kem->length_secret_key);
		free(secret_key);
		break;
	case 3:
		fwrite(&security_level, sizeof(int), 1, file);
		fwrite(ciphertext, sizeof(uint8_t), kem->length_ciphertext, file);
		clean_ptr(ciphertext, kem->length_ciphertext);
		free(ciphertext);
		break;
	}
	fclose(file);
}

void load(char *filename, uint8_t call, int *security_level)
{
	FILE *file = fopen(filename, "rb");
	if (file == NULL) {
		printf("Failed to open file for read\n");
		return;
	}
	switch(call)
	{
	case 1:
		fread(&security_level, sizeof(int), 1, file);
		fread(public_key, sizeof(uint8_t), kem->length_public_key, file);
		break;
	case 2:
		fread(&security_level, sizeof(int), 1, file);
		fread(secret_key, sizeof(uint8_t), kem->length_secret_key, file);
		break;
	case 3:
		fread(&security_level, sizeof(int), 1, file);
		fread(ciphertext, sizeof(uint8_t), kem->length_ciphertext, file);
		break;	
	}
	fclose(file);
}


void cleanup_heap(void) {
	if (kem != NULL) {
		OQS_MEM_secure_free(secret_key, kem->length_secret_key);
		OQS_MEM_secure_free(shared_secret_e, kem->length_shared_secret);
		OQS_MEM_secure_free(shared_secret_d, kem->length_shared_secret);
	}
	OQS_MEM_insecure_free(public_key);
	OQS_MEM_insecure_free(ciphertext);
	OQS_KEM_free(kem);
}
