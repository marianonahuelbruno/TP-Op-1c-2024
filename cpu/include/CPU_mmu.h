#ifndef CPU_MMU_H_
#define CPU_MMU_H_

#include <math.h>
#include <commons/temporal.h>
#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"

extern t_log* logger;
extern t_log* logger_debug;

extern uint32_t PID;
extern int32_t socket_cpu_memoria;

extern uint32_t cant_entradas_TLB;
extern char* algoritmo_TLB;
extern uint32_t tamanio_de_pagina;
extern bool usa_TLB;
extern t_list* tabla_TLB;
extern sem_t respuesta_marco;
extern uint32_t marco_pedido;
typedef struct
{
    uint32_t PID;
    uint32_t nro_pag;
    uint32_t marco;
    t_temporal* t_ingreso;
    t_temporal* t_ultimo_uso;
} entrada_TLB;

void inicializar_TLB();
uint32_t obtener_nro_pagina(uint32_t direccion_logica);
uint32_t obtener_desplazamiento(uint32_t direccion_logica);

uint32_t get_marco(uint32_t PID, uint32_t nro_pag);
entrada_TLB* buscar_en_tlb(uint32_t PID, uint32_t nro_pag);
uint32_t marco_TLB(entrada_TLB* entrada);
uint32_t pedir_marco_a_memoria(uint32_t PID, uint32_t nro_pag);
entrada_TLB* TLB_miss(uint32_t PID, uint32_t nro_pag);
uint32_t buscar_entrada_para_reemplazar();
uint32_t algoritmo_de_reemplazo(uint32_t indice_1, uint32_t indice_2);

void solicitar_lectura_string(uint32_t direccion_logica_READ, uint32_t bytes_a_copiar);
void escribir_en_memoria_string(char* string_leida, uint32_t direccion_logica_WRITE, uint32_t bytes_a_copiar);

uint32_t solicitar_MOV_IN(uint32_t direccion_logica, uint32_t tamanio_registro);
uint32_t solicitar_MOV_OUT(uint32_t direccion_logica, uint32_t tamanio_registro, int valor);
op_code recibir_respuesta_MOV_OUT();

#endif //CPU_MMU_H_
