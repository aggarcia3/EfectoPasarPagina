#pragma once

#include "stdafx.h"

// Asegurarse de que tenemos disponibles los macros min y max
#ifndef min
#define min(a, b) (a > b ? b : a)
#endif
#ifndef max
#define max(a, b) (a > b ? a : b)
#endif

// Registro con los par�metros globales del efecto
typedef struct
{
    uint8_t usarSSE;            // Verdadero si deben de usarse las rutinas SSE para procesar el efecto, falso en caso contrario.
    uint8_t bucle;                // Verdadero si realizar el efecto en bucle (empezando de nuevo al terminar la transici�n con la �ltima imagen).
    uint8_t estRendimiento;        // Verdadero si se deben de mostrar las estad�sticas de rendimiento al final de la ejecuci�n del efecto.
    uint8_t ocultarResultado;    // Verdadero si se debe de no mostrar el resultado de la transici�n en una ventana de OpenCV. Activar para mediciones de rendimiento.
    int tiempoEspera;            // Tiempo de espera entre pares de im�genes en la transici�n, en milisegundos.
    unsigned int nHilos;        // N�mero de hilos que se est�n usando para procesar este efecto.
    IplImage* res;              // Imagen resultado, donde se copia el resultado de la transici�n. Debe de tener el mismo tama�o que actual y sig.
    IplImage* actual;            // Imagen actual, de la que se hace la transici�n a siguiente.
    IplImage* sig;                // Imagen siguiente.
    uint8_t omega;                // Fase de giro, en grados (0-180�).
    int xEje;                   // Columna de centro de giro, en p�xeles.
    uint8_t activarSombra;      // 0 si no se debe de dibujar una sombra, un valor diferente en caso contrario.
    unsigned short anchoSombra; // Ancho de la sombra central, en p�xeles.
    uint8_t pasoGiro;           // En grados (1-90�).
} param_efecto;

// Registro con los par�metros de cada hilo usado para procesar el efecto
// Declaraci�n de tipo incompleta para poder introducir el puntero al procedimiento dentro de la estructura
typedef struct param_hilo param_hilo;
typedef void (*trabajoParalelo) (param_hilo*);
struct param_hilo
{
    trabajoParalelo trabajo;        // Trabajo a ejecutar por el hilo.
    param_efecto* paramsEfecto;        // Los par�metros del efecto ejecutado por el hilo.
    unsigned int idHilo;            // Identificador del hilo de ejecuci�n particular, usado para repartir el trabajo.
    pthread_t esteHilo;                // Alberga informaci�n sobre este hilo.
};

// ---------- //
// Constantes //
// ---------- //

// El nombre de la ventana donde se mostrar�n los resultados
#define NOMBRE_VENTANA "Efecto"

// Tama�o del b�fer de entrada de datos del usuario.
// Debe de ser lo suficientemente grande como para que sea muy poco probable que se reciban m�s caracteres de los que contiene
// (si se reciben m�s caracteres, las siguientes entradas usar�n caracteres sobrantes que se quedaron en el b�fer del SO)
// Solo se usar� si no se est� compilando la aplicaci�n con interfaz gr�fica Win32.
#define TAM_BUF 256

// Los canales de color que tendr�n las im�genes al ser procesadas con SSE.
// La rutina sin SSE soporta im�genes de cualquier n�mero de canales.
#define CANALES_COLOR_SSE 4