#ifndef UTILS_HELLO_H_
#define UTILS_HELLO_H_

#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include <commons/collections/list.h>
#include<readline/readline.h>
#include <semaphore.h>
#include <pthread.h>

typedef struct RECURSOS {
    char* nombre_recurso;        
    int32_t instancias_del_recurso; 
    int32_t instancias_solicitadas_del_recurso;  
    t_list* lista_de_espera;
    t_list* lista_de_asignaciones;     
    struct RECURSOS* siguiente_recurso; 
} t_recurso;

typedef enum instrucciones
{
	SET,
	SUM,
	SUB,
	MOV_IN,
	MOV_OUT,
    RESIZE,
    JNZ,
	COPY_STRING,
	IO_GEN_SLEEP,
    IO_STDIN_READ,
    IO_STDOUT_WRITE,
    IO_FS_CREATE,
    IO_FS_DELETE,
    IO_FS_TRUNCATE,
    IO_FS_WRITE,
    IO_FS_READ,
	WAIT,
	SIGNAL,
	EXIT,
    VOLVER
} cod_ins;

typedef struct instruccion
{
    cod_ins ins;
    char* arg1;
    char* arg2;
    char* arg3;
    char* arg4;
    char* arg5;
} t_instruccion;

typedef enum 
{
    NEW,
    READY,
    READY_PRIORITARIO,
    EXEC,
    BLOCKED,
    BLOCKED_PRIORITARIO,
    EXITT,
    ERROR
} t_estado;

typedef struct contexto_de_ejecucion_de_proceso
{
    uint32_t PC;
    uint8_t AX;
    uint8_t BX;
    uint8_t CX;
    uint8_t DX;
    uint32_t EAX;
    uint32_t EBX;
    uint32_t ECX;
    uint32_t EDX;
    uint32_t SI;
    uint32_t DI;
} t_contexto_ejecucion;

typedef struct pcb_de_proceso
{
   	uint32_t PID;
    t_estado estado;
    int64_t quantum_ejecutado;
    t_contexto_ejecucion CE;
    
} t_pcb;

typedef enum CODIGOS_DE_INTERFACES
{
    GENERICA,
    STDIN,
    STDOUT,
    DIALFS
} cod_interfaz;



 typedef struct 
{
    char* nombre_interfaz;
    cod_interfaz tipo_interfaz;
    int32_t socket_interfaz;
    t_list* cola_de_espera;
    sem_t control_envio_interfaz;
    sem_t utilizacion_interfaz;
} IO_type;




//Funciones de Utils.
void decir_hola(char* );
void read_console(t_log *);
t_config *start_config(char* path);
t_log *start_logger(char* fileName, char* processName, t_log_level logLevel);
void end_program(t_log* logger, t_config* config);
char* codigo_estado_string(t_estado codigo);
char* codigo_instruccion_string(cod_ins codigo);
#endif
