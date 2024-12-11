#ifndef RUN_H
#define RUN_H
#include <stdint.h>
#include <config_files.h>

/** 
 * call:
 * 1 - keypair
 * 2 - encaps
 * 3 - decaps
*/


// //mce 348864
int run_mce_348864(uint8_t call);

// //mce 460896
int run_mce_460896(uint8_t call);

// //mce 6688128
int run_mce_6688128(uint8_t call);


// //SERVICE FUNCTIONS
void create_space(int alg);
int get_using_memory(void);
void get_text(char *text, uint8_t call);
void get_security_level(const char *filename,int *security_level);
void save(char *filename, uint8_t call, int security_level);
void load(char *filename, uint8_t call, int *security_level);
void cleanup_heap(void);

void create_methrics(void);
void methrics_keypair(void);
void methrics_encaps(void);
void methrics_decaps(void);
void cpy_methrics(methrics *MOUT);
void clear_methrics(void);

#endif