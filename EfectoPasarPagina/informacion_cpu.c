#include "informacion_cpu.h"

// ---------------------------------------------------------------------- //
// Comprueba si la CPU sobre la que se ejecuta este programa soporta SSE2 //
// ---------------------------------------------------------------------- //

uint8_t ProcesadorSoportaSSE2()
{
    // Suponemos que la instrucción CPUID está disponible. Si lo está, podemos ver si el procesador
    // soporta SSE2. Esta instrucción fue introducida por Intel en 1993, en los procesadores Pentium
    // y 80486. Windows 7 ya necesita un procesador mejor que esos para funcionar, así que su disponibilidad
    // es una suposición bastante razonable
#ifdef _MSC_VER
    int registrosCPU[4];    // EAX, EBX, ECX, EDX
    __cpuid(registrosCPU, 1);
    return (registrosCPU[3] & 0x04000000) != 0;
#elif __GNUC__
    int eax, ebx, ecx, edx;
    __get_cpuid(1, &eax, &ebx, &ecx, &edx);
    return (edx & bit_SSE2) != 0;
#endif
}

unsigned int GetNumeroProcesadores()
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return (unsigned int) info.dwNumberOfProcessors;    // Esto cuenta los hilos proporcionados por HyperThreading
}