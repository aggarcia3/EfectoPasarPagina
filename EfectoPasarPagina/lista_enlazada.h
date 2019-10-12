#pragma once

#include "stdafx.h"

// El tama�o de una l�nea de la cach� suele ser de 64 bytes.
// Para que los elementos de un nodo quepan en dos l�neas de la cach� de manera �ptima,
// el n�mero de elementos n debe de ser el m�ximo tal que n <= 128 / sizeof(void*).
// Asumiendo sizeof(void*) == 8, resulta que n = ELEMS_NODO = 16.
#define ELEMS_NODO 16

typedef struct nodo nodo;
struct nodo
{
    nodo* sig;
    void** elementos;    // Array de ELEMS_NODO elementos
};

typedef struct
{
    nodo* primerNodo;
    nodo* ultimoNodo;
    size_t nElementos;
    size_t nNodos;
} listaEnlazada;

typedef struct
{
    listaEnlazada* lista;
    size_t elemSig;
    nodo* nodoActual;
} listaEnlazadaIter;

// Crea una nueva lista enlazada, sin elementos en ella.
// Se garantiza que el campo nElementos sea igual a cero.
// En caso de no haber memoria suficiente para realizar la operaci�n, esta funci�n devuelve un puntero nulo.
// Los usuarios de los campos de las estructuras listaEnlazada y nodo solo deben de realizar operaciones observadoras sobre ellos.
// Se recomienda utilizar el iterador proporcionado para recorrer secuencialmente la lista.
listaEnlazada* crearLista();

// A�ade un elemento, pasado por referencia, al final de la lista enlazada especificada.
// En caso de no haber memoria suficiente para realizar la operaci�n, o ser lista un puntero nulo, este procedimiento falla silenciosamente.
void anadirLista(listaEnlazada* lista, void* elemento);

// Vac�a la lista enlazada de elementos, dej�ndola en su estado inicial.
// Opcionalmente, puede especificarse un procedimiento que se ejecutar� en cada elemento antes de ser borrado de la lista, por ejemplo para liberar los recursos que utiliza.
// En caso de ser lista un puntero nulo, este procedimiento falla silenciosamente.
void vaciarLista(listaEnlazada* lista, void(*procVaciado)(void*));

// Elimina la lista enlazada de elementos, de tal manera que deje de existir en memoria. Todas las referencias a ella se volver�n inv�lidas.
// Opcionalmente, puede especificarse un procedimiento que se ejecutar� en cada elemento, por ejemplo para liberar los recursos que utiliza.
// En caso de ser lista un puntero nulo, este procedimiento falla silenciosamente.
void eliminarLista(listaEnlazada* lista, void(*procElim)(void*));

// Crea un registro iterador unidireccional sobre la lista enlazada especificada. Inicialmente, el iterador tiene como elemento siguiente el primero, si lo hay. De no haberlo, devuelve nulo.
listaEnlazadaIter* iterLista(listaEnlazada* lista);

// Devuelve el siguiente elemento apuntado por el iterador dado, y avanza una posici�n en la lista. De no haber tal elemento, devuelve nulo.
void* iterSig(listaEnlazadaIter* it);

// Devuelve verdadero si el iterador especificado tiene un elemento siguiente, falso en caso contrario.
BOOL iterTieneSig(listaEnlazadaIter* it);

// Borra el iterador especificado de memoria, liberando los recursos que utiliza previamente.
// Esto no afecta a la lista sobre la que itera.
void finalizarIter(listaEnlazadaIter* it);