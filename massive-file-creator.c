/*
 * massive-file-creator.c
 *
 *  Created on: Jul 21, 2012
 *      Author: marke
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "massive-file-creator.h"

/*------------------------------------------------------
 Variables globales
 -----------------------------------------------------*/

config_data config;

int main(int argc, char **argv) {

	if(get_params(argv + 1, argc, &config) != SUCCESS){
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int get_params(char** params, int params_amount, config_data* config_data) {

	int i;

	if (params_amount != DEFAULT_ARGS_AMOUNT) {
		return ERROR;
	}

	for (i = 0; i <= params_amount; i++) {
		switch (i) {
		case 1:
			config_data->threads_amount = strtol(params[i], NULL, 10);
			break;
		case 2:
			config_data->files_per_thread = strtol(params[i], NULL, 10);
			break;
		case 3:
			config_data->location = strdup(params[i]);
			break;
		}
	}

	return SUCCESS;

}
