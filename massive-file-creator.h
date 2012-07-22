/*
 * massive-file-creator.h
 *
 *  Created on: Jul 21, 2012
 *      Author: marke
 */

#ifndef MASSIVE_FILE_CREATOR_H_
#define MASSIVE_FILE_CREATOR_H_

/*--------------------------------------------------------------
	Constantes
--------------------------------------------------------------*/
#define ERROR -1
#define SUCCESS 0

#define DEFAULT_ARGS_AMOUNT 3

#define FILENAME_HEAD "file_"
#define FILENAME_LEN 50



/*--------------------------------------------------------------
	Definiciones de tipos
--------------------------------------------------------------*/

// Datos de config para threads generadores de archivos.
typedef struct {
	int files_amount;
	char* location;
	int thread_number;
} thread_params;

// Datos de configuracion general.
typedef struct {
	int threads_amount;
	int files_per_thread;
	char* location;
} global_config;

int get_params(char** params, int params_amount, global_config* config);
thread_params* get_thread_params(global_config config, int thread_number);
pthread_t* threads_create(global_config config);
void* file_generate(void* args);
void destroy_thread_params(thread_params* params);
int threads_wait_for_completion(pthread_t* threads, int threads_amount);

#endif /* MASSIVE_FILE_CREATOR_H_ */
