/*
 * massive-file-creator.c
 *
 *  Created on: Jul 21, 2012
 *      Author: marke
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "massive-file-creator.h"

/*------------------------------------------------------
 Variables globales
 -----------------------------------------------------*/

/*------------------------------------------------------
 Proceso principal
 -----------------------------------------------------*/
//TODO: separar en varos archivos de codigo
int main(int argc, char **argv) {

	global_config config;
	pthread_t* threads;

	if (get_params(argv + 1, argc - 1, &config) != SUCCESS) {
		printf("Error en los argumentos recibidos.\n");
		return EXIT_FAILURE;
	}

	threads = threads_create(config);

	threads_wait_for_completion(threads, config.threads_amount);

	return EXIT_SUCCESS;
}

int get_params(char** params, int params_amount, global_config* config_data) {

	int i;

	if (params_amount != DEFAULT_ARGS_AMOUNT) {
		return ERROR;
	}

	for (i = 0; i < params_amount; i++) {
		switch (i) {
		case 0:
			config_data->threads_amount = strtol(params[i], NULL, 10);
			break;
		case 1:
			config_data->files_per_thread = strtol(params[i], NULL, 10);
			break;
		case 2:
			config_data->location = strdup(params[i]);
			break;
		}
	}

	return SUCCESS;

}

pthread_t* threads_create(global_config config) {

	int i;
	thread_params* params;
	pthread_t oneThread;
	pthread_t* threads = malloc(sizeof(*threads) * config.threads_amount);

	for (i = 1; i <= config.threads_amount; i++) {

		params = get_thread_params(config, i);

		if (pthread_create(&oneThread, NULL, file_generate, (void*) params)
				!= 0) {
			printf("Error inicializando threads.\n");
			free(threads);
			destroy_thread_params(params);
			return NULL ;
		}

		threads[i - 1] = oneThread;
	}

	return threads;

}

thread_params* get_thread_params(global_config config, int thread_number) {

	thread_params* params = malloc(sizeof(thread_params));

	params->files_amount = config.files_per_thread;
	params->location = config.location;
	params->thread_number = thread_number;

	return params;
}

void* file_generate(void* args) {

	thread_params* params = args;
	char filename[FILENAME_LEN] = FILENAME_HEAD;
	char threadno[10];
	FILE* file;

	sprintf(threadno, "%d", params->thread_number);
	strcat(filename, threadno);

	if ((file = fopen(filename, "w")) == NULL ) { //TODO: Agregar ruta de archivo
		printf("Error generando archivo en thread numero %lu", pthread_self());
		pthread_exit((void*) EXIT_FAILURE);
	}

	pthread_exit((void*) EXIT_SUCCESS);
}

void destroy_thread_params(thread_params* params) {

	destroy_thread_params(params);
}

int threads_wait_for_completion(pthread_t* threads, int threads_amount) {

	int i;
	int retcode;

	for (i = 0; i < threads_amount; i++) {

		if (pthread_join(threads[i], (void*) &retcode) != 0) {
			printf("Error en finalizacion de thread de escritura %d.\n", i);
			return ERROR;
		}

		if (retcode != EXIT_SUCCESS)
			return ERROR;

	}

	return SUCCESS;

}
