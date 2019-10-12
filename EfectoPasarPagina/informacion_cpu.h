#pragma once

#include "stdafx.h"

// Comprueba si la CPU sobre la que se ejecuta este programa soporta SSE2.
// Devuelve falso si no soporta SSE2, verdadero en caso contrario.
uint8_t ProcesadorSoportaSSE2();

// Obtiene el n�mero de procesadores l�gicos (hilos de CPU) disponibles en el sistema.
unsigned int GetNumeroProcesadores();