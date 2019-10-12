/* stdafx.h: archivo de inclusión de los archivos de inclusión estándar del sistema
   o archivos de inclusión específicos de un proyecto utilizados frecuentemente,
   pero rara vez modificados
*/

#pragma once

// Si se define la constante que viene a continuación, esta aplicación se compilará como una aplicación
// gráfica Win32. En caso contrario, se compila utilizando solamente funciones disponibles en las librerías
// estándar de C, OpenCV y Cygwin.

// La aplicación gráfica Win32 no está diseñada para ser compilada en Netbeans, aunque debería de ser posible
// adaptarla con el suficiente empeño.

// Tanto la aplicación gráfica como la de consola deben de ser ejecutadas en un SO Windows.

// Si se está usando Microsoft Visual Studio, es posible que haya que cambiar las opciones del vinculador para indicarle
// que use el subsistema de consola (si no, esperará encontrar un punto de entrada inexistente para la aplicación).

#define SUBSISTEMA_GRAFICO_WIN32

// Marca la aplicación como una compilación de depuración, activando características de depuración.
//#define DEPURACION

#ifdef SUBSISTEMA_GRAFICO_WIN32
// Compilar para Windows 7 y posteriores SO
#define _WIN32_WINNT 0x0601

#include <winsdkver.h>
#include <SDKDDKVer.h>

// Excluir de la compilación cabeceras que no vamos a usar para mayor velocidad
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