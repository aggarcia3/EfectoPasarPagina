#pragma once

#include "stdafx.h"
#include "controlador_efecto.h"

#ifdef SUBSISTEMA_GRAFICO_WIN32

// Opciones extraídas de la línea de comandos
typedef struct
{
    BOOL ocultarGUI;
} opcionesLC;

// Procesa los argumentos de la línea de comandos.
resOperacionEfecto ProcesarALC(LPTSTR, opcionesLC*);

#else

// Procesa los argumentos de la línea de comandos.
resOperacionEfecto ProcesarALC(int argc, char** argv);

#endif