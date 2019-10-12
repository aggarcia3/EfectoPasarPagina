#pragma once

#include "stdafx.h"
#include "efecto.h"

// Resultado del intento de inicialización del efecto
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

// Añade una imagen a la lista de imágenes cargadas si es posible.
// No se encarga de inicializar las estructuras de datos necesarias para albergar imágenes.
#ifdef SUBSISTEMA_GRAFICO_WIN32
resOperacionEfecto CargarImg(LPCTSTR szRuta);
#else
resOperacionEfecto CargarImg(const char* ruta);
#endif

// Unifica la resolución de todas las imágenes cargadas hasta la fecha, para que tengan el mismo tamaño.
resOperacionEfecto AdecuarImgs();

// Libera la memoria usada por la estructura con los datos de las imágenes cargadas.
void DescargarImgs();

// Obtiene la configuración del efecto. Si no existe todavía, crea una predeterminada.
// Esta función puede devolver un puntero nulo en caso de que no se pueda crear la configuración.
param_efecto* GetConfiguracionEfecto();

// Inicia la visualización del efecto de transición.
void IniciarEfecto();