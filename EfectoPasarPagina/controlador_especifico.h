#pragma once

#include "stdafx.h"
#include "controlador_efecto.h"

#ifdef SUBSISTEMA_GRAFICO_WIN32
// Carga las im�genes seleccionadas por el usuario en el cuadro de di�logo especificado, reemplazando las que pudiera haber
// cargado anteriormente.
resOperacionEfecto PrepararImgs(HWND hwndDlg);

// Intenta cargar im�genes sin interacci�n con el usuario.
// Puede fallar porque no se pueda cargar alguna imagen.
resOperacionEfecto PrepararImgsLC(LPCTSTR* szNombreFichero, size_t nFicheros);

// Muestra la configuraci�n actual del efecto en el cuadro de di�logo, inicializando sus par�metros visuales.
void MostrarConfig(HWND* hwndDlg, param_efecto* paramsEfecto);

// Comprueba que el valor introducido por el usuario en el cuadro de edici�n es v�lido, y si no lo es toma medidas correctoras o informativas.
void ValidarValor(HWND* hwndDlg, WORD control);

// Aplica la configuraci�n definida en el estado actual de la ventana de configuraci�n.
void AplicarConfig(HWND* hwndDlg);

#else
// Intenta cargar im�genes sin interacci�n con el usuario, bajo el contexto de una aplicaci�n no gr�fica.
// Puede fallar porque no se pueda cargar alguna imagen.
resOperacionEfecto PrepararImgsLC(char** imgs, int nImgs);
#endif