#ifndef KERNEL_MEMORIA_H_
#define KERNEL_MEMORIA_H_

#include <stdlib.h>
#include <stdio.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <semaphore.h>
#include <commons/collections/queue.h>
#include <pthread.h>

#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"

#include "extern_globales.h"



pthread_mutex_t mutex_cont_plp;
pthread_mutex_t mutex_cont_pcp;
int32_t cantidad_procesos_bloq_plp=0;
int32_t cantidad_procesos_bloq_pcp=0;
bool barrera_activada= false;













#endif /*  KERNEL_MEMORIA_H_ */
