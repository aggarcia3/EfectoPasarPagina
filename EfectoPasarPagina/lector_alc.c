#include "lector_alc.h"
#include "controlador_especifico.h"
#include "controlador_efecto.h"
#include "efecto.h"

// --------------------------------------------------------------------- //
// Interpreta los argumentos de la línea de comandos pasados al programa //
// --------------------------------------------------------------------- //

#ifdef SUBSISTEMA_GRAFICO_WIN32

resOperacionEfecto ProcesarALC(LPTSTR args, opcionesLC* opciones)
{
    resOperacionEfecto toret = CORRECTA;

    int argc;
#ifdef UNICODE
    LPTSTR* argv = CommandLineToArgvW(args, &argc);
#else
#error "Se requiere soporte Unicode para compilar esta aplicación."
#endif

    opciones->ocultarGUI = FALSE;
    if (argv != NULL && argc > 1)
    {
        int ultimaOpcion = -1;
        BOOL bEncontradoParam = FALSE;
        BOOL bEstablecerShowoff = FALSE;

        for (int i = 0; i <= 1; ++i)
        {
            if (_tcsicmp(argv[i], TEXT("--nogui")) == 0)
            {
                opciones->ocultarGUI = TRUE;
                ultimaOpcion = i;
            }
            else if (_tcsicmp(argv[i], TEXT("--showoff")) == 0)
            {
                bEstablecerShowoff = TRUE;
                ultimaOpcion = i;
            }
            else
            {
                bEncontradoParam = TRUE;
            }

            if (ultimaOpcion == i && bEncontradoParam)
            {
                toret = ERR_ARGS_LINEA_COMANDOS;
                break;
            }
        }

        size_t nImgs = ultimaOpcion >= 0 ? argc - ultimaOpcion - 1 : argc;
        if (toret == CORRECTA && nImgs > 1)
        {
            // Cargar imágenes especificadas
            toret = PrepararImgsLC(&argv[ultimaOpcion + 1], nImgs);

            // Con todo cargado, podemos establecer si ocultar el resultado
            if (toret == CORRECTA && bEstablecerShowoff)
            {
                param_efecto* paramsEfecto = GetConfiguracionEfecto();
                if (paramsEfecto)
                {
                    paramsEfecto->ocultarResultado = (uint8_t) TRUE;
                }
            }
        }
    }

    LocalFree(argv);

    return toret;
}

#else

resOperacionEfecto ProcesarALC(int argc, char** argv)
{
    resOperacionEfecto toret = CORRECTA;

    if (argc > 2)
    {
        int ultimaOpcion = 0;
        int bEstablecerShowoff = 0;

        if (strcmp(argv[1], "--showoff") == 0)
        {
            bEstablecerShowoff = 1;
            ultimaOpcion = 1;
        }

        int nImgs = ultimaOpcion > 0 ? argc - ultimaOpcion - 1 : argc - 1;
        if (nImgs > 1)
        {
            // Cargar imágenes especificadas
            toret = PrepararImgsLC(&argv[ultimaOpcion + 1], nImgs);

            // Con todo cargado, podemos establecer si ocultar el resultado
            if (toret == CORRECTA && bEstablecerShowoff)
            {
                param_efecto* paramsEfecto = GetConfiguracionEfecto();
                if (paramsEfecto)
                {
                    paramsEfecto->ocultarResultado = (uint8_t) 1;
                }
            }
        }
    }
    else
    {
        toret = ERR_ARGS_LINEA_COMANDOS;
    }

    return toret;
}

#endif