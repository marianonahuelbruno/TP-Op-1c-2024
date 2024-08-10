#ifndef SERVICIOS_KERNEL_H_
#define SERVICIOS_KERNEL_H_

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

#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"

#include <pthread.h>
#include <semaphore.h>
#include <commons/collections/queue.h>


#include "extern_globales.h"


uint32_t identificador_PID=1;
pthread_mutex_t mutex_pid;













#endif /*  SERVICIOS_KERNEL_H_ */
