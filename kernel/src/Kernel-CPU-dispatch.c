#include "../include/Kernel-CPU-dispatch.h"
    
void atender_conexion_CPU_DISPATCH_KERNEL(){
  

//ENVIAR MENSAJE A CPU-DISPATCH
    enviar_mensaje("CONEXION CON KERNEL OK", socket_kernel_cpu_dispatch);
    log_info(logger, "Handshake enviado: CPU-DISPATCH");

    gestionar_dispatch ();
    
}

