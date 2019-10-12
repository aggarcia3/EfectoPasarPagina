#include "lista_enlazada.h"

// Implementaci�n de una lista simplemente enlazada desenrollada con referencias al primer y al �ltimo nodo.
// Mejora el rendimiento de la inserci�n respecto un array din�mico, siendo de complejidad O(1) incluso en el peor de los casos.
// Adem�s deber�a de aprovechar la localidad de referencia mejor que una lista simplemente enlazada trivial.
// Un array din�mico (realloc) tiene un problema: para una operaci�n de inserci�n de coste O(1) amortizado requiere
// una progresi�n geom�trica del tama�o del array, y una operaci�n de inserci�n no amortizada es de complejidad lineal.
// Ambas alternativas con realloc no me convencen en caso de trabajar con muchas im�genes (una por consumo de RAM, otra por consumo de CPU).
// https://en.wikipedia.org/wiki/Unrolled_linked_list

listaEnlazada* crearLista()
{
    return (listaEnlazada*) calloc(1, sizeof(listaEnlazada));
}

void anadirLista(listaEnlazada* lista, void* elemento)
{
    nodo* n;
    size_t i;
    
    if (lista->nElementos == UINT_MAX)
    {
        // No podemos insertar nada en la lista especificada
        return;
    }

    i = lista->nElementos % ELEMS_NODO;
    if (i == 0)
    {
        // Ser�a el primer elemento de un nodo. Necesitamos crear un nodo nuevo
        n = (nodo*) malloc(sizeof(nodo));

        if (n == NULL)
        {
            return;
        }

        n->elementos = (void**) malloc(ELEMS_NODO * sizeof(void*));

        if (n->elementos == NULL)
        {
            // No se pudo reservar memoria para el array de elementos
            free(n);
            return;
        }
        n->sig = NULL;

        if (lista->nElementos == 0)
        {
            lista->primerNodo = lista->ultimoNodo = n;
        }
        else
        {
            lista->ultimoNodo->sig = n;
            lista->ultimoNodo = n;
        }

        ++lista->nNodos;
    }
    else
    {
        // Usar el �ltimo nodo, tiene hueco a�n
        n = lista->ultimoNodo;
    }

    // A�adir el elemento al nodo en cuesti�n
    n->elementos[i] = elemento;

    ++lista->nElementos;
}

void vaciarLista(listaEnlazada* lista, void(*procVaciado)(void*))
{
    nodo* actual;
    nodo* siguiente;

    actual = lista->primerNodo;

    for (size_t i = lista->nNodos; i > 0; --i, actual = siguiente)
    {
        siguiente = actual->sig;

        if (procVaciado != NULL)
        {
            for (size_t j = 0; j < (siguiente == NULL ? lista->nElementos % ELEMS_NODO : ELEMS_NODO); ++j)
            {
                (*procVaciado)(actual->elementos[j]);
            }
        }

        free(actual->elementos);
        free(actual);
    }

    lista->nNodos = 0;
    lista->nElementos = 0;
}

void eliminarLista(listaEnlazada* lista, void(*procElim)(void*))
{
    vaciarLista(lista, procElim);
    free(lista);
}

listaEnlazadaIter* iterLista(listaEnlazada* lista)
{
    listaEnlazadaIter* toret = NULL;

    if (lista != NULL && (toret = (listaEnlazadaIter*) malloc(sizeof(listaEnlazadaIter)), toret != NULL))
    {
        toret->lista = lista;
        toret->elemSig = 0;
        toret->nodoActual = NULL;
    }

    return toret;
}

void* iterSig(listaEnlazadaIter* it)
{
    void* toret = NULL;

    if (it != NULL && it->lista->nElementos > 0 && it->elemSig < it->lista->nElementos)
    {
        // Inicializar nodo visitado si es necesario.
        // Usamos esto para no tener que recorrer la lista desde el principio con cada llamada a sig
        if (it->nodoActual == NULL)
        {
            it->nodoActual = it->lista->primerNodo;
        }

        toret = it->nodoActual->elementos[it->elemSig % ELEMS_NODO];
        if (++it->elemSig >= ELEMS_NODO)
        {
            it->nodoActual = it->nodoActual->sig;
        }
    }

    return toret;
}

BOOL iterTieneSig(listaEnlazadaIter* it)
{
    return it != NULL && it->elemSig < it->lista->nElementos;
}

void finalizarIter(listaEnlazadaIter* it)
{
    if (it != NULL)
    {
        free(it);
    }
}