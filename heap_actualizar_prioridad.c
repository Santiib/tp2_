#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "heap.h"

void heap_actualizar_prioridad(heap_t* heap, void* dato){
	size_t pos;
	for(pos = 0; pos < heap->cant; pos++){
		if(heap->datos[pos] == dato){
			break;
		}
	}
	if(pos == heap->cant){
		return;
	}
	
	if(heap->funcion_comparacion(heap->datos[(pos-1)/2], dato) < 0){
		upheap(heap->datos, dato, pos, heap->funcion_comparacion);
	}else if(heap->funcion_comparacion(heap->datos[2*pos+1], dato) > 0 || heap->funcion_comparacion(heap->datos[2*pos+2], dato) > 0){
		downheap(heap->datos, heap->cant, pos, heap->funcion_comparacion);
	}
}
