#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "abb.h"
#include "pila.h"


/* *****************************************************************
 *                    PRIMITIVAS DEL ITERADOR EXTERNO
 * *****************************************************************/

//Apila la traza izquierda de un nodo (borde izquierdo).
void apilar_traza_izquierda(pila_t *pila, nodo_abb_t *nodo){
	if(nodo->h_izq){
		pila_apilar(pila, nodo->h_izq);
		apilar_traza_izquierda(pila, nodo->h_izq);
	}else if(nodo->h_der){
		pila_apilar(pila, nodo->h_der);
		apilar_traza_izquierda(pila, nodo->h_der);
	}
}

//Posiciona al iterador en el primer nodo postorder del arbol.
void abb_iter_post_al_principio(abb_iter_t *iter){
	if(iter->arbol->raiz){
		pila_apilar(iter->pila_nodos, iter->arbol->raiz);
		apilar_traza_izquierda(iter->pila_nodos, iter->arbol->raiz);
	}
}

abb_iter_t *abb_iter_post_crear(const abb_t *arbol){
	abb_iter_t* abb_iter_post = malloc(sizeof(abb_iter_t));
	if(!abb_iter_post){return NULL;}
	pila_t* pila = pila_crear();
	if(!pila){
		free(abb_iter_post);
		return NULL;
	}
	abb_iter_post->arbol = arbol;
	abb_iter_post->pila_nodos = pila;
	abb_iter_post_al_principio(abb_iter_post);
	return abb_iter_post;
}

bool abb_iter_post_al_final(const abb_iter_t *iter){
	return pila_esta_vacia(iter->pila_nodos);
}

bool abb_iter_post_avanzar(abb_iter_t *iter){
	if(abb_iter_post_al_final(iter)){
		return false;
	}
	nodo_abb_t* nodo = pila_desapilar(iter->pila_nodos);
	nodo_abb_t* tope = pila_ver_tope(iter->pila_nodos);
	if(tope){
		if(nodo == tope->h_izq){
			if(tope->h_der){
				pila_apilar(iter->pila_nodos, tope->h_der);
				apilar_traza_izquierda(iter->pila_nodos, tope->h_der);
			}
		}
	}
	return true;
}

const char *abb_iter_post_ver_actual(const abb_iter_t *iter){
	if (!pila_esta_vacia(iter->pila_nodos))
		return (((nodo_abb_t*)pila_ver_tope(iter->pila_nodos))->clave);
	
	return NULL;	
}

void abb_iter_post_destruir(abb_iter_t* iter){
	pila_destruir(iter->pila_nodos);
	free(iter);	
}


/* *****************************************************************
 *                    PRIMITIVAS DEL ITERADOR INTERNO
 * *****************************************************************/

bool nodo_abb_post_order(nodo_abb_t *nodo_abb, bool visitar(const char *, void *, void *), void *extra){
	if(!nodo_abb)
		return true;
	
	if(!nodo_abb_post_order(nodo_abb->h_izq, visitar, extra))
		return false;
		
	if(!nodo_abb_post_order(nodo_abb->h_der, visitar, extra))
		return false;
	
	return visitar(nodo_abb->clave, nodo_abb->dato, extra);
}


void abb_post_order(abb_t *arbol, bool visitar(const char *, void *, void *), void *extra){
	if (arbol->raiz)
		nodo_abb_post_order(arbol->raiz,visitar,extra);	
}
