#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<getopt.h>
#include<time.h>
#include<ctype.h>
#include<mpi.h>
#include "machine.h"
#include "array.h"


void reverse_engineer(int ** in, int ** out, int num_nodes);
void swap_alg(int num_nodes, int num_swaps, int **a, int **b);

Machine host;

int main(int argc, char *argv[]){

	srand(time(NULL));
	int i, num_nodes, num_swaps, size_local;
	double time_taken;
	Array * a_loc;
	clock_t begin, end;

	MPI_Init(&argc,&argv);



	num_nodes = atoi(argv[1]);
	num_swaps = atoi(argv[2]);



	init_machine(argc, argv, num_nodes);


	// assign mem
	int ** a = (int **)malloc(sizeof(int *)*num_nodes);
	int ** b = (int **)malloc(sizeof(int *)*num_nodes);

	for(i=0;i<num_nodes;i++){
		a[i] = (int *)malloc(sizeof(int)*3);
		b[i] = (int *)malloc(sizeof(int)*3);
	}



	// assign neighbours:
	// simply give node the corresponding node in the other set
	// and the next two as neighbours

	begin = MPI_Wtime(); // begin timing

	for(i=0;i<num_nodes;i++){
		a[i][0]=i;
		a[i][1]=(i+1)%num_nodes;
		a[i][2]=(i+2)%num_nodes;
	}

	// get set b's neighbours
	reverse_engineer(a, b, num_nodes);

	size_local = num_nodes/host.np;

	a_loc = init_array(size_local, a);

	// print initial config
	if(num_nodes<10){
		printf("Initial config\nSet A:\n");
		for(i=0;i<num_nodes;i++)
			printf("%d: %d %d %d\n", i, a[i][0], a[i][1], a[i][2]);

		printf("Set B:\n");
		for(i=0;i<num_nodes;i++)
			printf("%d: %d %d %d\n", i, b[i][0], b[i][1], b[i][2]);
	}

	

	swap_alg(num_nodes, num_swaps, a, b);
	
	// end timing
	end = MPI_Wtime();

	// calculate time


	time_taken = (double)(end - begin);

	// print final config

	if(num_nodes<10){
		printf("Final config after %d swaps\nSet A:\n", num_swaps);
		for(i=0;i<num_nodes;i++)
			pprintf("%d: %d %d %d\n", i, a[i][0], a[i][1], a[i][2]);

		printf("Set B:\n");
		for(i=0;i<num_nodes;i++)
			printf("%d: %d %d %d\n", i, b[i][0], b[i][1], b[i][2]);
	}

	pprintf("Time to execute: %f\n", time_taken);

	// clean up time
	for(i=0;i<num_nodes;i++){
		free(a[i]);
		free(b[i]);
	}

	free(a);
	free(b);

	MPI_Finalize();

	return 0;
}


void swap_alg(int num_nodes, int num_swaps, Array * a, Array * b){

	int flag, flag2;
	int i, count;
	int a_first, index_n1, index_n2;
	int index1, index2, index3, index4;
	int b_index1, b_index2;
	int a_point1, a_point2;
	int b_point1, b_point2;

	int rank1;

// do required no of swaps
	for(i=0;i<num_swaps;i++){

		// do initial selection on rank 0
		if(host.rank == 0){
			a_first = (rand())%num_nodes; // first point in a picked at random

			rank1 = a_first/host.np;	
		}

		do {
			count=0;
			flag2=0;

			index_n1 = rand()%3; // index of first neighbour
			index_n2 = rand()%3; // index of sencond neighbour
			while(index_n1 == index_n2)
				index_n2 = rand()%3; // ensure indices don't match

			// find points in set b connected to point 'index' in a
			b_point1 = a[a_first][index_n1]; // point1 in b
			b_point2 = a[a_first][index_n2]; // point2 in b


			index1=0;
			index2=0;

			// find correct indices
			while(b[b_point1][index1] != a_first)
				index1++;

			while(b[b_point2][index2] != a_first)
				index2++;

			do{
				flag=0;
				count++;
				b_index1 = rand()%3; // neighbour index for first point in b to get point in a
				b_index2 = rand()%3; // same for second point

				// ensure that we don't pick a_first
				while(b_index1 == index1)
					b_index1 = rand()%3;

				while(b_index2 == index2)
					b_index2 = rand()%3;


				if(b[b_point1][b_index1] == b[b_point2][b_index2]) // both are same point in a
					flag=1;    

				if(b[b_point1][b_index1] == b[b_point2][3-(index2+b_index2)]) // point1 is same as a neighbour of 2
					flag=1;

				if(b[b_point2][b_index2] == b[b_point1][3-(index1+b_index1)])
					flag=1;

				// ensure that it stops and starts again if no valid neighbour is found
				if(count>20){
					flag2=1;
					break;
				}

			} while(flag==1);
		} while (flag2==1);

		// set the final points chosen in a
		a_point1 = b[b_point1][b_index1];
		a_point2 = b[b_point2][b_index2];


		index3=0;
		index4=0;

		// find correct indices
		while(a[a_point1][index3] != b_point1)
			index3++;

		while(a[a_point2][index4] != b_point2)
			index4++;

		// swap the points
		a[a_point1][index3] = b_point2;
		a[a_point2][index4] = b_point1;

		b[b_point2][b_index2] = a_point1;
		b[b_point1][b_index1] = a_point2;

	}
}

void reverse_engineer(int ** in, int ** out, int num_nodes){

	int i, j, index, k;

	for(i=0;i<num_nodes;i++){
		for(j=0;j<3;j++){
			out[i][j]=-1;
		}
	}

	for(i=0;i<num_nodes;i++){
		for(j=0;j<3;j++){
			index=in[i][j];
			k=0;
			while(out[index][k]!=-1){
				k++;
			}
			out[index][k]=i;
		}
	}
}
