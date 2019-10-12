#pragma once

#include "stdafx.h"
#include "controlador_efecto.h"

#ifdef SUBSISTEMA_GRAFICO_WIN32

// Opciones extra�das de la l�nea de comandos
typedef struct
{
    BOOL ocultarGUI;
} opcionesLC;

// Procesa los argumentos de la l�nea de comandos.
resOperacionEfecto ProcesarALC(LPTSTR, opcionesLC*);

#else

// Procesa los argumentos de la l�nea de comandos.
resOperacionEfecto ProcesarALC(int argc, char** argv);

#endif