#pragma once

#include "stdafx.h"

// Asegurarse de que tenemos disponibles los macros min y max
#ifndef min
#define min(a, b) (a > b ? b : a)
#endif
#ifndef max
#define max(a, b) (a > b ? a : b)
#endif

// Registro con los parámetros globales del efecto
typedef struct
{
    uint8_t usarSSE;            // Verdadero si deben de usarse las rutinas SSE para procesar el efecto, falso en caso contrario.
    uint8_t bucle;                // Verdadero si realizar el efecto en bucle (empezando de nuevo al terminar la transición con la última imagen).
    uint8_t estRendimiento;        // Verdadero si se deben de mostrar las estadísticas de rendimiento al final de la ejecución del efecto.
    uint8_t ocultarResultado;    // Verdadero si se debe de no mostrar el resultado de la transición en una ventana de OpenCV. Activar para mediciones de rendimiento.
    int tiempoEspera;            // Tiempo de espera entre pares de imágenes en la transición, en milisegundos.
    unsigned int nHilos;        // Número de hilos que se están usando para procesar este efecto.
    IplImage* res;              // Imagen resultado, donde se copia el resultado de la transición. Debe de tener el mismo tamaño que actual y sig.
    IplImage* actual;            // Imagen actual, de la que se hace la transición a siguiente.
    IplImage* sig;                // Imagen siguiente.
    uint8_t omega;                // Fase de giro, en grados (0-180º).
    int xEje;                   // Columna de centro de giro, en píxeles.
    uint8_t activarSombra;      // 0 si no se debe de dibujar una sombra, un valor diferente en caso contrario.
    unsigned short anchoSombra; // Ancho de la sombra central, en píxeles.
    uint8_t pasoGiro;           // En grados (1-90º).
} param_efecto;

// Registro con los parámetros de cada hilo usado para procesar el efecto
// Declaración de tipo incompleta para poder introducir el puntero al procedimiento dentro de la estructura
typedef struct param_hilo param_hilo;
typedef void (*trabajoParalelo) (param_hilo*);
struct param_hilo
{
    trabajoParalelo trabajo;        // Trabajo a ejecutar por el hilo.
    param_efecto* paramsEfecto;        // Los parámetros del efecto ejecutado por el hilo.
    unsigned int idHilo;            // Identificador del hilo de ejecución particular, usado para repartir el trabajo.
    pthread_t esteHilo;                // Alberga información sobre este hilo.
};

// ---------- //
// Constantes //
// ---------- //

// El nombre de la ventana donde se mostrarán los resultados
#define NOMBRE_VENTANA "Efecto"

// Tamaño del búfer de entrada de datos del usuario.
// Debe de ser lo suficientemente grande como para que sea muy poco probable que se reciban más caracteres de los que contiene
// (si se reciben más caracteres, las siguientes entradas usarán caracteres sobrantes que se quedaron en el búfer del SO)
// Solo se usará si no se está compilando la aplicación con interfaz gráfica Win32.
#define TAM_BUF 256

// Los canales de color que tendrán las imágenes al ser procesadas con SSE.
// La rutina sin SSE soporta imágenes de cualquier número de canales.
#define CANALES_COLOR_SSE 4