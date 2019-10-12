#include "controlador_efecto.h"
#include "controlador_especifico.h"
#include "informacion_cpu.h"
#include "lector_alc.h"

// ---------------------------------------------------------------------------------- //
// Controla la configuración del efecto bajo el contexto de una aplicación de consola //
// ---------------------------------------------------------------------------------- //

#ifndef SUBSISTEMA_GRAFICO_WIN32

// Pide al usuario parámetros de configuración del efecto, si desea proporcionarlos.
static void ConfigurarEfecto();

int main(int argc, char** argv)
{
    resOperacionEfecto toret;

    // Forzar página de caracteres ANSI Windows-1252 para salida de caracteres si es posible
    // (Debe de coincidir con la página de caracteres ANSI en la que se hayan codificado las cadenas en los archivos de código fuente)
    if (IsValidCodePage(1252))
    {
        SetConsoleCP(1252);
        SetConsoleOutputCP(1252);
    }

    // Título de la ventana
    SetConsoleTitleA("Efecto de pasar página");

    toret = ProcesarALC(argc, argv);

    // Mostrar mensaje de estado
    fprintf(stderr, "%s\n\n", mensajesError[toret]);
    if (toret == ERR_ARGS_LINEA_COMANDOS)
    {
        fprintf(stderr, "Sintaxis: %s [--showoff] (imagen 1) (imagen 2) (... [imagen %d])\n", argv[0], INT_MAX - 1);
    }

    if (toret == CORRECTA)
    {
        ConfigurarEfecto();
        IniciarEfecto();
    }

    return toret;
}

resOperacionEfecto PrepararImgsLC(char** imgs, int nImgs)
{
    resOperacionEfecto toret = CORRECTA;

    IplImage** arrImgs;
    if ((arrImgs = malloc(nImgs * sizeof(IplImage*)), arrImgs))
    {
        stImgsCargadas.pArrImgs = arrImgs;
        stImgsCargadas.nMaxImgs = nImgs;
        for (int i = 0; i < nImgs && toret == CORRECTA; ++i)
        {
            toret = CargarImg(imgs[i]);
        }

        if (toret == CORRECTA)
        {
            AdecuarImgs();
        }
    }
    else
    {
        toret = ERR_MEM;
    }

    return toret;
}

static void ConfigurarEfecto()
{
    param_efecto* paramEfecto = GetConfiguracionEfecto();
    char buf[TAM_BUF];

    if (paramEfecto == NULL)
    {
        fputs("No se ha podido recuperar la configuración predeterminada del efecto", stderr);
        exit(ERR_MEM);
    }
    else
    {
        puts("-- OPCIONES DEL EFECTO --");

        // ¿Quiere el usuario modificar los parámetros de ejecución?
        fputs("¿Modificar configuración predeterminada del efecto? (S/N): ", stdout);
        fgets(buf, TAM_BUF - 1, stdin);
        if (buf[0] == 'S' || buf[0] == 's')
        {
            // Hilos
            do
            {
                printf("Cantidad de hilos (1-%u; tope recomendado %d): ", UINT_MAX, GetNumeroProcesadores());
                fgets(buf, TAM_BUF - 1, stdin);
                unsigned long n = strtoul(buf, NULL, 0);
                paramEfecto->nHilos = (unsigned int) (n > UINT_MAX ? UINT_MAX : n);
            } while (paramEfecto->nHilos < 1);

            // Posición del eje
            do
            {
                fputs("Posición relativa del eje (0 - totalmente a la izquierda, 1 - totalmente a la derecha): ", stdout);
                fgets(buf, TAM_BUF - 1, stdin);
                paramEfecto->xEje = (int)(strtod(buf, NULL) * stImgsCargadas.pArrImgs[0]->width);
            } while (paramEfecto->xEje < 0 || paramEfecto->xEje > stImgsCargadas.pArrImgs[0]->width);

            // Sombreado
            fputs("¿Aplicar efecto de sombreado? (S/N): ", stdout);
            fgets(buf, TAM_BUF - 1, stdin);
            paramEfecto->activarSombra = buf[0] == 'S' || buf[0] == 's';

            // Ancho de la sombra, si aplicable
            if (paramEfecto->activarSombra)
            {
                do
                {
                    fputs("Ancho de la sombra (en píxeles, máximo 50): ", stdout);
                    fgets(buf, TAM_BUF - 1, stdin);
                    paramEfecto->anchoSombra = (unsigned short) atoi(buf);
                } while (paramEfecto->xEje + paramEfecto->anchoSombra / 2 >= stImgsCargadas.pArrImgs[0]->width || paramEfecto->xEje - paramEfecto->anchoSombra / 2 < 0 || paramEfecto->anchoSombra > 50);
            }

            // Paso de giro
            do
            {
                fputs("Paso de giro (1-90 grados): ", stdout);
                fgets(buf, TAM_BUF - 1, stdin);
                paramEfecto->pasoGiro = (unsigned char) atoi(buf);
            } while (paramEfecto->pasoGiro <= 0 || paramEfecto->pasoGiro > 90);

            // Uso de SSE
            if (ProcesadorSoportaSSE2())
            {
                fputs("Usar rutinas SIMD (S/N): ", stdout);
                fgets(buf, TAM_BUF - 1, stdin);
                paramEfecto->usarSSE = buf[0] == 'S' || buf[0] == 's';
            }

            // Bucle
            fputs("Reproducir efecto en bucle (S/N): ", stdout);
            fgets(buf, TAM_BUF - 1, stdin);
            paramEfecto->bucle = buf[0] == 'S' || buf[0] == 's';

            // Estadísticas de rendimiento
            fputs("Mostrar estadísticas de rendimiento al final (S/N): ", stdout);
            fgets(buf, TAM_BUF - 1, stdin);
            paramEfecto->estRendimiento = buf[0] == 'S' || buf[0] == 's';

            // Tiempo de espera entre imágenes
            fputs("Milisegundos de espera entre imágenes (<= 0: hasta que el usuario presione una tecla, > 0: los milisegundos especificados): ", stdout);
            fgets(buf, TAM_BUF - 1, stdin);
            paramEfecto->tiempoEspera = atoi(buf);
        }
    }
}

#endif