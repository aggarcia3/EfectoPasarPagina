#pragma once

#include "stdafx.h"
#include "controlador_efecto.h"

#ifdef SUBSISTEMA_GRAFICO_WIN32
// Carga las imágenes seleccionadas por el usuario en el cuadro de diálogo especificado, reemplazando las que pudiera haber
// cargado anteriormente.
resOperacionEfecto PrepararImgs(HWND hwndDlg);

// Intenta cargar imágenes sin interacción con el usuario.
// Puede fallar porque no se pueda cargar alguna imagen.
resOperacionEfecto PrepararImgsLC(LPCTSTR* szNombreFichero, size_t nFicheros);

// Muestra la configuración actual del efecto en el cuadro de diálogo, inicializando sus parámetros visuales.
void MostrarConfig(HWND* hwndDlg, param_efecto* paramsEfecto);

// Comprueba que el valor introducido por el usuario en el cuadro de edición es válido, y si no lo es toma medidas correctoras o informativas.
void ValidarValor(HWND* hwndDlg, WORD control);

// Aplica la configuración definida en el estado actual de la ventana de configuración.
void AplicarConfig(HWND* hwndDlg);

#else
// Intenta cargar imágenes sin interacción con el usuario, bajo el contexto de una aplicación no gráfica.
// Puede fallar porque no se pueda cargar alguna imagen.
resOperacionEfecto PrepararImgsLC(char** imgs, int nImgs);
#endif