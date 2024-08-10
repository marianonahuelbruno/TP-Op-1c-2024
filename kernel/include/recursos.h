
#ifndef TPANUAL_RECURSOS_H
#define TPANUAL_RECURSOS_H

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
#include <pthread.h>
#include <semaphore.h>
#include <commons/collections/queue.h>

#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"
#include <commons/collections/dictionary.h>

#include "extern_globales.h"





t_recurso* lista_de_recursos;
int32_t cantidadDeRecursos;





/*
typedef struct
{
    uint32_t instancias;
    uint32_t n_recurso;
    t_list *bloqueados;
} t_recurso;

t_dictionary *dict_recursos;
char** config_recursos;
char** config_instancias_recursos;
int32_t cantidad_recursos;

int32_t obtener_cantidad_recursos(char** config_recursos);

void cargar_recursos(char** recursos, char** instancias_recursos, uint32_t cant_recursos);

void liberar_recursos(t_pcb* pcb, char** lista_recursos);


*/
#endif //TPANUAL_RECURSOS_H
