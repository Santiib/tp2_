#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "hash.c"


void imprimir_lineas_hash(hash_t* hash){
	hash_iter_t* iter = hash_iter_crear(hash);
	if(!iter){return;}
	while(!hash_iter_al_final(iter)){
		const char* linea = hash_iter_ver_actual(iter);
		printf("%s\n", linea);
		hash_iter_avanzar(iter);
	}
	hash_iter_destruir(iter);
}

char *crear_copia(const char *s)
{
    char *n = malloc(strlen(s) + 1);
    if(n == NULL) return NULL;
    strcpy(n, s);
    return n;
}

void imprimir_lineas(FILE* archivo_1, FILE* archivo_2, int modo){
	hash_t *hash = hash_crear(free);
	if(!hash){return;}
	
	hash_t *ya_verificadas_hash = hash_crear(NULL);
	if(!ya_verificadas_hash){
		hash_destruir(hash);
		return;
	}
	
	char *linea = NULL;
	size_t len = 0;
	while(getline(&linea, &len, archivo_1) > 0){
		if(!hash_pertenece(hash, linea)){
			char *copia = crear_copia(linea);
			if(!copia){return;}
			hash_guardar(hash, linea, copia);
		}
	}
	
	while(getline(&linea, &len, archivo_2) > 0){
	
		if(!hash_pertenece(ya_verificadas_hash, linea)){
			if(hash_pertenece(hash, linea)){
				if(modo == 0){
					printf("%s\n", linea);
				}else if(modo == 1){
					char *a_borrar = hash_borrar(hash, linea);
					free(a_borrar);
				}
			}else{
				if(modo == 2){
					printf("%s\n", linea);
				}
			}
			hash_guardar(ya_verificadas_hash, linea, NULL);
		}
	
	}
	
	if(modo == 1){
		imprimir_lineas_hash(hash);
	}
	
	if(linea){free(linea);}
	hash_destruir(hash);
	hash_destruir(ya_verificadas_hash);
}

int verificar_modo(char* modo){
	if(strcmp(modo, "-1") == 0){
		return 1;
	}else if(strcmp(modo, "-2") == 0){
		return 2;
	}else{
		return -1;
	}
}

int main(int argc, char **argv){
	if(argc != 3 && argc != 4){return 0;}
	int modo = 0;
	if(argc == 4){
		modo = verificar_modo(argv[3]);
		if(modo < 0){return 0;}
	}
	
	FILE* archivo_1 = fopen(argv[1], "r");
	if(!archivo_1){
		fprintf(stderr,"%s\n","No se pudo abrir el archivo");
		return 0;
	}
	FILE* archivo_2 = fopen(argv[2], "r");
	if(!archivo_2){
		fclose(archivo_1);
		fprintf(stderr,"%s\n","No se pudo abrir el archivo");
		return 0;
	}
	
	imprimir_lineas(archivo_1, archivo_2, modo);
	fclose(archivo_1);
	fclose(archivo_2);
	return 1;
}
