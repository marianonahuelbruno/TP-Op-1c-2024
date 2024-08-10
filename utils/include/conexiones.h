#ifndef CONEXIONES_TP_H_
#define CONEXIONES_TP_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <readline/readline.h>
#include <commons/log.h>
#include "utils.h"

#include <commons/memory.h>

/*----------Estructuras----------*/

// ---------- CÓDIGOS DE OPERACIÓN ---------- //
typedef enum CODIGOS_DE_OPERACIONES{
    CONTEXTO,
	SIG_INS,
	FETCH,
    PROCESO_NO_CARGADO,
	TAM_PAG,
    TLB_MISS, // aviso para cpu: si llega -1 es que la pag no tiene asignado ningun marco 
    CREAR_PROCESO,
    ELIMINAR_PROCESO,
    CARGA_EXITOSA_PROCESO,
    ERROR_AL_CARGAR_EL_PROCESO,
    OUT_OF_MEMORY,
    SOLICITUD_IO_STDIN_READ, 
    SOLICITUD_IO_STDOUT_WRITE, 
    SOLICITUD_MOV_OUT,
    SOLICITUD_MOV_IN,
    SOLICITUD_RESIZE,
    SOLICITUD_COPY_STRING_READ,
    SOLICITUD_COPY_STRING_WRITE,
	RECIBIR_CE_DISPATCH,
    RETORNAR,
	MENSAJE,
    HANDSHAKE,
    PAQUETE,
    DESALOJO_POR_WAIT,
    DESALOJO_POR_SIGNAL,
    DESALOJO_POR_QUANTUM,
    DESALOJO_POR_FIN_PROCESO,
    DESALOJO_POR_CONSOLA,    
    DESALOJO_POR_INTERRUPCION,
    DESALOJO_POR_IO_GEN_SLEEP,
    DESALOJO_POR_IO_STDIN,
    DESALOJO_POR_IO_STDOUT,
    DESALOJO_POR_IO_FS_CREATE,
    DESALOJO_POR_IO_FS_DELETE,
    DESALOJO_POR_IO_FS_TRUNCATE,
    DESALOJO_POR_IO_FS_WRITE,
    DESALOJO_POR_IO_FS_READ,
    VERIFICAR_CONEXION,
    FINALIZA_IO,
    OK,
    FALLO,
    NUEVA_IO,
    SOLICITUD_EXITOSA_IO,
    ERROR_SOLICITUD_IO,
    INT_NO,
    INT_QUANTUM,
    INT_CONSOLA
} op_code;


typedef struct
{
    uint32_t size;
    void* stream;
} t_buffer;

typedef struct
{
    op_code codigo_operacion;
    t_buffer* buffer;
} t_paquete;

 typedef struct 
{
    uint32_t PID_cola;
    t_paquete* paquete_cola; 

} t_pid_paq;


/*----------Fin Estructuras----------*/

/*----------Cliente----------*/

int32_t crear_conexion(char* ip, char* puerto);
void liberar_conexion(int32_t socket_cliente);

/*----------Fin Cliente----------*/

/*----------Servidor----------*/

int32_t iniciar_servidor(char* puerto_escucha, t_log* logger);
int32_t esperar_cliente(int32_t socket, t_log* logger);

/*----------Fin Servidor----------*/

/*----------Mensajeria----------*/
void enviar_mensaje(char* mensaje, uint32_t socket);
t_paquete* crear_paquete(op_code codigo);
void crear_buffer(t_paquete* paquete);
void enviar_paquete(t_paquete* paquete, int32_t socket);
void* serializar_paquete(t_paquete* paquete, uint32_t bytes);
void eliminar_paquete(t_paquete* paquete);

op_code recibir_operacion(int32_t socket);
t_list* recibir_paquete(int32_t socket);
void recibir_mensaje(int32_t socket, t_log* logger);
void* recibir_buffer(uint32_t* size, int32_t socket);

void enviar_CE(int32_t socket, uint32_t PID, t_contexto_ejecucion contexto);
void recibir_CE(int32_t socket, uint32_t* PID, t_contexto_ejecucion* contexto_contenedor);
/*----------Fin Mensajeria----------*/


/*----------Serializacion----------*/
void agregar_a_paquete_uint8(t_paquete* paquete, uint8_t numero);
void agregar_a_paquete_uint32(t_paquete* paquete, uint32_t numero);
void agregar_a_paquete_string(t_paquete* paquete, uint32_t tamanio, char* string);
void agregar_a_paquete_cod_ins(t_paquete* paquete, cod_ins codigo);
void agregar_a_paquete_bytes(t_paquete* paquete, uint32_t tamanio, void* bytes);

uint8_t leer_de_buffer_uint8(void* buffer, uint32_t* desplazamiento);
uint32_t leer_de_buffer_uint32(void* buffer, uint32_t* desplazamiento);
char* leer_de_buffer_string(void* buffer, uint32_t* desplazamiento);
cod_ins leer_de_buffer_cod_ins(void* buffer, uint32_t* desplazamiento);
void* leer_de_buffer_bytes(void* buffer, uint32_t* desplazamiento, uint32_t tamanio_leido);

void agregar_a_paquete_op_code(t_paquete* paquete, op_code codigo);
op_code leer_de_buffer_op_code(void* buffer, uint32_t* desplazamiento);

void serializar_CE(t_paquete* paquete, t_contexto_ejecucion contexto);
void leer_de_buffer_CE(void* buffer, uint32_t* desplazamiento, t_contexto_ejecucion* contexto_contenedor);

void agregar_a_paquete_cod_interfaz(t_paquete* paquete, cod_interfaz interfaz);
cod_interfaz leer_de_buffer_tipo_interfaz(void* buffer, uint32_t* desplazamiento);

void enviar_instruccion_con_PID_por_socket(op_code codigo_operacion, uint32_t PID,int32_t socket_a_enviar);
uint32_t recibir_de_buffer_solo_PID(int32_t socket_a_recibir);

char* codigo_operacion_string(op_code codigo);

/*----------Fin Serializacion----------*/

/*----------Pruebas funcionamiento----------*/
void verificar_paquete(void* );
void imprimir_paquete(t_paquete* paquete);
#endif //CONEXIONES_TP_H_
