#include "hash.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#define TAM_I 191//Tamanio inicial de la tabla de hash
#define STEP 5//Cantidad de posiciones que me muevo para buscar una clave en la 
//tabla

typedef struct nodo_hash{
	void* dato;
	char* clave;
	enum {vacio,ocupado,borrado} estado;
}nodo_t;

struct hash{
	nodo_t** tabla;
	size_t tam;//Tamanio actual de la tabla 
	size_t cant;//Cantidad de elementos en el hash
	size_t borrados;
	hash_destruir_dato_t destruir_dato;
};

typedef struct hash_iter{
	const hash_t* hash;
	size_t posicion;
}hash_iter_t;


//Crea un nodo y lo inicializa a NULL.
//Si todo se realizo correctamente lo devuelve.
//En caso de fallar devuelve NULL
nodo_t* crear_nodo(void){
	nodo_t* nodo = malloc(sizeof(nodo_t));
	if(nodo == NULL) return NULL;
	nodo -> estado = vacio;
	nodo -> clave = NULL;
	nodo -> dato = NULL;
	return nodo;
}
//Recibe un tamanio
//Crea una tabla donde cada posicion es un nodo. 
//En caso de fallar devuelve NULL
nodo_t** hash_crear_tabla(size_t  tam){
	nodo_t** tabla = malloc(tam*sizeof(nodo_t*));
	if(tabla == NULL){
		free(tabla);
		return NULL;
	}
	unsigned int i;

	for (i=0;i<tam;i++){
		tabla[i]=crear_nodo();
		if(tabla[i]==NULL){
			free(tabla);
			return NULL;
		}
	}	
	return tabla;

}
hash_t *hash_crear(hash_destruir_dato_t destruir_dato){
	hash_t* hash = malloc(sizeof(hash_t));
	if(hash == NULL) return NULL;
	hash-> tabla = hash_crear_tabla(TAM_I);
	if(hash -> tabla == NULL){
		free(hash);
		return NULL;
	}
	hash -> cant = 0;
	hash -> borrados = 0;
	hash -> tam = TAM_I;
	hash -> destruir_dato = destruir_dato;
	return hash;
}

/*******************************************************************************
*				FUNCION DE HASHING (Robert Sedgwicks algorithms in C)		   *
*******************************************************************************/
unsigned int hashear(const char* str, size_t max){ 
	unsigned int b    = 378551;
	unsigned int a    = 63689;
	unsigned int hash = 0;
	unsigned int i    = 0;
	for(i = 0;*str; str++, i++){
		hash = hash * a + (*str);
		a = a * b;
	}
	return (hash%(unsigned int)max);
}

//Devuelve true o false segun la disponibilidad de una posicion.
bool pos_disponible(hash_t* hash, unsigned int pos){
		return(hash->tabla[pos]->estado == vacio);
	}

char *strdup(const char *s)
{
    char *n = malloc(strlen(s) + 1);
    if(n == NULL) return NULL;
    strcpy(n, s);
    return n;
}

//Devuelve la pos de la clave recibida.
//Pre: clave debe pertenecer al hash.
unsigned int obtener_pos(hash_t *hash, const char *clave){
	unsigned int pos = hashear(clave, hash->tam);
	while(hash->tabla[pos]->estado!=vacio){
		if(hash->tabla[pos]->estado == ocupado && strcmp(clave,
			(hash->tabla[pos])->clave)==0)break;
		pos += STEP;
		if(pos >= hash -> tam) pos = 0;
	}
	return pos;
}
//Crea una nueva tabla para el hash. Devuelve true si se realizo correctamente
bool hash_redimensionar(hash_t* hash){
	double fc = (double)(hash->cant + hash->borrados)/((double)hash->tam);
	if( (fc>=0.7)||((fc<=0.15) && (hash->tam > TAM_I))){
		size_t tam;
		//Si esta al 70% agrando al triple la tabla
		if(fc>=0.7) tam = hash->tam*3;
		//SI esta al 15% de ocupacion, disminuyo el tamanio a la mitad
		else tam = hash->tam/2;
		nodo_t** aux = hash_crear_tabla(tam);
		if(aux == NULL)return false;
		unsigned int i;
		for(i=0;i<hash->tam;i++){
			nodo_t* nodo = hash->tabla[i];
			if(nodo->estado == ocupado){
				unsigned int pos = hashear(nodo->clave,tam);
				while(aux[pos]->estado!=vacio){
					pos+=STEP;
					if(pos >= tam)pos=0;
				}
				aux[pos]->clave = nodo->clave;
				aux[pos]->dato = nodo -> dato;
				aux[pos]->estado = ocupado;
			}
			free(hash->tabla[i]);
		}
		free(hash->tabla);
		hash->tam = tam;
		hash->tabla = aux;
	}
	return true;
}

bool hash_guardar(hash_t *hash, const char *clave, void *dato){
	unsigned int pos;
	if(hash_pertenece(hash,clave)){
		pos = obtener_pos(hash,clave);
		if(hash->destruir_dato)hash->destruir_dato(hash->tabla[pos]->dato);
	}
	else{
		char *copia_clave=strdup(clave);
		if(copia_clave==NULL)return false;
		pos = hashear(clave, hash->tam);
		while(!pos_disponible(hash, pos)){
			pos += STEP;
			if(pos >= hash->tam)pos=0;
		}
		hash->cant++;
		hash->tabla[pos]-> clave = copia_clave;
	}
	hash->tabla[pos]-> estado = ocupado;
	hash->tabla[pos]-> dato = dato;
	if(!hash_redimensionar(hash))return false; 

	return true;
}

void *hash_borrar(hash_t *hash, const char *clave){
	if(!hash_pertenece(hash, clave)) return NULL;
	unsigned int pos = obtener_pos(hash, clave);
	hash->tabla[pos]->estado = borrado;
	free((char*)hash->tabla[pos]->clave);
	hash->borrados ++;
	hash->cant--;
	if(!hash_redimensionar(hash))return false; 
	return hash->tabla[pos]->dato;
}

void *hash_obtener(const hash_t *hash, const char *clave){
	if(!hash_pertenece(hash,clave))return NULL;
	return (hash -> tabla[obtener_pos((hash_t*)hash,clave)])->dato;
}

bool hash_pertenece(const hash_t *hash, const char *clave){
	unsigned int pos = hashear(clave,hash-> tam);
	while(pos < hash->tam){
		nodo_t* nodo = hash->tabla[pos];
		if(nodo->estado == vacio) return false;
		if(nodo -> estado == borrado){
			pos+=STEP;
			continue;}
		if(strcmp(nodo->clave,clave)==0)return true;
		pos+=STEP;
		if(pos >= hash -> tam)pos = 0;	

	}
	return false;
}
size_t hash_cantidad(const hash_t *hash){
	return hash -> cant;
}

void hash_destruir(hash_t *hash){
	size_t i=0;
	for(i=0;i<hash->tam;i++){
		if(hash->tabla[i]->estado == ocupado){
			if(hash->destruir_dato!=NULL){
				hash->destruir_dato(hash->tabla[i]->dato);
			}
			free((char*)hash->tabla[i]->clave);
		}		
		free(hash->tabla[i]);
	}
	free(hash->tabla);
	free(hash);
	}
	

bool hash_iter_al_final(const hash_iter_t *iter){
	return(iter->posicion >= iter->hash->tam);
}

bool hash_iter_avanzar(hash_iter_t *iter){
	iter->posicion ++;
	while(!hash_iter_al_final(iter)){
		if(iter->hash->tabla[iter->posicion]->estado!=ocupado)
			iter->posicion ++;
		else return true;
	}
	return false;
}

hash_iter_t *hash_iter_crear(const hash_t *hash){
	hash_iter_t* iter = malloc(sizeof(hash_iter_t));
	if(iter == NULL)
		return NULL;
	iter->hash = hash;	
	if(hash->cant ==0) iter-> posicion = hash->tam;
	else{
		iter->posicion = 0;
	    while(iter->hash->tabla[iter->posicion]->estado != ocupado && iter->posicion < hash->tam){
			iter->posicion ++;
		}
	}
	return iter;
}

const char *hash_iter_ver_actual(const hash_iter_t *iter){
	if(hash_iter_al_final(iter)) return NULL;
	return iter->hash->tabla[iter->posicion]->clave;
}

void hash_iter_destruir(hash_iter_t* iter){
	free(iter);
}
