#pragma once

#include "stdafx.h"
#include "efecto.h"

// Resultado del intento de inicializaci�n del efecto
typedef enum
{
    CORRECTA = EXIT_SUCCESS,
    ERR_MEM,
    ERR_ARGS_LINEA_COMANDOS,
    ERR_CARGA_IMG,
    ERR_PROPIEDADES_IMG,
    ERR_CREACION_HILO,
    ERR_IMGS_INSUFICIENTES
} resOperacionEfecto;

typedef struct
{
    size_t nImgs;
    size_t nMaxImgs;
    IplImage** pArrImgs;
} imagenesCargadas;

extern imagenesCargadas stImgsCargadas;

#ifdef SUBSISTEMA_GRAFICO_WIN32
extern const TCHAR* mensajesError[];
#else
#undef TEXT
#define TEXT(quote) quote    // Macro NOP
extern const char* mensajesError[];
#endif

// A�ade una imagen a la lista de im�genes cargadas si es posible.
// No se encarga de inicializar las estructuras de datos necesarias para albergar im�genes.
#ifdef SUBSISTEMA_GRAFICO_WIN32
resOperacionEfecto CargarImg(LPCTSTR szRuta);
#else
resOperacionEfecto CargarImg(const char* ruta);
#endif

// Unifica la resoluci�n de todas las im�genes cargadas hasta la fecha, para que tengan el mismo tama�o.
resOperacionEfecto AdecuarImgs();

// Libera la memoria usada por la estructura con los datos de las im�genes cargadas.
void DescargarImgs();

// Obtiene la configuraci�n del efecto. Si no existe todav�a, crea una predeterminada.
// Esta funci�n puede devolver un puntero nulo en caso de que no se pueda crear la configuraci�n.
param_efecto* GetConfiguracionEfecto();

// Inicia la visualizaci�n del efecto de transici�n.
void IniciarEfecto();