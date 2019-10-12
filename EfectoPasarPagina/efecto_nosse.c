#include "estrategias_efecto.h"
#include "cosenos_precomputados.h"

static void (*computerVariablesPrimera) (param_hilo* params);
static void (*computarVariablesSegunda) (param_hilo* params);
static void (*sombreadoPrimeraMitad) (uint8_t** ccActual, uint8_t** ccRes, param_hilo* params);
static void (*primeraTransformacionColumnas) (uint8_t* ccActual, uint8_t** ccRes, param_hilo* params);
static void (*segundaTransformacionColumnas) (uint8_t* ccSiguiente, uint8_t** ccRes, param_hilo* params);
static void (*sombreadoSegundaMitad) (uint8_t* ccSiguiente, uint8_t* ccRes, param_hilo* params);

static void computarVariablesPrimeraSombra(param_hilo* params);
static void computarVariablesPrimeraNoSombra(param_hilo* params);
static void primeraMitadSombra(uint8_t** ccActual, uint8_t** ccRes, param_hilo* params);
static void primeraMitadNoSombra(uint8_t** ccActual, uint8_t** ccRes, param_hilo*);
static void primTransformacionSombra(uint8_t* ccActual, uint8_t** ccRes, param_hilo* params);
static void primTransformacionNoSombra(uint8_t* ccActual, uint8_t** ccRes, param_hilo*);
static void computarVariablesSegundaSombra(param_hilo* params);
static void computarVariablesSegundaNoSombra(param_hilo* params);
static void segTransformacionSombra(uint8_t* ccSiguiente, uint8_t** ccRes, param_hilo* params);
static void segTransformacionNoSombra(uint8_t* ccSiguiente, uint8_t** ccRes, param_hilo* params);
static void segundaMitadSombra(uint8_t* ccSiguiente, uint8_t* ccRes, param_hilo* params);
static void segundaMitadNoSombra(uint8_t* ccSiguiente, uint8_t* ccRes, param_hilo* params);

// Para más información sobre el propósito de esta estructura, ver los procedimientos
// asignados a computerVariablesPrimera y computarVariablesSegunda.
typedef struct
{
    int ce;
    int cie;
    unsigned int sumando;
    unsigned int operando;
    int xLimite;
    int xLimiteSombra;
    int xLimiteAnterior;
	int omegaAnt;
} constantesEfecto;
static constantesEfecto ctes = { 0 };

void preparar(param_efecto* params)
{
    // Inicializar punteros con la estrategia de sombreado que vamos a utilizar.
    // Así aprovechamos mejor el pipeline (saltos incondicionales)
    if (params->activarSombra && params->anchoSombra > 0)
    {
        computerVariablesPrimera = &computarVariablesPrimeraSombra;
        sombreadoPrimeraMitad = &primeraMitadSombra;
        primeraTransformacionColumnas = &primTransformacionSombra;
        computarVariablesSegunda = &computarVariablesSegundaSombra;
        segundaTransformacionColumnas = &segTransformacionSombra;
        sombreadoSegundaMitad = &segundaMitadSombra;
    }
    else
    {
        computerVariablesPrimera = &computarVariablesPrimeraNoSombra;
        sombreadoPrimeraMitad = &primeraMitadNoSombra;
        primeraTransformacionColumnas = &primTransformacionNoSombra;
        computarVariablesSegunda = &computarVariablesSegundaNoSombra;
        segundaTransformacionColumnas = &segTransformacionNoSombra;
        sombreadoSegundaMitad = &segundaMitadNoSombra;
    }
}

void copiaInicial(param_hilo* params)
{
    IplImage* actual = params->paramsEfecto->actual;
    IplImage* res = params->paramsEfecto->res;
    
    // Copiar imagen actual a resultado
    for (int i = params->idHilo; i < res->height; i += params->paramsEfecto->nHilos)
    {
        uint8_t* ccActual = actual->imageData + i * actual->widthStep;
        uint8_t* ccRes = res->imageData + i * res->widthStep;

        for (int j = 0; j < res->width; ++j)
        {
            for (int k = 0; k < res->nChannels; ++k)
            {
                *ccRes++ = *ccActual++;
            }
        }
    }
}

void primeraMitad(param_hilo* params)
{
    IplImage* actual = params->paramsEfecto->actual;
    IplImage* sig = params->paramsEfecto->sig;
    IplImage* res = params->paramsEfecto->res;

    // Computar constantes necesarias
    (*computerVariablesPrimera)(params);

    for (int y = params->idHilo; y < res->height; y += params->paramsEfecto->nHilos)
    {
        uint8_t* ccActual = actual->imageData + y * actual->widthStep + res->nChannels * (params->paramsEfecto->xEje - (params->paramsEfecto->anchoSombra >> 1));
        uint8_t* ccSig = sig->imageData + y * sig->widthStep + res->nChannels * ctes.xLimite;
        uint8_t* ccRes = res->imageData + y * res->widthStep + res->nChannels * (params->paramsEfecto->xEje - (params->paramsEfecto->anchoSombra >> 1));
        
        // Hacer la primera mitad del sombreado, si aplicable
        (*sombreadoPrimeraMitad)(&ccActual, &ccRes, params);

        // Transformar columnas desde el eje hasta el límite
        (*primeraTransformacionColumnas)(ccActual, &ccRes, params);

        // Copiar parte no transformada de imagen siguiente
        for (int j = ctes.xLimite; j <= ctes.xLimiteAnterior; ++j)
        {
            for (int k = 0; k < res->nChannels; ++k)
            {
                *ccRes++ = *ccSig++;
            }
        }
    }
}

static void computarVariablesPrimeraSombra(param_hilo* params)
{
    computarVariablesPrimeraNoSombra(params);

    // Computa constantes usadas en bucles internos que solo cambian con el ángulo de giro
    // y solo se usan al activar efectos de sombreado
    ctes.sumando = 65536 - (params->paramsEfecto->omega << 16) / 90;
    ctes.operando = ((params->paramsEfecto->omega << 8) / 90) * (256 / params->paramsEfecto->anchoSombra);
    ctes.xLimiteSombra = min(params->paramsEfecto->xEje + (params->paramsEfecto->anchoSombra >> 1), ctes.xLimite);
}

static void computarVariablesPrimeraNoSombra(param_hilo* params)
{
    ctes.ce = cosenosEnteros[params->paramsEfecto->omega];
    ctes.cie = cosenosInversosEnteros[params->paramsEfecto->omega - 1];

    // Máxima coordenada X para la que hay píxeles de la página que se dobla
    ctes.xLimite = (((params->paramsEfecto->res->width - params->paramsEfecto->xEje) * ctes.ce) >> 13) + params->paramsEfecto->xEje;
	ctes.xLimiteAnterior = (((params->paramsEfecto->res->width - params->paramsEfecto->xEje) * cosenosEnteros[params->paramsEfecto->omega - params->paramsEfecto->pasoGiro]) >> 13) + params->paramsEfecto->xEje;
}

static void primeraMitadSombra(uint8_t** ccActual, uint8_t** ccRes, param_hilo* params)
{
    // Sombrear píxeles hasta el eje
    for (int x = params->paramsEfecto->xEje - (params->paramsEfecto->anchoSombra >> 1); x < params->paramsEfecto->xEje; ++x)
    {
        // El multiplicador a continuación es una versión optimizada del siguiente pseudo-C:
        //float mult = (float) ((1 - params->paramsEfecto->omega / 90) + (params->paramsEfecto->xEje - x) / (params->paramsEfecto->anchoSombra / 2.0f) * (params->paramsEfecto->omega / 90.0f));
        for (int k = 0; k < params->paramsEfecto->res->nChannels; ++k)
        {
            **ccRes = (uint8_t) (((ctes.sumando + ctes.operando * ((params->paramsEfecto->xEje - x) << 1)) * **ccActual) >> 16);
            ++*ccRes;
            ++*ccActual;
        }
    }
}

static void primeraMitadNoSombra(uint8_t** ccActual, uint8_t** ccRes, param_hilo* params)
{
    // Establecer punteros a la posición esperada por otros procedimientos
    *ccActual += (params->paramsEfecto->anchoSombra >> 1) * params->paramsEfecto->res->nChannels;
    *ccRes += (params->paramsEfecto->anchoSombra >> 1) * params->paramsEfecto->res->nChannels;
}

static void primTransformacionSombra(uint8_t* ccActual, uint8_t** ccRes, param_hilo* params)
{
    int x = params->paramsEfecto->xEje;
    int xPrima;

    // Coordenada horizontal transformada del píxel de la imagen actual, con sombreado
    for (xPrima = ((x - params->paramsEfecto->xEje) * ctes.cie) >> 13;
        x < ctes.xLimiteSombra;
        ++x, xPrima = ((x - params->paramsEfecto->xEje) * ctes.cie) >> 13)
    {
        // Copiar píxeles, incluyendo sombra
        uint8_t* ccActualTrans = ccActual + params->paramsEfecto->res->nChannels * xPrima;
        for (int k = 0; k < params->paramsEfecto->res->nChannels; ++k)
        {
            **ccRes = (uint8_t) (((ctes.sumando + ctes.operando * ((x - params->paramsEfecto->xEje) << 1)) * *ccActualTrans++) >> 16);
            ++*ccRes;
        }
    }

    // Aplicar solamente transformaciones de píxeles
    for (; x < ctes.xLimite;
        ++x, xPrima = ((x - params->paramsEfecto->xEje) * ctes.cie) >> 13)
    {
        uint8_t* ccActualTrans = ccActual + params->paramsEfecto->res->nChannels * xPrima;
        for (int k = 0; k < params->paramsEfecto->res->nChannels; ++k)
        {
            **ccRes = *ccActualTrans++;
            ++*ccRes;
        }
    }
}

static void primTransformacionNoSombra(uint8_t* ccActual, uint8_t** ccRes, param_hilo* params)
{
    for (int x = params->paramsEfecto->xEje; x < ctes.xLimite; ++x)
    {
        int xPrima = ((x - params->paramsEfecto->xEje) * ctes.cie) >> 13;

        // Coordenada horizontal transformada del píxel de la imagen actual
        uint8_t* ccActualTrans = ccActual + params->paramsEfecto->res->nChannels * xPrima;
        for (int k = 0; k < params->paramsEfecto->res->nChannels; ++k)
        {
            **ccRes = *ccActualTrans++;
            ++*ccRes;
        }
    }
}

void eliminarArtefactosPasoGiro(param_hilo* params)
{
    IplImage* sig = params->paramsEfecto->sig;
    IplImage* res = params->paramsEfecto->res;

    // Copiar parte no transformada de imagen siguiente restante
    for (int y = params->idHilo; y < res->height; y += params->paramsEfecto->nHilos)
    {
        uint8_t* ccSig = sig->imageData + y * sig->widthStep + res->nChannels * params->paramsEfecto->xEje;
        uint8_t* ccRes = res->imageData + y * res->widthStep + res->nChannels * params->paramsEfecto->xEje;

        for (int x = params->paramsEfecto->xEje; x < (((res->width - params->paramsEfecto->xEje) * cosenosEnteros[params->paramsEfecto->omega]) >> 13) + params->paramsEfecto->xEje; ++x)
        {
            for (int k = 0; k < res->nChannels; ++k)
            {
                *ccRes++ = *ccSig++;
            }
        }
    }
}

void segundaMitad(param_hilo* params)
{
    IplImage* siguiente = params->paramsEfecto->sig;
    
    // Computar constantes necesarias
    (*computarVariablesSegunda)(params);

    for (int y = params->idHilo; y < params->paramsEfecto->res->height; y += params->paramsEfecto->nHilos)
    {
        uint8_t* ccSiguiente = siguiente->imageData + y * siguiente->widthStep + params->paramsEfecto->res->nChannels * params->paramsEfecto->xEje;
        uint8_t* ccRes = params->paramsEfecto->res->imageData + y * params->paramsEfecto->res->widthStep + params->paramsEfecto->res->nChannels * ctes.xLimite;

        (*segundaTransformacionColumnas)(ccSiguiente, &ccRes, params);

        (*sombreadoSegundaMitad)(ccSiguiente, ccRes, params);
    }
}

static void computarVariablesSegundaSombra(param_hilo* params)
{
    computarVariablesSegundaNoSombra(params);

    // Computa constantes usadas en bucles internos que solo cambian con el ángulo de giro
    // y solo se usan al activar efectos de sombreado
    ctes.sumando = 65536 - ((180 - params->paramsEfecto->omega) << 16) / 90;
    ctes.operando = (((180 - params->paramsEfecto->omega) << 8) / 90) * (256 / params->paramsEfecto->anchoSombra);
    ctes.xLimiteSombra = max(params->paramsEfecto->xEje - (params->paramsEfecto->anchoSombra >> 1), ctes.xLimite);
}

static void computarVariablesSegundaNoSombra(param_hilo* params)
{
    ctes.ce = -cosenosEnteros[180 - params->paramsEfecto->omega];
    // IMPORTANTE: lo matemáticamente correcto (y sensato) es que el valor al que se inicialice
    // cie tenga el signo opuesto. Sin embargo, el comportamiento del redondeo es diferente
    // al usar bit shifts entre números negativos y positivos, y queremos el tratamiento recibido por números positivos.
    // No usar tal redondeo genera pequeños artefactos en la imagen, que me pasé un buen rato intentando depurar.
    // CONSECUENCIA: debemos de cambiar de signo el resultado computado con esta variable para poder usarlo. Esto debería
    // de ser una operación relativamente rápida (calcular ca2, que es ca1 + 1 = NOT res + 1)
    ctes.cie = cosenosInversosEnteros[179 - params->paramsEfecto->omega];

    // Máxima coordenada X en la que se mantienen píxeles de la primera imagen
    ctes.xLimite = params->paramsEfecto->xEje - ((params->paramsEfecto->xEje * -ctes.ce) >> 13);
}

static void segTransformacionSombra(uint8_t* ccSiguiente, uint8_t** ccRes, param_hilo* params)
{
    int x = ctes.xLimite;
    int xPrima;

    // Coordenada horizontal transformada del píxel de la imagen actual
    for (xPrima = ((x - params->paramsEfecto->xEje) * ctes.cie) >> 13;
        x < ctes.xLimiteSombra;
        ++x, xPrima = ((x - params->paramsEfecto->xEje) * ctes.cie) >> 13)
    {
        // Transformar la posición de las columnas actuales dependiendo del ángulo de giro
        uint8_t* ccSiguienteTrans = ccSiguiente + params->paramsEfecto->res->nChannels * xPrima;
        for (int k = 0; k < params->paramsEfecto->res->nChannels; ++k)
        {
            **ccRes = *ccSiguienteTrans++;
            ++*ccRes;
        }
    }

    for (; x < params->paramsEfecto->xEje;
        ++x, xPrima = ((x - params->paramsEfecto->xEje) * ctes.cie) >> 13)
    {
        // Transformar la posición de las columnas actuales dependiendo del ángulo de giro. Añadir sombra
        uint8_t* ccSiguienteTrans = ccSiguiente + params->paramsEfecto->res->nChannels * xPrima;
        for (int k = 0; k < params->paramsEfecto->res->nChannels; ++k)
        {
            **ccRes = (uint8_t) (((ctes.sumando + ctes.operando * ((params->paramsEfecto->xEje - x) << 1)) * *ccSiguienteTrans++) >> 16);
            ++*ccRes;
        }
    }
}

static void segTransformacionNoSombra(uint8_t* ccSiguiente, uint8_t** ccRes, param_hilo* params)
{
    for (int x = ctes.xLimite; x < params->paramsEfecto->xEje; ++x)
    {
        // Coordenada horizontal transformada del píxel de la imagen actual
        int xPrima = ((x - params->paramsEfecto->xEje) * ctes.cie) >> 13;

        // Transformar la posición de las columnas actuales dependiendo del ángulo de giro
        uint8_t* ccSiguienteTrans = ccSiguiente + params->paramsEfecto->res->nChannels * xPrima;
        for (int k = 0; k < params->paramsEfecto->res->nChannels; ++k)
        {
            **ccRes = *ccSiguienteTrans++;
            ++*ccRes;
        }
    }
}

static void segundaMitadSombra(uint8_t* ccSiguiente, uint8_t* ccRes, param_hilo* params)
{
    // Añadir sombreado adicional a partir del eje
    for (int x = params->paramsEfecto->xEje; x < params->paramsEfecto->xEje + (params->paramsEfecto->anchoSombra >> 1); ++x)
    {
        // El multiplicador a continuación es una versión optimizada del siguiente pseudo-C:
        //float mult = (float) ((params->paramsEfecto->omega - 90) / 90 + (x - params->paramsEfecto->xEje) / (params->paramsEfecto->anchoSombra / 2.0f) * (1 - (params->paramsEfecto->omega - 90) / 90));
        for (int k = 0; k < params->paramsEfecto->res->nChannels; ++k)
        {
            *ccRes++ = (uint8_t) (((ctes.sumando + ctes.operando * ((x - params->paramsEfecto->xEje) << 1)) * *ccSiguiente++) >> 16);
        }
    }
}

static void segundaMitadNoSombra(uint8_t* ccSiguiente, uint8_t* ccRes, param_hilo* params)
{
    return;
}