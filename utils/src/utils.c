#include "../include/utils.h"
#include <commons/config.h>
#include <commons/log.h>
#include <readline/readline.h>

void decir_hola(char* quien) {
    printf("Estassss %s ON!!\n", quien);
}

void read_console(t_log *logger) {
    char* readed;
    readed = readline(">> ");
    log_info(logger, ">> %s", readed);

    while (strcmp(readed, "")!=0) {
        free(readed);
        readed = readline(">> ");
        log_info(logger, ">> %s", readed);
    };
    free(readed);
}

t_config *start_config(char* path) {
    t_config* new_config = config_create(path);
    if(new_config == NULL){
        perror("Error al cargar la configuracion de la CPU.");
        exit(EXIT_FAILURE);
    }

    return new_config;
}

t_log *start_logger( char* fileName, char* processName, t_log_level logLevel) {
    t_log* new_logger = log_create( fileName,  processName, 1, logLevel);
    if(new_logger == NULL){
        perror("Error al crear el Log.");
        exit(EXIT_FAILURE);
    }
    return new_logger;
}


void end_program(t_log* logger, t_config* config)
{

    log_destroy(logger);
    config_destroy(config);
}

char* codigo_estado_string(t_estado codigo){  ////////////////////////// TRANSFORMA EL CODIGO DE ESTADO EN UN STRING PARA LOGGEO
	if(codigo == NEW) 				return "NUEVO";
	if(codigo == READY) 			return "READY";
    if(codigo == READY_PRIORITARIO) return "READY_PRIORITARIO";
	if(codigo == EXEC) 	        	return "EJECUCION";
	if(codigo == BLOCKED) 			return "BLOQUEO";
	if(codigo == EXITT) 			return "EXIT";
    if(codigo == BLOCKED_PRIORITARIO) 			return "BLOCKEO_PRIORITARIO";
	else							return "ERROR";
}

char* codigo_instruccion_string(cod_ins codigo){                    ////////////////////////// TRANSFORMA EL CODIGO DE INSTRUCCION EN UN STRING PARA LOGGEO
	
	if (codigo == SET)                     return	"SET";
	if (codigo == SUM)                     return	"SUM";
	if (codigo == SUB)                     return	"SUB";
	if (codigo == MOV_IN)                  return	"MOV_IN";
	if (codigo == MOV_OUT)                 return	"MOV_OUT";
    if (codigo == RESIZE)                  return   "RESIZE";
    if (codigo == JNZ)                     return   "JNZ";
	if (codigo == COPY_STRING)             return	"COPY_STRING";
	if (codigo == IO_GEN_SLEEP)            return	"IO_GEN_SLEEP";
    if (codigo == IO_STDIN_READ)           return   "IO_STDIN_READ";
    if (codigo == IO_STDOUT_WRITE)         return   "IO_STDOUT_WRITE";
    if (codigo == IO_FS_CREATE)            return   "IO_FS_CREATE";
    if (codigo == IO_FS_DELETE)            return   "IO_FS_DELETE";
    if (codigo == IO_FS_TRUNCATE)          return   "IO_FS_TRUNCATE";
    if (codigo == IO_FS_WRITE)             return   "IO_FS_WRITE";
    if (codigo == IO_FS_READ)              return   "IO_FS_READ";
	if (codigo == WAIT)                    return	"WAIT";
	if (codigo == SIGNAL)                  return	"SIGNAL";
	if (codigo == EXIT)                    return	"EXIT";
    else							return "ERROR";
}