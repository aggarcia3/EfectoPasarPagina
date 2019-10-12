/* stdafx.h: archivo de inclusi�n de los archivos de inclusi�n est�ndar del sistema
   o archivos de inclusi�n espec�ficos de un proyecto utilizados frecuentemente,
   pero rara vez modificados
*/

#pragma once

// Si se define la constante que viene a continuaci�n, esta aplicaci�n se compilar� como una aplicaci�n
// gr�fica Win32. En caso contrario, se compila utilizando solamente funciones disponibles en las librer�as
// est�ndar de C, OpenCV y Cygwin.

// La aplicaci�n gr�fica Win32 no est� dise�ada para ser compilada en Netbeans, aunque deber�a de ser posible
// adaptarla con el suficiente empe�o.

// Tanto la aplicaci�n gr�fica como la de consola deben de ser ejecutadas en un SO Windows.

// Si se est� usando Microsoft Visual Studio, es posible que haya que cambiar las opciones del vinculador para indicarle
// que use el subsistema de consola (si no, esperar� encontrar un punto de entrada inexistente para la aplicaci�n).

#define SUBSISTEMA_GRAFICO_WIN32

// Marca la aplicaci�n como una compilaci�n de depuraci�n, activando caracter�sticas de depuraci�n.
//#define DEPURACION

#ifdef SUBSISTEMA_GRAFICO_WIN32
// Compilar para Windows 7 y posteriores SO
#define _WIN32_WINNT 0x0601

#include <winsdkver.h>
#include <SDKDDKVer.h>

// Excluir de la compilaci�n cabeceras que no vamos a usar para mayor velocidad
#define VC_EXTRALEAN
#define NOCLIPBOARD
#define NOIME
#define NOMCX
#define NOSERVICE
#define NOKANJI
#define NOCOMM
#define NOPROFILER
#define MMNOJOY
#define MMNOAUX
#define MMNOMIDI
#define MMNOMMIO
#define MMNOMCI
#define MMNODRV
#endif

#include <windows.h>
#ifdef SUBSISTEMA_GRAFICO_WIN32
#include <string.h>
#include <tchar.h>
#include <mmsystem.h>
#include <shellapi.h>
#include <commctrl.h>
#endif

#include <highgui.h>
#include <cv.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#define _TIMESPEC_DEFINED
#include <pthread.h>
#include <emmintrin.h>
#ifdef _MSC_VER
#include <intrin.h>
#elif defined(__GNUC__)
#include <cpuid.h>
#endif