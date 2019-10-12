#include "controlador_especifico.h"
#include "controlador_efecto.h"
#include "lista_enlazada.h"
#include "informacion_cpu.h"
#include "resource.h"

#ifdef SUBSISTEMA_GRAFICO_WIN32

// ------------------------------------------------------------------------------------- //
// Controla la configuración del efecto bajo el contexto de una aplicación gráfica Win32 //
// ------------------------------------------------------------------------------------- //

#define MAX_NOMBRE_FICH 2048

// Carga las imágenes contenidas en los ficheros de la cadena especificada,
// establecida por GetOpenFileName. Asume las flags OFN_ALLOWMULTISELECT | OFN_EXPLORER.
static resOperacionEfecto LeerImagenesOFN(LPCTSTR szFicheros, HWND hwndDlg);

resOperacionEfecto PrepararImgs(HWND hwndDlg)
{
    resOperacionEfecto toret = CORRECTA;

    TCHAR szFicheros[MAX_NOMBRE_FICH];
    OPENFILENAME ofn = { 0 };
    ofn.lStructSize = sizeof(ofn);
    // Formatos soportados extraídos de https://docs.opencv.org/3.0-beta/modules/imgcodecs/doc/reading_and_writing_images.html
    ofn.lpstrFilter = TEXT("Imágenes\0*.bmp;*.dib;*.jpeg;*.jpg;*.jpe;*.jp2;*.png;*.webp;*.pbm;*.pgm;*.ppm;*.sr;*.ras;*.tiff;*.tif\0\0");
    ofn.lpstrFile = szFicheros;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFicheros);
    ofn.lpstrTitle = TEXT("Abrir imágenes");
    ofn.Flags = OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_FILEMUSTEXIST;
    ofn.lpstrDefExt = TEXT("jpg");
    
    // Pedirle al usuario qué imágenes usar
    if (GetOpenFileName(&ofn))
    {
        DescargarImgs();
        toret = LeerImagenesOFN(ofn.lpstrFile, hwndDlg);
        SetDlgItemText(hwndDlg, IDC_ESTADO, mensajesError[toret]);
        EnableWindow(GetDlgItem(hwndDlg, IDC_BOTON_EMPEZAR), toret == CORRECTA);
        EnableWindow(GetDlgItem(hwndDlg, IDC_OPC), toret == CORRECTA);
    }

    return toret;
}

resOperacionEfecto PrepararImgsLC(LPCTSTR* szNombreFichero, size_t nFicheros)
{
    resOperacionEfecto toret = CORRECTA;

    if (nFicheros > 0)
    {
        stImgsCargadas.pArrImgs = (IplImage**) malloc(nFicheros * sizeof(IplImage*));
        if (stImgsCargadas.pArrImgs)
        {
            stImgsCargadas.nMaxImgs = nFicheros;
            for (size_t i = 0; i < nFicheros && toret == CORRECTA; ++i)
            {
                toret = CargarImg(szNombreFichero[i]);
            }

            // Descargar imágenes si algo fue mal, y adecuarlas si todo fue bien
            if (toret != CORRECTA)
            {
                DescargarImgs();
            }
            else
            {
                AdecuarImgs();
            }
        }
        else
        {
            toret = ERR_MEM;
        }
    }

    return toret;
}

static resOperacionEfecto LeerImagenesOFN(LPCTSTR szFicheros, HWND hwndDlg)
{
    // Formato de szFicheros:
    // - Si el usuario escoge un fichero: carpeta\0fichero\0\0
    // - Si el usuario escoge varios ficheros: carpeta\0fich1\0fich2\0fichN\0\0

    resOperacionEfecto toret = CORRECTA;
    listaEnlazada* ficheros = NULL;

    if (szFicheros && (ficheros = crearLista(), ficheros != NULL))
    {
        BOOL bFinCadena = FALSE;
        BOOL bNuloAnteriorEncontrado = FALSE;
        int iUltimaPosNulo = -1;

        for (int i = 0; i < MAX_NOMBRE_FICH - 2 && !bFinCadena; i += 2)
        {
            TCHAR actual = szFicheros[i];
            TCHAR sig = szFicheros[i + 1];
            TCHAR sigSig = szFicheros[i + 2];

            BOOL bNuloEncontrado = actual == '\0' || sig == '\0';
            bFinCadena = actual == '\0' && sig == '\0' || sig == '\0' && sigSig == '\0';
            int iPosNulo = bNuloEncontrado ? (actual == '\0' ? i : i + 1) : -1;

            //if (bNuloEncontrado && !bNuloAnteriorEncontrado && bFinCadena)
            //{
                // Primer nulo encontrado.
                // Se trata de la carpeta en la que el usuario seleccionó ficheros, o el único fichero seleccionado
                // Como está en el fin de cadena, se trata de la ruta completa (carpeta + nombre de fichero) del único fichero seleccionado
                // Aunque podríamos añadirlo a la lista con anadirLista(ficheros, (void*) szFicheros), esto es inútil: la transición debe de realizarse entre dos imágenes; necesitamos que el usuario selecione 2 o más
                // Por eso simplemente no hacemos nada
            //}

            if (bNuloEncontrado && bNuloAnteriorEncontrado)
            {
                // Se encontró un nulo que no es el primero
                TCHAR* bufer = (TCHAR*) calloc(MAX_NOMBRE_FICH, sizeof(TCHAR));
                if (bufer)
                {
                    // Generar el nombre de fichero completo, concatenando la ruta con el nombre
                    _tcscat_s(bufer, MAX_NOMBRE_FICH, szFicheros);
                    _tcscat_s(bufer, MAX_NOMBRE_FICH, _TEXT("\\"));
                    _tcscat_s(bufer, MAX_NOMBRE_FICH, szFicheros + iUltimaPosNulo + 1);
                    anadirLista(ficheros, bufer);
                }
                else
                {
                    // No seguir iterando, algo fue mal reservando memoria
                    toret = ERR_MEM;
                    break;
                }
            }

            bNuloAnteriorEncontrado = bNuloAnteriorEncontrado || bNuloEncontrado;
            iUltimaPosNulo = iPosNulo > 0 ? iPosNulo : iUltimaPosNulo;
        }
    }

    // Generar array definitivo con imágenes a partir de lista de ficheros
    if (ficheros->nElementos > 1)
    {
        IplImage** arrImgs;
        listaEnlazadaIter* it;
        TCHAR* buferActual;
        if ((arrImgs = (IplImage**) malloc(ficheros->nElementos * sizeof(IplImage*)), arrImgs) && (it = iterLista(ficheros), it))
        {
            // Descargar imágenes actuales de memoria, si hay
            DescargarImgs();

            stImgsCargadas.pArrImgs = arrImgs;
            stImgsCargadas.nMaxImgs = ficheros->nElementos;
            while ((buferActual = (TCHAR*) iterSig(it), buferActual) && toret == CORRECTA)
            {
                // Cargar todas las imágenes en el array
                toret = CargarImg(buferActual);
            }

            // Descargar imágenes si algo fue mal, y adecuarlas si todo fue bien
            if (toret != CORRECTA)
            {
                DescargarImgs();
            }
            else
            {
                AdecuarImgs();
            }

            finalizarIter(it);
        }
        else
        {
            toret = ERR_MEM;
        }
    }
    else
    {
        toret = ERR_IMGS_INSUFICIENTES;
    }

    // Liberar memoria temporal que hemos reservado
    if (ficheros != NULL)
    {
        eliminarLista(ficheros, &free);
    }

    return toret;
}

void MostrarConfig(HWND* hwndDlg, param_efecto* paramsEfecto)
{
    // Mostrar número de hilos
    TCHAR buf[11];
    _sntprintf_s(buf, 10, _TRUNCATE, _TEXT("%d"), paramsEfecto->nHilos);
    SetWindowText(GetDlgItem(*hwndDlg, IDC_N_NUCLEOS), buf);
    SendMessage(GetDlgItem(*hwndDlg, IDC_N_NUCLEOS), EM_SETLIMITTEXT, 10, (LPARAM) NULL);

    // Configurar rango de número de hilos
    SendMessage(GetDlgItem(*hwndDlg, IDC_INCR_N_NUCLEOS), UDM_SETRANGE, 0, MAKELPARAM(SHRT_MAX, 1));

    // Establecer rango de la barra deslizante de posición del eje
    HWND hwndBarraDeslizantePosEje = GetDlgItem(*hwndDlg, IDC_POS_EJE);
    SendMessage(hwndBarraDeslizantePosEje, TBM_SETRANGE, TRUE, MAKELPARAM(0, 100));    // Saltos de 0.01
    for (short pos = 25; pos < 100; pos += 25)
    {
        // Ticks de 0.25 en 0.25
        SendMessage(hwndBarraDeslizantePosEje, TBM_SETTIC, 0, pos);
    }
    SendMessage(hwndBarraDeslizantePosEje, TBM_SETPOS, TRUE, paramsEfecto->xEje * 100 / stImgsCargadas.pArrImgs[0]->width);

    // Establecer si la sombra fue activada o no
    SendMessage(GetDlgItem(*hwndDlg, IDC_ACTIVAR_SOMBRA), BM_SETCHECK, paramsEfecto->activarSombra ? BST_CHECKED : BST_UNCHECKED, (LPARAM) NULL);

    // Establecer rango de la barra deslizante de tamaño de sombra
    HWND hwndBarraDeslizanteTamSombra = GetDlgItem(*hwndDlg, IDC_ANCHO_SOMBRA);
    SendMessage(hwndBarraDeslizanteTamSombra, TBM_SETRANGE, TRUE, MAKELPARAM(1, 50));
    for (short pos = 10; pos < 50; pos += 10)
    {
        // Ticks de 10 en 10 píxeles
        SendMessage(hwndBarraDeslizanteTamSombra, TBM_SETTIC, 0, pos);
    }
    SendMessage(hwndBarraDeslizanteTamSombra, TBM_SETPOS, TRUE, paramsEfecto->anchoSombra);

    // Establecer rango de la barra deslizante del paso de giro
    HWND hwndBarraDeslizantePasoGiro = GetDlgItem(*hwndDlg, IDC_PASO_GIRO);
    SendMessage(hwndBarraDeslizantePasoGiro, TBM_SETRANGE, TRUE, MAKELPARAM(1, 90));
    for (short pos = 5; pos < 90; pos += 5)
    {
        // Ticks de 15 en 15 grados
        SendMessage(hwndBarraDeslizantePasoGiro, TBM_SETTIC, 0, pos);
    }
    SendMessage(hwndBarraDeslizantePasoGiro, TBM_SETPOS, TRUE, paramsEfecto->pasoGiro);

    // Establecer si SSE fue activado o no
    SendMessage(GetDlgItem(*hwndDlg, IDC_SSE), BM_SETCHECK, paramsEfecto->usarSSE ? BST_CHECKED : BST_UNCHECKED, (LPARAM) NULL);
    EnableWindow(GetDlgItem(*hwndDlg, IDC_SSE), ProcesadorSoportaSSE2());

    // Establecer si el bucle fue activado o no
    SendMessage(GetDlgItem(*hwndDlg, IDC_BUCLE), BM_SETCHECK, paramsEfecto->bucle ? BST_CHECKED : BST_UNCHECKED, (LPARAM) NULL);

    // Establecer si las estadísticas de rendimiento fueron activadas o no
    SendMessage(GetDlgItem(*hwndDlg, IDC_ESTADISTICAS), BM_SETCHECK, paramsEfecto->estRendimiento ? BST_CHECKED : BST_UNCHECKED, (LPARAM) NULL);

    // Establecer si la imagen resultado se muestra o no
    SendMessage(GetDlgItem(*hwndDlg, IDC_OCULTAR_RES), BM_SETCHECK, paramsEfecto->ocultarResultado ? BST_CHECKED : BST_UNCHECKED, (LPARAM) NULL);

    // Mostrar tiempo de espera entre imágenes
    _sntprintf_s(buf, 10, _TRUNCATE, _TEXT("%d"), paramsEfecto->tiempoEspera);
    SetWindowText(GetDlgItem(*hwndDlg, IDC_MS_ESPERA), buf);
    SendMessage(GetDlgItem(*hwndDlg, IDC_MS_ESPERA), EM_SETLIMITTEXT, 10, (LPARAM) NULL);
}

void ValidarValor(HWND* hwndDlg, WORD control)
{
    switch (control)
    {
        case IDC_N_NUCLEOS:
        {
            TCHAR buf[11];
            HWND hwndNNucleos = GetDlgItem(*hwndDlg, IDC_N_NUCLEOS);
            unsigned int nucleos;

            SendMessage(hwndNNucleos, WM_GETTEXT, 11, (LPARAM) buf);
            nucleos = (unsigned int) _tstoi(buf);

            if (nucleos < 1)
            {
                // Mínimo de un núcleo de CPU
                SetWindowText(hwndNNucleos, TEXT("1"));
            }

            if (nucleos > GetNumeroProcesadores())
            {
                EDITBALLOONTIP avisoDemasiadosNucleos;
                avisoDemasiadosNucleos.cbStruct = sizeof(EDITBALLOONTIP);
                avisoDemasiadosNucleos.pszTitle = L"Número alto de hilos";
                avisoDemasiadosNucleos.pszText = L"El número de hilos introducido es mayor que el número de hilos de CPU disponibles en el sistema, lo cual afectará negativamente al rendimiento del efecto.";
                avisoDemasiadosNucleos.ttiIcon = TTI_WARNING;

                SendMessage(hwndNNucleos, EM_SHOWBALLOONTIP, 0, (LPARAM) &avisoDemasiadosNucleos);
            }

            break;
        }
    }
}

void AplicarConfig(HWND* hwndDlg)
{
    TCHAR buf[11];
    param_efecto* paramsEfecto = GetConfiguracionEfecto();

    // Establecer número de hilos
    SendMessage(GetDlgItem(*hwndDlg, IDC_N_NUCLEOS), WM_GETTEXT, 11, (LPARAM) buf);
    paramsEfecto->nHilos = (unsigned int) _tstoi(buf);

    // Establecer posición del eje
    paramsEfecto->xEje = (int) (SendMessage(GetDlgItem(*hwndDlg, IDC_POS_EJE), TBM_GETPOS, 0, 0) * stImgsCargadas.pArrImgs[0]->width / 100);

    // Establecer activación de la sombra
    paramsEfecto->activarSombra = SendMessage(GetDlgItem(*hwndDlg, IDC_ACTIVAR_SOMBRA), BM_GETCHECK, 0, 0) == BST_CHECKED;

    // Establecer ancho de la sombra
    unsigned short anchoSombraBruto = (unsigned short) SendMessage(GetDlgItem(*hwndDlg, IDC_ANCHO_SOMBRA), TBM_GETPOS, 0, 0);
    paramsEfecto->anchoSombra = (unsigned short) (paramsEfecto->xEje >= anchoSombraBruto && paramsEfecto->xEje + anchoSombraBruto < stImgsCargadas.pArrImgs[0]->width ? anchoSombraBruto : paramsEfecto->xEje);

    // Establecer paso de giro
    paramsEfecto->pasoGiro = (uint8_t) SendMessage(GetDlgItem(*hwndDlg, IDC_PASO_GIRO), TBM_GETPOS, 0, 0);

    // Establecer si usar SSE
    paramsEfecto->usarSSE = SendMessage(GetDlgItem(*hwndDlg, IDC_SSE), BM_GETCHECK, 0, 0) == BST_CHECKED;

    // Establecer si activar bucle
    paramsEfecto->bucle = SendMessage(GetDlgItem(*hwndDlg, IDC_BUCLE), BM_GETCHECK, 0, 0) == BST_CHECKED;

    // Establecer si mostrar estadísticas de rendimiento
    paramsEfecto->estRendimiento = SendMessage(GetDlgItem(*hwndDlg, IDC_ESTADISTICAS), BM_GETCHECK, 0, 0) == BST_CHECKED;

    // Establecer si mostrar imagen resultado o no
    paramsEfecto->ocultarResultado = SendMessage(GetDlgItem(*hwndDlg, IDC_OCULTAR_RES), BM_GETCHECK, 0, 0) == BST_CHECKED;

    // Establecer tiempo de espera entre imágenes
    SendMessage(GetDlgItem(*hwndDlg, IDC_MS_ESPERA), WM_GETTEXT, 11, (LPARAM) buf);
    paramsEfecto->tiempoEspera = _tstoi(buf);
}

#endif