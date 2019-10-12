#pragma once

#include "efecto.h"

// Este fichero de cabecera permite elegir en tiempo de ejecuci�n entre diferentes estrategias
// de uso de instrucciones de CPU para renderizar el efecto.

// No SSE

// Declara e inicializa variables usadas por el efecto de transici�n no SSE.
// Debe de llamarse este procedimiento solo una vez, antes de iniciar el efecto.
// Se asume que todos los par�metros pasados a esta estructura son constantes, a excepci�n de omegaRad, actual y sig.
void preparar(param_efecto*);
// Copia la imagen actual a resultado.
void copiaInicial(param_hilo*);
// Giro de la mitad derecha de la imagen actual, primeros 90�.
void primeraMitad(param_hilo*);
// Eliminar artefactos visuales que pudiesen surgir bajo pasos de giro no divisores de 90�, tras acabar con la primera mitad.
void eliminarArtefactosPasoGiro(param_hilo* params);
// Giro de la mitad izquierda de la imagen siguiente, siguientes 90�.
void segundaMitad(param_hilo*);

// SSE

// Declara e inicializa variables usadas por el efecto de transici�n SSE.
// Debe de llamarse este procedimiento solo una vez, antes de iniciar el efecto.
// Se asume que todos los par�metros pasados a esta estructura son constantes, a excepci�n de omegaRad, actual y sig.
void prepararSSE(param_efecto*);
// Copia la imagen actual a resultado.
void copiaInicialSSE(param_hilo*);
// Giro de la mitad derecha de la imagen actual, primeros 90�.
void primeraMitadSSE(param_hilo*);
// Eliminar artefactos visuales que pudiesen surgir bajo pasos de giro no divisores de 90�, tras acabar con la primera mitad.
void eliminarArtefactosPasoGiroSSE(param_hilo* params);
// Giro de la mitad izquierda de la imagen siguiente, siguientes 90�.
void segundaMitadSSE(param_hilo*);