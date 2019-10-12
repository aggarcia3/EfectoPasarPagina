#include "controlador_efecto.h"
#include "estrategias_efecto.h"
#include "efecto.h"
#include "ficheros_embebidos.h"
#include "informacion_cpu.h"
#include "resource.h"

// ------------------------------------------------------------------------------------------------ //
// Pone en marcha el efecto y lo controla directamente, manteniendo el diálogo con la API de OpenCV //
// ------------------------------------------------------------------------------------------------ //

// Definir algunas variables declaradas una y solo una vez

imagenesCargadas stImgsCargadas = { 0 };
#ifdef SUBSISTEMA_GRAFICO_WIN32
const TCHAR* mensajesError[] = {
#else
const char* mensajesError[] = {
#endif
    TEXT("Se han cargado todas las imágenes con éxito."),
    TEXT("No se pudo reservar la memoria necesaria para las operaciones solicitadas."),
    TEXT("Los argumentos de la línea de comandos no son válidos."),
    TEXT("No se pudo cargar alguna de las imágenes. ¿Son todas válidas?"),
    TEXT("Las imágenes no cumplen las propiedades requeridas."),
    TEXT("No se han podido crear hilos de ejecución para el efecto."),
    TEXT("No se seleccionaron imágenes suficientes para cargar.")
};

// Definir y declarar funciones de ámbito local a este fichero

static void LanzarTrabajoParalelo(trabajoParalelo t);
void* Hilo(void* args);

static void (*mostrarRes) ();

static void MostrarResVentanaOpenCV();
static void NoMostrarRes();

static param_hilo* paramsHilo;

param_efecto* GetConfiguracionEfecto()
{
    static param_efecto* toret = NULL;

    if (!toret)
    {
        toret = (param_efecto*) calloc(1, sizeof(param_efecto));
    }

    if (toret && toret->nHilos == 0)
    {
        toret->usarSSE = 0;
        toret->bucle = 0;
        toret->estRendimiento = 1;
        toret->tiempoEspera = 2500;
        toret->nHilos = GetNumeroProcesadores();
        toret->xEje = stImgsCargadas.pArrImgs[0]->width >> 1;
        toret->activarSombra = 1;
        toret->anchoSombra = toret->xEje >= 20 && toret->xEje + 20 < stImgsCargadas.pArrImgs[0]->width ? 20 : toret->xEje;
        toret->pasoGiro = 1;
    }

    return toret;
}

void IniciarEfecto()
{
    param_efecto* paramsEfecto = GetConfiguracionEfecto();
    IplImage* primeraImg = stImgsCargadas.pArrImgs[0];
    size_t n = stImgsCargadas.nImgs;

    // Comprobar que se pudo reservar memoria para los parámetros del efecto
    if (!paramsEfecto)
    {
#ifdef SUBSISTEMA_GRAFICO_WIN32
        MessageBeep(MB_ICONERROR);
        MessageBox(NULL, mensajesError[ERR_MEM], TEXT("Efecto de pasar página"), MB_OK | MB_ICONERROR);
#else
        exit(ERR_MEM);
#endif
        return;
    }

    // Crear array con parámetros de hilos
    paramsHilo = (param_hilo*) calloc(paramsEfecto->nHilos, sizeof(param_hilo));
    if (paramsHilo)
    {
        // Asignar valores de configuración constantes a cada hilo
        for (unsigned long i = 0; i < paramsEfecto->nHilos; ++i)
        {
            paramsHilo[i].idHilo = i;
            paramsHilo[i].paramsEfecto = paramsEfecto;
        }
    }
    else
    {
#ifdef SUBSISTEMA_GRAFICO_WIN32
        MessageBeep(MB_ICONERROR);
        MessageBox(NULL, mensajesError[ERR_MEM], TEXT("Efecto de pasar página"), MB_OK | MB_ICONERROR);
#else
        exit(ERR_MEM);
#endif
        return;
    }

    // Crear imagen resultado
    paramsEfecto->res = cvCreateImage(cvSize(primeraImg->width, primeraImg->height), primeraImg->depth, primeraImg->nChannels);
    if (!paramsEfecto->res)
    {
#ifdef SUBSISTEMA_GRAFICO_WIN32
        MessageBeep(MB_ICONERROR);
        MessageBox(NULL, mensajesError[ERR_MEM], TEXT("Efecto de pasar página"), MB_OK | MB_ICONERROR);
#else
        exit(ERR_MEM);
#endif
        return;
    }

    // Crear ventana donde mostrar los resultados
    if (!paramsEfecto->ocultarResultado)
    {
#ifdef SUBSISTEMA_GRAFICO_WIN32
        RECT escritorio;
        HWND hwndVentanaOpenCV;

        // Obtener la resolución del escritorio, para saber cómo dimensionar mejor la ventana
        GetWindowRect(GetDesktopWindow(), &escritorio);

        // Autodimensionar la ventana según lo aprendido
        cvNamedWindow(NOMBRE_VENTANA, paramsEfecto->res->width > escritorio.right || paramsEfecto->res->height > escritorio.bottom ? CV_WINDOW_NORMAL : CV_WINDOW_AUTOSIZE);

        // Obtener referencia a la ventana creada
        hwndVentanaOpenCV = cvGetWindowHandle(NOMBRE_VENTANA);

        // Cambiar cursor del ratón al normal
        SetClassLongPtr(hwndVentanaOpenCV, GCLP_HCURSOR, (LONG_PTR) LoadCursor(NULL, IDC_ARROW));
#else
        cvNamedWindow(NOMBRE_VENTANA, CV_WINDOW_AUTOSIZE);
#endif
    }

    trabajoParalelo procCopiaInicial;
    trabajoParalelo procEliminarArtefactosPasoGiro;
    trabajoParalelo procPrimeraMitad;
	trabajoParalelo procSegundaMitad;
    if (paramsEfecto->usarSSE)
    {
        prepararSSE(paramsEfecto);
        procCopiaInicial = &copiaInicialSSE;
        procPrimeraMitad = &primeraMitadSSE;
        procEliminarArtefactosPasoGiro = &eliminarArtefactosPasoGiroSSE;
        procSegundaMitad = &segundaMitadSSE;
    }
    else
    {
        preparar(paramsEfecto);
        procCopiaInicial = &copiaInicial;
        procPrimeraMitad = &primeraMitad;
        procEliminarArtefactosPasoGiro = &eliminarArtefactosPasoGiro;
        procSegundaMitad = &segundaMitad;
    }

    if (paramsEfecto->ocultarResultado)
    {
        mostrarRes = &NoMostrarRes;
    }
    else
    {
        mostrarRes = &MostrarResVentanaOpenCV;
    }

    clock_t inicio = clock();

    uint8_t omega = paramsEfecto->pasoGiro;
    for (size_t i = 0; paramsEfecto->bucle || i < n - 1; omega = paramsEfecto->pasoGiro, i = (i + 1) % n)
    {
#ifdef SUBSISTEMA_GRAFICO_WIN32
        // Reproducir efecto de sonido de paso de página
        if (!paramsEfecto->ocultarResultado)
        {
            PlaySound((LPCTSTR) pasar_pagina_wav, NULL, SND_MEMORY | SND_NODEFAULT | SND_ASYNC);
        }
#endif

        // Actualizar referencias a imágenes
        paramsEfecto->actual = stImgsCargadas.pArrImgs[i];
        paramsEfecto->sig = stImgsCargadas.pArrImgs[(i + 1) % n];

        // Poner hilos en marcha para la copia inicial
        // (Hace falta tener la imagen copiada en res, no podemos mostrar la imagen actual directamente)
        LanzarTrabajoParalelo(procCopiaInicial);

        // Primeros 90º. Giro de la mitad derecha de la imagen actual
        for (paramsEfecto->omega = omega; omega <= 90; omega += paramsEfecto->pasoGiro, paramsEfecto->omega = omega)
        {
            LanzarTrabajoParalelo(procPrimeraMitad);

#ifdef DEPURACION
#ifdef _MSC_VER
            TCHAR pszMsg[32];
            _stprintf_s(pszMsg, 32, TEXT("Omega actual (prim. mitad): %d\n"), (int) omega);
            OutputDebugString(pszMsg);
#else
            printf("Omega actual (seg. mitad): %d\n", (int) omega);
#endif
            cvWaitKey(0);
#endif
        }
        
        // Eliminar artefactos visuales posibles tras la primera parte
        if (omega - paramsEfecto->pasoGiro != 90)
        {
            paramsEfecto->omega = omega - paramsEfecto->pasoGiro;   // Último omega procesado por la primera parte
            LanzarTrabajoParalelo(procEliminarArtefactosPasoGiro);
        }

        // Siguientes 90º. Giro de la mitad izquierda de la imagen siguiente
        for (paramsEfecto->omega = omega; omega < 180; omega += paramsEfecto->pasoGiro, paramsEfecto->omega = omega)
        {
            LanzarTrabajoParalelo(procSegundaMitad);

#ifdef DEPURACION
#ifdef _MSC_VER
			TCHAR pszMsg[32];
			_stprintf_s(pszMsg, 32, TEXT("Omega actual (seg. mitad): %d\n"), (int) omega);
			OutputDebugString(pszMsg);
#else
            printf("Omega actual (seg. mitad): %d\n", (int) omega);
#endif
            cvWaitKey(0);
#endif
        }

        // Mostrar segunda imagen tal cual al llegar a 180º como optimización
        if (!paramsEfecto->ocultarResultado)
        {
            cvShowImage(NOMBRE_VENTANA, paramsEfecto->sig);
        }

        if (!paramsEfecto->ocultarResultado && (paramsEfecto->bucle || i < n - 2))
        {
            cvWaitKey(paramsEfecto->tiempoEspera);
        }
    }

    clock_t fin = clock();

    DescargarImgs();
    free(paramsHilo);
    free(paramsEfecto);

    // Si aplicable, mostrar información de rendimiento
    if (paramsEfecto->estRendimiento)
    {
        double tiempoTotal = ((double) (fin - inicio)) / CLOCKS_PER_SEC;
#ifdef SUBSISTEMA_GRAFICO_WIN32
        TCHAR pszMsg[128];
        _stprintf_s(pszMsg, 128, TEXT("-- ESTADÍSTICAS DE RENDIMIENTO %s--\n\nTiempo de CPU usado: %.3f s"), (paramsEfecto->ocultarResultado ? TEXT("") : TEXT("(NO FIABLES) ")), tiempoTotal);
        MessageBox(NULL, pszMsg, TEXT("Estadísticas de rendimiento"), MB_OK | MB_ICONASTERISK);
#else
        printf("\n-- ESTADÍSTICAS DE RENDIMIENTO --\nTiempo de CPU usado: %.3f s\n\nCierra cualquier ventana o presiona una tecla para salir.", tiempoTotal);
#endif
    }

    if (!paramsEfecto->ocultarResultado)
    {
        cvWaitKey(0);
    }

    cvDestroyAllWindows();
}

static void LanzarTrabajoParalelo(trabajoParalelo t)
{
    param_efecto* paramsEfecto = GetConfiguracionEfecto();

    // Poner hilos en marcha para el trabajo parámetro
    for (unsigned long i = 0; i < paramsEfecto->nHilos; ++i)
    {
        paramsHilo[i].trabajo = t;
        if (pthread_create(&paramsHilo[i].esteHilo, NULL, &Hilo, &paramsHilo[i]))
        {
#ifdef SUBSISTEMA_GRAFICO_WIN32
            MessageBeep(MB_ICONERROR);
            MessageBox(NULL, mensajesError[ERR_CREACION_HILO], TEXT("Efecto de pasar página"), MB_OK | MB_ICONERROR);        
#endif
            exit(ERR_CREACION_HILO);
        }
    }

    // Esperar a que todos los hilos finalicen
    for (unsigned long i = 0; i < paramsEfecto->nHilos; ++i)
    {
        pthread_join(paramsHilo[i].esteHilo, NULL);
    }

    // Mostrar el resultado si aplicable
    (*mostrarRes)(paramsEfecto);
}

void* Hilo(void* arg)
{
    param_hilo* p = (param_hilo*) arg;

    // Hacer aquello para lo que nos han creado
    (*p->trabajo)(p);

    return NULL;
}

static void MostrarResVentanaOpenCV()
{
    cvShowImage(NOMBRE_VENTANA, GetConfiguracionEfecto()->res);
    cvWaitKey(1);
}

static void NoMostrarRes()
{
    return;
}

#ifdef SUBSISTEMA_GRAFICO_WIN32
resOperacionEfecto CargarImg(LPCTSTR szRuta)
#else
resOperacionEfecto CargarImg(const char* ruta)
#endif
{
    resOperacionEfecto toret = CORRECTA;

#if defined(SUBSISTEMA_GRAFICO_WIN32) && defined(UNICODE)
    size_t tamRuta = wcslen(szRuta) * sizeof(wchar_t);
    char* buf = malloc(tamRuta);
    IplImage* actual;

    if (buf)
    {
        // Convertir cadena Unicode (caracteres anchos) a char* (multibyte)
        wcstombs_s(NULL, buf, tamRuta, szRuta, _TRUNCATE);
        actual = cvLoadImage(buf, CV_LOAD_IMAGE_UNCHANGED);
        free(buf);

#elif !defined(SUBSISTEMA_GRAFICO_WIN32)
        IplImage* actual = cvLoadImage(ruta, CV_LOAD_IMAGE_UNCHANGED);
#else
#error "Se requiere soporte Unicode para compilar esta aplicación."
#endif

        if (actual)
        {
            size_t n = stImgsCargadas.nImgs;
            if (n > 0 && stImgsCargadas.pArrImgs[n - 1]->nChannels != actual->nChannels)
            {
                toret = ERR_PROPIEDADES_IMG;
            }
            else if (n < stImgsCargadas.nMaxImgs)
            {
                // Añadir elementos a un array estático mientras quede hueco
                stImgsCargadas.pArrImgs[stImgsCargadas.nImgs++] = actual;
            }
            else
            {
                // No podemos añadir la imagen al array. Liberar la memoria usada
                cvReleaseImage(&actual);
            }
        }
        else
        {
            toret = ERR_CARGA_IMG;
        }

#if defined(SUBSISTEMA_GRAFICO_WIN32) && defined(UNICODE)
    }
#elif defined(SUBSISTEMA_GRAFICO_WIN32)
#error "Se requiere soporte Unicode para compilar esta aplicación."
#endif

    return toret;
}

#define MENOR_NO_ASIGNADO (size_t) -1
resOperacionEfecto AdecuarImgs()
{
    resOperacionEfecto toret = CORRECTA;
    size_t menor = MENOR_NO_ASIGNADO;

    for (size_t i = 0; i < stImgsCargadas.nImgs && toret == CORRECTA; ++i)
    {
        IplImage* actual = stImgsCargadas.pArrImgs[i];

        // Estamos ante una imagen mayor que la menor. Cambiar su tamaño
        if (menor != MENOR_NO_ASIGNADO && actual->imageSize > stImgsCargadas.pArrImgs[menor]->imageSize)
        {
            IplImage* nuevaImg = cvCreateImage(cvSize(stImgsCargadas.pArrImgs[menor]->width, stImgsCargadas.pArrImgs[menor]->height), IPL_DEPTH_8U, actual->nChannels);
            if (!nuevaImg)
            {
                toret = ERR_MEM;
            }
            else
            {
                cvResize(actual, nuevaImg, CV_INTER_LINEAR);
                cvReleaseImage(&actual);
                actual = nuevaImg;
            }
        }

        // Encontramos una nueva imagen menor
        if (menor == MENOR_NO_ASIGNADO || stImgsCargadas.pArrImgs[menor]->imageSize > actual->imageSize)
        {
            menor = i;

            // Reducir el tamaño de las anteriores
            for (size_t j = i; j > 0; --j)
            {
                IplImage* nuevaImg = cvCreateImage(cvSize(actual->width, actual->height), IPL_DEPTH_8U, actual->nChannels);
                if (!nuevaImg)
                {
                    toret = ERR_MEM;
                }
                else
                {
                    cvResize(stImgsCargadas.pArrImgs[j - 1], nuevaImg, CV_INTER_LINEAR);
                    cvReleaseImage(&stImgsCargadas.pArrImgs[j - 1]);
                    stImgsCargadas.pArrImgs[j - 1] = nuevaImg;
                }
            }
        }

        stImgsCargadas.pArrImgs[i] = actual;
    }

    // Advertir de que los algoritmos podrían hacer desbordamientos con imágenes muy grandes
    // (El máximo estándar con pleno soporte es 8K UHDV)
    if (stImgsCargadas.nImgs > 0 && stImgsCargadas.pArrImgs[0]->width > 8191)
    {
#ifdef SUBSISTEMA_GRAFICO_WIN32
        MessageBox(NULL, TEXT("La resolución de salida del efecto es de ancho mayor que 8191. No se garantiza el correcto funcionamiento del efecto con estas resoluciones."), TEXT("Efecto de pasar página"), MB_OK | MB_ICONEXCLAMATION);
#else
        printf("-- ADVERTENCIA --\nLa resolución salida del efecto es de ancho mayor que 8191. No se garantiza el correcto funcionamiento del efecto con estas resoluciones.\n\n");
#endif
    }

    return toret;
}

void DescargarImgs()
{
    if (stImgsCargadas.nImgs > 0)
    {
        for (size_t i = 0; i < stImgsCargadas.nImgs; ++i)
        {
            cvReleaseImage(&stImgsCargadas.pArrImgs[i]);
        }
        free(stImgsCargadas.pArrImgs);

        stImgsCargadas.nImgs = stImgsCargadas.nMaxImgs = 0;
    }
}