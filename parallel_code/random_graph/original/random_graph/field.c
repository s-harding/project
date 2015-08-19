#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 

#include "machine.h" 
#include "array.h" 
#include "field.h" 


Field * init_field(Array * a){

	int i, j, size;
	double d;
	Field *f = malloc(sizeof(Field));

	size = a->x_local; // store space for all local points

	f->value = malloc(sizeof(int)*size);

	for(i=0;i<size;i++){
		d = drand48();

		if(d>0.5){
			f->value[i] = 1;
		} else {
			f->value[i] = -1;
		}
	}

	f->halo_count = (int *) malloc(sizeof(int) * host.np);
	
	for(i=0;i<host.np;i++)
		f->halo_count[i]=0;

	//determine how many neighbours are on each process

	for(i=0;i<a->x_local;i++){
		// j=3 for trivalent - need to generalise
		for(j=0;j<3;j++){
			if(a->neighbour[i][j] < host.rank*a->x_local || a->neighbour[i][j] >= (host.rank+1)*a->x_local)
				f->halo_count[a->neighbour[i][j]/a->x_local]++;
		}
	}
	
	// malloc halo space

	f->halo = (int **) malloc(sizeof(int *) * host.np);
	for(i=0;i<host.np;i++){
		if(f->halo_count[i] != 0){
			f->halo[i] = (int *) malloc(sizeof(int) * f->halo_count[i]);
		} else {
			f->halo[i] = NULL;
		}
	}

	for(i=0;i<host.np;i++){
		for(j=0;j<f->halo_count[i];j++){
			f->halo[i][j]=0;
		}
	}	

	return f;
}

void free_field(Field* f){

//	pprintf("in free field\n");

	int i;

	for(i=0;i<host.np;i++)
		free(f->halo[i]);

	free(f->halo);

	free(f->value);

	free(f->halo_count);

	free(f);
}

