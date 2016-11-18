#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "abb.h"
#include "pila.h"

/* Definición del struct nodo.
 */

typedef struct nodo_abb{
	struct nodo_abb* h_izq;
	struct nodo_abb* h_der;
	char* clave;
	void* dato;
}nodo_abb_t;

/* Definición del struct abb.
 */
 
struct abb{
	nodo_abb_t* raiz;
	size_t cant;
	abb_comparar_clave_t funcion_comparacion;
	abb_destruir_dato_t destruir_dato;
};

/* Definición del struct iterador.
 */
 
struct abb_iter{
	const abb_t* arbol;
	pila_t* pila_nodos;
};


/* *****************************************************************
 *                    PRIMITIVAS DEL ABB
 * *****************************************************************/
 
abb_t* abb_crear(abb_comparar_clave_t cmp, abb_destruir_dato_t destruir_dato){
	abb_t* abb = malloc(sizeof(abb_t));
	if(abb == NULL){return NULL;}
	abb->raiz = NULL;
	abb->cant = 0;
	abb->funcion_comparacion = cmp;
	abb->destruir_dato = destruir_dato;
	return abb;
}

//Dada una clave y dato crea un nodo_abb.
//Si no se pudo crear devuelve NULL.
nodo_abb_t* crear_nodo(const char *clave, void *dato){
	nodo_abb_t* nodo_abb = malloc(sizeof(nodo_abb_t));
	
	if(nodo_abb == NULL){return NULL;}
	
	nodo_abb->clave = calloc(strlen(clave) + 1, sizeof(char));

	if(!nodo_abb->clave){
		free(nodo_abb);
		return NULL;
	}
	
	strcpy(nodo_abb->clave,clave);
	
	nodo_abb->h_izq = NULL;
	nodo_abb->h_der = NULL;
	nodo_abb->dato = dato;
	return nodo_abb;
}

// Busca un nodo según la clave y devuelve un puntero al mismo.
// Si no lo encuentra (el nodo no está) devuelve NULL.
nodo_abb_t* buscar_nodo(nodo_abb_t* nodo, abb_comparar_clave_t cmp, const char *clave){
	if (!nodo)
		return NULL;
		
	int resultado = cmp(nodo->clave, clave);
	
	if(resultado == 0) // clave == nodo->clave
		return nodo;
	
	if (resultado > 0)
		return buscar_nodo(nodo->h_izq, cmp, clave); // clave < nodo->clave => va a estar a la izquierda
		
	return buscar_nodo(nodo->h_der, cmp, clave); // clave > nodo->clave => va a estar a la derecha
}

nodo_abb_t* buscar_nodo_padre(nodo_abb_t* nodo, abb_comparar_clave_t cmp, const char *clave, nodo_abb_t** padre){
	if (!nodo)
		return *padre;
		
	*padre = nodo;
	
	int resultado = cmp(nodo->clave, clave);
		
	if (resultado > 0)
		return buscar_nodo_padre(nodo->h_izq, cmp, clave, padre); // clave < nodo->clave
		
	return buscar_nodo_padre(nodo->h_der, cmp, clave, padre); // clave > nodo->clave
}

bool abb_guardar(abb_t *arbol, const char *clave, void *dato){
	nodo_abb_t* nodo_ya_existe = buscar_nodo(arbol->raiz, arbol->funcion_comparacion, clave);
	
	if (nodo_ya_existe){
		if(arbol->destruir_dato){
			arbol->destruir_dato(nodo_ya_existe->dato);
		}
		nodo_ya_existe->dato = dato; // ya estaba entonces piso dato
		return true;
	}
	
	// no está entonces hay que crear un nodo para guardar
	nodo_abb_t* nodo_abb = crear_nodo(clave, dato);
	
	if(nodo_abb == NULL){return false;}
	arbol->cant++;
	
	if (!arbol->raiz){
		arbol->raiz = nodo_abb;
		return true;		
	}
		
	nodo_abb_t* nodo_aux;	
	nodo_abb_t* nodo_padre = buscar_nodo_padre(arbol->raiz, arbol->funcion_comparacion, clave, &nodo_aux);

	// ya se el padre ahora falta saber si soy su hijo derecho o izquierdo
	int resultado = arbol->funcion_comparacion(nodo_padre->clave, clave);
		
	if (resultado > 0){
		nodo_padre->h_izq = nodo_abb;   // clave < nodo_padre->clave => soy su hijo izquierdo											
	}else{
		nodo_padre->h_der = nodo_abb;	// clave > nodo_padre->clave => soy su hijo derecho	
	}
	
	return true;
}

// Seguidilla de funciones relacionadas al abb_borrar

void *ver_caso_borrado(abb_t *arbol, nodo_abb_t *actual, nodo_abb_t *anterior);

void *_abb_borrar(abb_t *arbol, const char *clave, nodo_abb_t *actual, nodo_abb_t *anterior){
	if (!actual)
		return NULL;
		
	int resultado = arbol->funcion_comparacion(actual->clave,clave);
	
	if (resultado == 0)
		return ver_caso_borrado(arbol, actual, anterior);

	if (resultado > 0)
		return _abb_borrar(arbol, clave, actual->h_izq, actual);
	
	return _abb_borrar(arbol, clave, actual->h_der, actual);
}

void acciones_borrado(abb_t *arbol, nodo_abb_t *nodo){
	arbol->cant--;
	free(nodo->clave);
	free(nodo);	
}

void *borrar_caso_hoja(abb_t *arbol, nodo_abb_t *actual, nodo_abb_t *anterior){
	void *dato = actual->dato;

	if (!anterior){
		arbol->raiz = NULL;
	}else{
		if (anterior->h_izq == actual){
			anterior->h_izq = NULL;
		}else{
			anterior->h_der = NULL;
		}
	}
	acciones_borrado(arbol, actual);
	return dato;
}

void *borrar_caso_un_hijo(abb_t *arbol, nodo_abb_t *actual, nodo_abb_t *anterior){
	void *dato = actual->dato;
	
	if (!anterior){
		if (actual->h_izq){
			arbol->raiz = actual->h_izq;
		}else{
			arbol->raiz = actual->h_der;
		}
	}else{
		if (anterior->h_izq == actual){
			if (actual->h_izq){
				anterior->h_izq = actual->h_izq;
			}else{
				anterior->h_izq = actual->h_der;
			}
		}else{
			if (actual->h_izq){
				anterior->h_der = actual->h_izq;
			}else{
				anterior->h_der = actual->h_der;
			}
		}
	}	
	acciones_borrado(arbol, actual);
	return dato;
}

nodo_abb_t* reemplazo(nodo_abb_t* nodo){
	if (nodo->h_izq)
		return reemplazo(nodo->h_izq);
	
	return nodo;
}

void* borrar_caso_dos_hijos(abb_t *arbol, nodo_abb_t *actual, nodo_abb_t *anterior){
	void* dato = actual->dato;
	
	nodo_abb_t* nodo_reemplazo = actual->h_der;
	nodo_reemplazo = reemplazo(nodo_reemplazo);
	
	char* clave_aux = calloc(strlen(nodo_reemplazo->clave) + 1, sizeof(char));
	if (!clave_aux)
		return NULL;
	
	strcpy(clave_aux, nodo_reemplazo->clave);
	
	void* dato_aux = abb_borrar(arbol, clave_aux);

	free(actual->clave);
	actual->clave = clave_aux;
	actual->dato = dato_aux;
	return dato;
}

bool es_hoja(nodo_abb_t* nodo){
	return (!nodo->h_der && !nodo->h_izq);
}

void *ver_caso_borrado(abb_t *arbol, nodo_abb_t *actual, nodo_abb_t *anterior){
	if (es_hoja(actual))
		return borrar_caso_hoja(arbol, actual, anterior);
	
	if (actual->h_izq && actual->h_der)
		return borrar_caso_dos_hijos(arbol, actual, anterior);
	
	return borrar_caso_un_hijo(arbol, actual, anterior);
}

void *abb_borrar(abb_t *arbol, const char *clave){
	if (!arbol->raiz)
		return NULL;
		
	nodo_abb_t *anterior = NULL;
	nodo_abb_t *actual = arbol->raiz;

	return _abb_borrar(arbol, clave, actual, anterior);
}

// fin funciones borrado

bool abb_pertenece(const abb_t *arbol, const char *clave){
	nodo_abb_t* nodo_a_buscar = buscar_nodo(arbol->raiz, arbol->funcion_comparacion, clave);
		
	if (!nodo_a_buscar)
		return false;
	
	return true;
}

void *abb_obtener(const abb_t *arbol, const char *clave){
	nodo_abb_t* nodo_a_buscar = buscar_nodo(arbol->raiz, arbol->funcion_comparacion, clave);
		
	if (!nodo_a_buscar)
		return NULL;
	
	return nodo_a_buscar->dato;
}

size_t abb_cantidad(abb_t *arbol){
	return arbol->cant;	
}

//Destruye los nodos. junto con sus claves y datos, en postorder.
void abb_destruir_nodos(abb_t *arbol, nodo_abb_t *nodo){
	if(!nodo){
		return;
	}
	abb_destruir_nodos(arbol, nodo->h_izq);
	abb_destruir_nodos(arbol, nodo->h_der);
	free(nodo->clave);
	if(arbol->destruir_dato){
		arbol->destruir_dato(nodo->dato);
	}
	free(nodo);
}

void abb_destruir(abb_t *arbol){
	abb_destruir_nodos(arbol, arbol->raiz);
	free(arbol);
}


/* *****************************************************************
 *                    PRIMITIVAS DEL ITERADOR EXTERNO INORDER
 * *****************************************************************/

//Apila todos los hijos izquierdos de un nodo dado.
void apilar_hijos_izquierdos(pila_t *pila, nodo_abb_t *nodo){
	if(nodo->h_izq){
		pila_apilar(pila, nodo->h_izq);
		apilar_hijos_izquierdos(pila, nodo->h_izq);
	}
}

//Posiciona al iterador en el primer nodo inorder del arbol.
void abb_iter_in_al_principio(abb_iter_t *iter){
	if(iter->arbol->raiz){
		pila_apilar(iter->pila_nodos, iter->arbol->raiz);
		apilar_hijos_izquierdos(iter->pila_nodos, iter->arbol->raiz);
	}
}

abb_iter_t *abb_iter_in_crear(const abb_t *arbol){
	abb_iter_t* abb_iter = malloc(sizeof(abb_iter_t));
	if(!abb_iter){return NULL;}
	pila_t* pila = pila_crear();
	if(!pila){
		free(abb_iter);
		return NULL;
	}
	abb_iter->arbol = arbol;
	abb_iter->pila_nodos = pila;
	abb_iter_in_al_principio(abb_iter);
	return abb_iter;
}

bool abb_iter_in_avanzar(abb_iter_t *iter){
	if(abb_iter_in_al_final(iter)){
		return false;
	}
	nodo_abb_t* nodo = pila_desapilar(iter->pila_nodos);
	if(nodo->h_der){
		pila_apilar(iter->pila_nodos, nodo->h_der);
		apilar_hijos_izquierdos(iter->pila_nodos, nodo->h_der);
	}
	return true;
}

bool abb_iter_in_al_final(const abb_iter_t *iter){
	return pila_esta_vacia(iter->pila_nodos);
}

const char *abb_iter_in_ver_actual(const abb_iter_t *iter){
	if (!pila_esta_vacia(iter->pila_nodos))
		return (((nodo_abb_t*)pila_ver_tope(iter->pila_nodos))->clave);
	
	return NULL;	
}

void abb_iter_in_destruir(abb_iter_t* iter){
	pila_destruir(iter->pila_nodos);
	free(iter);	
}


/* *****************************************************************
 *                    PRIMITIVAS DEL ITERADOR INTERNO INORDER
 * *****************************************************************/
 
bool nodo_abb_in_order(nodo_abb_t *nodo_abb, bool visitar(const char *, void *, void *), void *extra){
	if (!nodo_abb)
		return true;	
	
	if (!nodo_abb_in_order(nodo_abb->h_izq, visitar, extra))
		return false;	
	
	if (!visitar(nodo_abb->clave,nodo_abb->dato,extra))
		return false;
	
	return nodo_abb_in_order(nodo_abb->h_der, visitar, extra);				
}

void abb_in_order(abb_t *arbol, bool visitar(const char *, void *, void *), void *extra){
	if (arbol->raiz)
		nodo_abb_in_order(arbol->raiz,visitar,extra);	
}


/* *****************************************************************
 *                    PRIMITIVAS DEL ITERADOR EXTERNO POSTORDER
 * *****************************************************************/

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
 *                    PRIMITIVAS DEL ITERADOR INTERNO POSTORDER
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
