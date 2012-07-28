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
#include <time.h>
#include "massive-file-creator.h"
#include <openssl/md5.h>
#include <unistd.h>

/*------------------------------------------------------
 Proceso principal
 -----------------------------------------------------*/
int main(int argc, char **argv) {

	global_config config;
	pthread_t* threads;

	if (get_params(argv + 1, argc - 1, &config) != SUCCESS) {
		printf("Error en los argumentos recibidos. Ejemplo de uso:\n\n");
		printf("./massive-file-creator 4 1024 ./home/ prefix_01_\n");
		return EXIT_FAILURE;
	}

	print_a_global_header(config);

	threads = threads_create(config);

	threads_get_and_print_results(threads, config.threads_amount,
			config.location, config.files_prefix);

	threads_destroy(threads, 0);

	destroy_params(config);

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
			config_data->threads_amount = sanitize_threads_amount(
					strtol(params[i], NULL, 10));
			break;
		case 1:
			config_data->file_len = sanitize_file_len(strtol(params[i], NULL, 10));
			break;
		case 2:
			config_data->location = sanitize_location(params[i]);
			break;
		case 3:
			config_data->files_prefix = strdup(params[i]);
			break;
		}

	}
	return SUCCESS;
}

pthread_t* threads_create(global_config config) {

	int i;
	thread_params* params;
	pthread_t* threads;

	threads = malloc(sizeof(*threads) * config.threads_amount);

	for (i = 1; i <= config.threads_amount; i++) {

		params = create_thread_params(config, i);

		if (pthread_create(&threads[i - 1], NULL, file_generate, (void*) params)
				!= 0) {
			printf("Error inicializando threads.\n");
			threads_destroy(threads, i - 1);
			destroy_thread_params(params);
			return NULL ;
		}

	}

	return threads;

}

thread_params* create_thread_params(global_config config, int thread_number) {

	thread_params* params = malloc(sizeof(thread_params));

	params->file_len = config.file_len;
	params->location = config.location;
	params->thread_number = thread_number;
	params->file_prefix = config.files_prefix;

	return params;
}

void build_file_name(char* filename, int file_id, char* path, char* prefix) {

	char file_id_str[10];

	strcpy(filename, path);
	strcat(filename, prefix);
	strcat(filename, FILENAME_HEAD);
	sprintf(file_id_str, "%d", file_id);
	strcat(filename, file_id_str);
	strcat(filename, FILENAME_TAIL);
}

void* file_generate(void* args) {

	thread_params* params = args;
	char filename[FILENAME_LEN];
	char buffer[BUFF_LEN];
	FILE* file;
	int i;
	unsigned char digest_writen[MD5_DIGEST_LENGTH],
			digest_read[MD5_DIGEST_LENGTH];
	MD5_CTX context;
	int thread_number, file_len;

// Rescatar parametros de thread
	thread_number = params->thread_number;
	file_len = params->file_len;

// Construir nombre de archivo.
	build_file_name(filename, thread_number, params->location,
			params->file_prefix);

	destroy_thread_params(params);

// Inicializar contexto para md5sum.
	MD5_Init(&context);

// Escribir archivo con contenido aleatorio y actualizar contexto md5sum.
	if ((file = fopen(filename, "w")) == NULL ) {
		printf("Error generando archivo en thread numero %lu.\n",
				pthread_self());
		self_terminate_as(ERROR, NULL, NULL );
	}

	for (i = 1; i <= file_len; i++) {

		fill_buffer(buffer, BUFF_LEN);

		if (fwrite(buffer, 1, BUFF_LEN, file) != BUFF_LEN) {
			printf("Error escribiendo archivo en thread numero %lu.\n",
					pthread_self());
			fclose(file);
			self_terminate_as(ERROR, NULL, NULL );
		}

		MD5_Update(&context, buffer, BUFF_LEN);

	}

	if (fclose(file) == EOF) {
		printf("Error cerrando archivo para escritura en thread numero %lu.\n",
				pthread_self());
		self_terminate_as(ERROR, NULL, NULL );
	}

// Obtener digesto de datos ESCRITOS.
	if (MD5_Final(digest_writen, &context) != MD5_OK) {
		printf("Error leyendo md5sum de datos ESCRITOS en thread numero %lu.\n",
				pthread_self());
	}

// LEER archivo escrito y calcular md5sum.

// TODO: Quizas este piola hacer que todos los threads empiecen a leer el archivo de vuelta al mismo tiempo,
//       asi hariamos una prueba completa.

	if (file_get_digest(filename, digest_read) == ERROR) {
		self_terminate_as(ERROR, NULL, NULL );
	}

// Retornar md5sum a quien gestione los threads.
	self_terminate_as(SUCCESS, digest_writen, digest_read);

	return NULL ; //Nunca se deberia llegar aca...
}

void destroy_thread_params(thread_params* params) {

	free(params);

}

int threads_get_and_print_results(pthread_t* threads, int threads_amount,
		char* location, char* prefix) {

	int i;
	thread_return* ret_data;

	print_result_header();

	for (i = 0; i < threads_amount; i++) {

		if (pthread_join(threads[i], (void*) &ret_data) != 0) {
			printf("%d - Error en finalizacion de thread %lu.\n", i + 1,
					threads[i]); //TODO: Emprolijar y desacomplar
			continue;
		}

		if (ret_data->retcode != SUCCESS) {
			printf("%d - El thread %lu termino con error.\n", i + 1, threads[i]); //TODO: Emprolijar y desacomplar
			continue;
		}

		print_result_line(ret_data, i + 1, location, prefix);

		release_thread_retdata(ret_data);

	}

	print_result_trailer();

	return SUCCESS;

}

void threads_destroy(pthread_t* threads, int threads_amount) {

//TODO: Mejorar esto, no es estrictamente necesario.. pero bue...

	free(threads);
}

void self_terminate_as(int retcode, MD5_DIGEST digest_writen,
		MD5_DIGEST digest_read) {

	thread_return* ret = malloc(sizeof(thread_return));

	ret->retcode = retcode;

	if (digest_writen != NULL && digest_read != NULL ) {

		ret->md5sum_writen = malloc(MD5_DIGEST_LENGTH);
		memcpy(ret->md5sum_writen, digest_writen, MD5_DIGEST_LENGTH);

		ret->md5sum_read = malloc(MD5_DIGEST_LENGTH);
		memcpy(ret->md5sum_read, digest_read, MD5_DIGEST_LENGTH);

	}

	pthread_exit((void*) ret);

}

void fill_buffer(char* buffer, int len) {

// TODO: Inicializar con seed una sola vez en tod_o el programa.

	time_t the_time = time(NULL );
	int i;
	char ch;

	srandom((unsigned int) the_time);

	for (i = 0; i < len; i++) {
		ch = (char) random();
		buffer[i] = ch;
	}

}

int file_get_digest(char* filename, MD5_DIGEST digest) {

	FILE* file;
	char buffer[BUFF_LEN];
	int bytes_read;
	MD5_CTX context;

// Inicializar contexto para md5sum.
	MD5_Init(&context);

// Leer archivo actualizando contexto md5sum.
	if ((file = fopen(filename, "r")) == NULL ) {
		printf("Error abriendo archivo para lectura en thread numero %lu.\n",
				pthread_self());
		return ERROR;
	}

	while ((bytes_read = fread(buffer, 1, BUFF_LEN, file)) > 0) {
		MD5_Update(&context, buffer, bytes_read);
	}

	if (fclose(file) == EOF) {
		printf("Error cerrando archivo para lectura en thread numero %lu.\n",
				pthread_self());
		return ERROR;
	}

	if (MD5_Final(digest, &context) != MD5_OK) {
		printf("Error leyendo md5sum de archivo LEIDO en thread numero %lu.\n",
				pthread_self());
		return ERROR;
	}

	return SUCCESS;
}

void print_result_line(thread_return* ret_data, int line_number, char* location,
		char* prefix) {

	char filename[FILENAME_LEN];
	build_file_name(filename, line_number, location, prefix);

	printf("%3d - Archivo: %s\n", line_number, filename);
	printf("%3d - Se escribio con md5sum: ", line_number);
	print_md5sum(ret_data->md5sum_writen);
	printf("\n");
	printf("%3d - Tras la lectura se obtuvo md5sum: ", line_number);
	print_md5sum(ret_data->md5sum_read);
	printf("\n");
	printf("%3d - Resultado: %s.\n", line_number,
			memcmp(ret_data->md5sum_read, ret_data->md5sum_writen,
					MD5_DIGEST_LENGTH) ? "BAD" : "OK");
	printf("\n");
}

void print_result_header() {

	printf("\n");
	printf("Resultados:\n");
	printf("\n\n");

}

void print_md5sum(MD5_DIGEST md5sum) {

	int i;

	for (i = 0; i < MD5_DIGEST_LENGTH; i++)
		printf("%02x", md5sum[i]);

}

void print_result_trailer() {

	printf("\n\n");
	printf("Fin!\n");

}

void release_thread_retdata(thread_return* ret_data) {
	free(ret_data->md5sum_read);
	free(ret_data->md5sum_writen);
	free(ret_data);
}

char* sanitize_location(char* location) {

	int location_len = strlen(location);
	char* sanitized_loc;

	if (location[location_len - 1] != '/') {

		sanitized_loc = malloc(location_len + 1 + 1);
		strcpy(sanitized_loc, location);
		strcat(sanitized_loc, "/");

		return sanitized_loc;

	} else {
		return strdup(location);
	}

}

void destroy_params(global_config config) {
	free(config.location);
	free(config.files_prefix);
}

int sanitize_threads_amount(long int actual_threads_amount) {
	if (actual_threads_amount > MAX_THREADS_AMOUNT) {
		printf("La cantidad de threads se limita a %d.\n\n", MAX_THREADS_AMOUNT);
		return MAX_THREADS_AMOUNT;
	} else
		return actual_threads_amount;
}

int sanitize_file_len(long int actual_file_len){
	if (actual_file_len > MAX_FILE_LEN) {
		printf("El tamano de los archivos se limita a %d.\n\n", MAX_FILE_LEN);
		return MAX_FILE_LEN;
	} else
		return actual_file_len;
}

void print_a_global_header(global_config config) {

	printf("- Massive file creator -\n\n");
	printf("Generando %d archivos de tamaño %dkb en la ruta \"%s\" usando el prefijo \"%s\"\n\n",
			config.threads_amount,
			config.file_len,
			config.location,
			config.files_prefix);
}
