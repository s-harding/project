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
		if(d>0.5) f->value[i] = 1;
		else f->value[i] = -1;
	}
	//for(i=0;i<size;i++)
		//f->value[i]=host.rank;



	f->halo_count = (int *) malloc(sizeof(int) * host.np);
	
	for(i=0;i<host.np;i++) f->halo_count[i]=0;

	//for(i=0;i<host.np;i++) pprintf("halo count %d from proc %d\n", f->halo_count[i], i);

	//determine how many neighbours are on each process

	for(i=0;i<a->x_local;i++){
		// j=3 for trivalent - need to generalise
		for(j=0;j<3;j++){
			if(a->neighbour[i][j] < host.rank*a->x_local || a->neighbour[i][j] >= (host.rank+1)*a->x_local)
				f->halo_count[a->neighbour[i][j]/a->x_local]++;
		}
	}

	

	f->halo = (int **) malloc(sizeof(int *) * host.np);
	for(i=0;i<host.np;i++){
		f->halo[i] = (int *) malloc(sizeof(int) * f->halo_count[i]);
	}


	return f;
}

void free_field(Field* f){

	free(f->halo);
	free(f->value);
	free(f);
}

/*
void write_field(char* filestub, Field *f, Array *a){
	int x,y;
	FILE* o;
	char *filename = malloc(sizeof(char) * (strlen(filestub)+10));

	sprintf(filename,"%s.%.3d",filestub,host.rank);
	o = fopen(filename,"w");

	// Write the active part, with offsets. Don't write process identities
	for (x=0;x<a->x_local;x++)
		for (y=0;y<a->x_local;y++)
			fprintf(o,"%d %d %d\n",x+a->x_offset,y+a->x_offset,f->value[x][y]);

	fclose(o);
	free(filename);
}*/
