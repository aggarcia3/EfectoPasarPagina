#include "lista_enlazada.h"

// Implementación de una lista simplemente enlazada desenrollada con referencias al primer y al último nodo.
// Mejora el rendimiento de la inserción respecto un array dinámico, siendo de complejidad O(1) incluso en el peor de los casos.
// Además debería de aprovechar la localidad de referencia mejor que una lista simplemente enlazada trivial.
// Un array dinámico (realloc) tiene un problema: para una operación de inserción de coste O(1) amortizado requiere
// una progresión geométrica del tamaño del array, y una operación de inserción no amortizada es de complejidad lineal.
// Ambas alternativas con realloc no me convencen en caso de trabajar con muchas imágenes (una por consumo de RAM, otra por consumo de CPU).
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
        // Sería el primer elemento de un nodo. Necesitamos crear un nodo nuevo
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
        // Usar el último nodo, tiene hueco aún
        n = lista->ultimoNodo;
    }

    // Añadir el elemento al nodo en cuestión
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