#ifndef TP_MEMORIA_PAGINACION_H_
#define TP_MEMORIA_PAGINACION_H_

#include <commons/bitarray.h>
#include <readline/readline.h>
#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"
#include "extGlobales.h"

typedef struct{
    int pid;
    t_list* instrs;
    t_list* paginas;
}procesoM;

typedef struct{
    int marco;
    bool presencia;
}tabla_pag; //tabla de paginas en si

typedef struct {
    int pid;                  
    t_list* paginas;          // lista de tablas_pag
}tabla_pag_proceso; //tabla de paginas de cada proceso

bool crear_procesoM(char* path_instrucciones, uint32_t PID);
void eliminar_procesoM(uint32_t PID);

procesoM* buscar_proceso_por_pid(uint32_t pid);
t_list* obtener_instrs(uint32_t pid);

// tablas
void añadirTablaALista(t_list* paginas, uint32_t PID);
//t_list* crear_tabla_pag(uint32_t size);
//void crear_paginas(uint32_t PID, int cantidad_paginas);
tabla_pag_proceso* obtener_tabla_pag_proceso(uint32_t PID);
tabla_pag* obtener_pagina_proceso(tabla_pag_proceso* tabla_proceso, int numero_pagina);
tabla_pag* buscar_siguiente_pagina(tabla_pag_proceso* tabla_proceso, int marco_actual);

int asignar_marco(uint32_t PID, int numero_pagina);
void liberar_frames(t_list* paginas);
uint32_t encontrar_frame(uint32_t PID, uint32_t pagina);
int obtener_marco(int direccion_fisica);
int obtener_desplazamiento(int direccion_fisica);

bool resize(uint32_t PID, uint32_t size);
bool añadir_pagina_a_proceso(tabla_pag_proceso* tabla, uint32_t num_paginas, uint32_t PID);
void eliminar_pagina_de_proceso(tabla_pag_proceso* tabla, int num_paginas);

bool escribir_memoria(uint32_t direccion_fisica, uint32_t bytes, void* valor, uint32_t PID);

void* leer_memoria(uint32_t direccion_fisica, uint32_t tamanio_acceso, uint32_t PID);
char* leer_memoria_string(uint32_t direccion_fisica, uint32_t size, uint32_t PID);


#endif //TP_MEMORIA_PAGINACION_H_
