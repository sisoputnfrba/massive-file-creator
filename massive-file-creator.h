/*
 * massive-file-creator.h
 *
 *  Created on: Jul 21, 2012
 *      Author: marke
 */

#ifndef MASSIVE_FILE_CREATOR_H_
#define MASSIVE_FILE_CREATOR_H_

#include <pthread.h>

/*--------------------------------------------------------------
 Constantes
 --------------------------------------------------------------*/
#define ERROR -1
#define SUCCESS 0
#define MD5_OK 1

#define DEFAULT_ARGS_AMOUNT 4
#define MAX_THREADS_AMOUNT 256
#define MAX_FILE_LEN 10 * 1024   //10mb

#define FILENAME_HEAD "file_"
#define FILENAME_TAIL ".test"
#define FILENAME_LEN 1024
#define BUFF_LEN 1024

/*--------------------------------------------------------------
 Definiciones de tipos
 --------------------------------------------------------------*/

// Digesto md5
typedef unsigned char* MD5_DIGEST;

// Datos de config para threads generadores de archivos.
typedef struct {
	int file_len;
	char* location;
	int thread_number;
	char* file_prefix;
} thread_params;

// Datos de configuracion general.
typedef struct {
	int threads_amount;
	int file_len;
	char* location;
	char* files_prefix;
} global_config;

// Estructura de retorno para threads
typedef struct {
	int retcode;
	MD5_DIGEST md5sum_writen; // md5sum de datos escritos
	MD5_DIGEST md5sum_read;   // md5sum de datos leidos
} thread_return;

int get_params(char** params, int params_amount, global_config* config);
void destroy_params(global_config config);
thread_params* create_thread_params(global_config config, int thread_number);
pthread_t* threads_create(global_config config);
void* file_generate(void* args);
void destroy_thread_params(thread_params* params);
int threads_get_and_print_results(pthread_t* threads, int threads_amount,
		char* location, char* prefix);
void threads_destroy(pthread_t* threads, int threads_amount);
void build_file_name(char* filename, int file_id, char* path, char* prefix);
void self_terminate_as(int retcode, MD5_DIGEST digest_writen,
		MD5_DIGEST digest_read);
void fill_buffer(char* buffer, int len);
int file_get_digest(char* filename, MD5_DIGEST digest);
void print_result(thread_return ret_data);
void print_result_line(thread_return* ret_data, int line_number,
		char* location, char* prefix);
void print_md5sum(MD5_DIGEST md5sum);
void print_result_header();
void print_result_trailer();
void release_thread_retdata(thread_return* ret_data);
char* sanitize_location(char* location);
int sanitize_threads_amount(long int actual_threads_amount);
int sanitize_file_len(long int actual_file_len);
void print_a_global_header(global_config config);

#endif /* MASSIVE_FILE_CREATOR_H_ */
