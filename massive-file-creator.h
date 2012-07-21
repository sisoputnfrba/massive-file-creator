/*
 * massive-file-creator.h
 *
 *  Created on: Jul 21, 2012
 *      Author: marke
 */

#ifndef MASSIVE_FILE_CREATOR_H_
#define MASSIVE_FILE_CREATOR_H_



#define ERROR -1
#define SUCCESS 0

#define DEFAULT_ARGS_AMOUNT 3

// Datos de configuracion general
typedef struct {
	int threads_amount;
	int files_per_thread;
	char* location;
} config_data;

int get_params(char** params, int params_amount, config_data* config);

#endif /* MASSIVE_FILE_CREATOR_H_ */
