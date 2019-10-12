#include "stdafx.h"
#include "resource.h"
#include "lector_alc.h"
#include "controlador_especifico.h"
#include "controlador_efecto.h"

#ifdef SUBSISTEMA_GRAFICO_WIN32

// --------------------------------------------------------- //
// Punto de entrada de la aplicaci�n. Vista de la aplicaci�n //
// --------------------------------------------------------- //

static INT_PTR CALLBACK DialogProc(HWND, UINT, WPARAM, LPARAM);

static INT_PTR CALLBACK DialogOpcProc(HWND, UINT, WPARAM, LPARAM);

static resOperacionEfecto resCargaALC;

static HICON GetIconoAplicacion();

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    opcionesLC opcionesLineaComandos;

    // Procesar argumentos de l�nea de comandos
    resCargaALC = ProcesarALC(lpCmdLine, &opcionesLineaComandos);

    // Crear cuadro de di�logo principal
    INT_PTR iRes = 0;
    if (!opcionesLineaComandos.ocultarGUI)
    {
        iRes = DialogBox(NULL, MAKEINTRESOURCE(IDD_DIALOGO_PPAL), NULL, DialogProc);
    }
    else if (resCargaALC == CORRECTA && stImgsCargadas.nImgs > 0)
    {
        // Si cargamos im�genes y se debe de ocultar la GUI, iniciar el efecto directamente
        IniciarEfecto();
    }

    // Manejar errores
    if (iRes != EXIT_SUCCESS)
    {
        TCHAR pszMsg[128];
        _stprintf_s(pszMsg, 128, TEXT("Ha ocurrido un error no esperado durante la ejecuci�n de la aplicaci�n.\n\nC�digo de error: %d"), (int) iRes);
        MessageBeep(MB_ICONERROR);
        MessageBox(NULL, pszMsg, TEXT("Efecto de pasar p�gina"), MB_OK | MB_ICONERROR);
    }

    return (int) iRes;
}


static INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    INT_PTR toret = FALSE;

    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            // Mostrar mensaje de estado correspondiente
            if (resCargaALC == CORRECTA && stImgsCargadas.nImgs > 0)
            {
                SetDlgItemText(hwndDlg, IDC_ESTADO, TEXT("Se han cargado las im�genes especificadas en los argumentos de la l�nea de comandos."));
                EnableWindow(GetDlgItem(hwndDlg, IDC_BOTON_EMPEZAR), TRUE);
                EnableWindow(GetDlgItem(hwndDlg, IDC_OPC), TRUE);
            }

            if (resCargaALC != CORRECTA)
            {
                SetDlgItemText(hwndDlg, IDC_ESTADO, mensajesError[resCargaALC]);
            }

            // Cargar icono
            HICON hIcono = GetIconoAplicacion();
            if (hIcono)
            {
                SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM) hIcono);
                SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM) hIcono);
            }
            else
            {
                EndDialog(hwndDlg, GetLastError());
            }

            toret = TRUE;
            break;
        }

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDC_EXAMINAR:
                    PrepararImgs(hwndDlg);
                    break;
                case IDC_BOTON_EMPEZAR:
                    // No es necesario corroborar que todo est� inicializado debido a se activa y desactiva
                    // este bot�n para que ignore o haga caso a este evento seg�n corresponda
                    EndDialog(hwndDlg, 0);
                    IniciarEfecto();
                    break;
                case IDC_ACERCA_DE:
                    MessageBox(NULL,

                        L"Efecto de transici�n de pasar p�gina v1.0.0.1.\n\n"

                        L"Proyecto de Arquitecturas Paralelas, curso 2017-2018.\n\n"

                        L"La librer�a OpenCV 3.0.0 se usa bajo los t�rminos de la licencia BSD, disponible en https://opencv.org/license.html.\n\n"

                        L"Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the \"Software\"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:\n\n"

                        L"The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.\n\n"

                        L"THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.",
                    
                    TEXT("Acerca de"), MB_OK | MB_ICONASTERISK);

                    break;
                case IDC_OPC:
                    DialogBox(NULL, MAKEINTRESOURCE(IDD_DIALOGO_OPCS), hwndDlg, DialogOpcProc);
                    break;
            }

            toret = TRUE;
            break;
        }

        case WM_CLOSE:
        {
            EndDialog(hwndDlg, 0);
            toret = TRUE;
            break;
        }
    }

    return toret;
}

static INT_PTR CALLBACK DialogOpcProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    INT_PTR toret = FALSE;

    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            param_efecto* paramsEfecto = GetConfiguracionEfecto();
            if (!paramsEfecto)
            {
                MessageBox(hwndDlg, TEXT("No se pudo recuperar la configuraci�n del efecto. Si este problema persiste, prueba a liberar memoria principal e int�ntalo de nuevo."), TEXT("Opciones de efecto"), MB_OK | MB_ICONERROR);
                EndDialog(hwndDlg, 0);
            }
            else
            {
                MostrarConfig(&hwndDlg, paramsEfecto);
            }
            break;
        }

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDC_N_NUCLEOS:
                {
                    if (HIWORD(wParam) == EN_UPDATE) {
                        ValidarValor(&hwndDlg, LOWORD(wParam));
                        toret = TRUE;
                    }
                    break;
                }
                case IDC_ACEPTAR_CONF:
                {
                    // Guardar configuraci�n
                    AplicarConfig(&hwndDlg);
                }
                case IDC_CANCELAR_CONF:
                {
                    // Salir
                    EndDialog(hwndDlg, 0);
                    toret = TRUE;
                }
            }
            break;
        }

        case WM_CLOSE:
        {
            EndDialog(hwndDlg, 0);
            toret = TRUE;
            break;
        }
    }

    return toret;
}

static HICON GetIconoAplicacion()
{
    // "LoadIcon loads the icon resource only if it has not been loaded; otherwise, it retrieves a handle to the existing resource."
    // - https://msdn.microsoft.com/en-us/library/windows/desktop/ms648072(v=vs.85).aspx
    return LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICONO_PPAL));
}

#endif