#include<stdlib.h>
#include<stdio.h>
#include "machine.h" 
#include "array.h" 
#include "field.h"
#include "comms.h" 

// =======================================
// Only use this structure inside this file 


BoundaryComm* init_comm(Array* a){

	int i, j;
	BoundaryComm *c = malloc(sizeof(BoundaryComm));

	c->send_count = (int *) malloc(sizeof(int) * host.np);
	c->recv_count = (int *) malloc(sizeof(int) * host.np);

	c->send = (MPI_Request *) malloc(sizeof(MPI_Request) * (host.np-1));
	c->recv = (MPI_Request *) malloc(sizeof(MPI_Request) * (host.np-1));

	c->send_status = (MPI_Status *) malloc(sizeof(MPI_Status) * (host.np-1));
	c->recv_status = (MPI_Status *) malloc(sizeof(MPI_Status) * (host.np-1));

	MPI_Request * send = (MPI_Request *) malloc(sizeof(MPI_Request) * (host.np-1));
	MPI_Request * recv = (MPI_Request *) malloc(sizeof(MPI_Request) * (host.np-1));

	MPI_Status * send_status = (MPI_Status *) malloc(sizeof(MPI_Status) * (host.np-1));
	MPI_Status * recv_status = (MPI_Status *) malloc(sizeof(MPI_Status) * (host.np-1));

	MPI_Request * send_ptr = send;
	MPI_Request * recv_ptr = recv; 

	for(i=0;i<host.np;i++)
		c->send_count[i]=0;
	for(i=0;i<host.np;i++)
		c->recv_count[i]=0;

	//determine how many to be sent to each process

	for(i=0;i<a->x_local;i++){
		// j=3 for trivalent - need to generalise
		for(j=0;j<3;j++){
			if(a->neighbour[i][j] < host.rank*a->x_local || a->neighbour[i][j] >= (host.rank+1)*a->x_local){
				c->send_count[a->neighbour[i][j]/a->x_local]++;
			}
		}
	}

	c->buffer_send = (int **) malloc(sizeof(int *) * host.np);

	for(i=0;i<host.np;i++){
		if(c->send_count[i] != 0){
			c->buffer_send[i] = (int *) malloc(sizeof(int) * c->send_count[i]);
		} else {
			c->buffer_send[i] = (int *) malloc(sizeof(int) * 1);
		}
	}

	for(i=0;i<host.np;i++){
		if(host.rank != i){
			MPI_Isend(&c->send_count[i], 1, MPI_INT, i, 100*host.rank, MPI_COMM_WORLD, send_ptr);
			send_ptr++;
			MPI_Irecv(&c->recv_count[i], 1, MPI_INT, i, 100*i, MPI_COMM_WORLD, recv_ptr);
			recv_ptr++;
		}
	}

	MPI_Waitall(host.np-1, send, send_status);
	MPI_Waitall(host.np-1, recv, recv_status);

	c->buffer_recv = (int **) malloc(sizeof(int *) * host.np);

	for(i=0;i<host.np;i++){
		if(c->recv_count[i] != 0){
			c->buffer_recv[i] = (int *) malloc(sizeof(int) * c->recv_count[i]);
		} else {
			c->buffer_recv[i] = (int *) malloc(sizeof(int) * 1);
		}
	}

	free(send);
	free(send_status);
	free(recv);
	free(recv_status);

	return c; 
}


void free_comm(BoundaryComm* c){

	int i;

	for(i=0;i<host.np;i++){
		free(c->buffer_send[i]);
		free(c->buffer_recv[i]);
	}

	free(c->send);
	free(c->send_status);

	free(c->recv);
	free(c->recv_status);

	free(c->buffer_send);
	free(c->buffer_recv);

	free(c->send_count);
	free(c->recv_count);

	free(c);

	//pprintf("fe\n");
}

// Only visible inside this file
void send(Field* f, Array* a, BoundaryComm* c){

	int i, j, *k;

	k = (int *) malloc(sizeof(int) * host.np);

	for(i=0;i<host.np;i++) k[i]=0;

	MPI_Request * send_ptr = c->send;
	MPI_Request * recv_ptr = c->recv;

	// copy to buffer
	for(i=0;i<a->x_local;i++){
		for(j=0;j<3;j++){
			if(a->neighbour[i][j] < host.rank*a->x_local || a->neighbour[i][j] >= (host.rank+1)*a->x_local){
				c->buffer_send[a->neighbour[i][j]/a->x_local][k[a->neighbour[i][j]/a->x_local]] = f->value[i];
				k[a->neighbour[i][j]/a->x_local]++;
			}
		}
	}

	for(i=0;i<host.np;i++){
		if(host.rank !=i){
			MPI_Isend(c->buffer_send[i], c->send_count[i], MPI_INT, i, 100*host.rank, MPI_COMM_WORLD, send_ptr);
			send_ptr++;
			MPI_Irecv(c->buffer_recv[i], c->recv_count[i], MPI_INT, i, 100*i, MPI_COMM_WORLD, recv_ptr);
			recv_ptr++;
		}
	}

	free(k);

	MPI_Waitall(host.np-1, c->send, c->send_status);
	MPI_Waitall(host.np-1, c->recv, c->recv_status);

}



void unpack(Field* f, Array* a, BoundaryComm* c){

	int i, j;

	MPI_Barrier(MPI_COMM_WORLD);

	for(i=0;i<host.np;i++){
		for(j=0;j<c->recv_count[i];j++){
				f->halo[i][j] = c->buffer_recv[i][j];
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);
}






// Can be called from outside file


void send_boundary_data(Field* f, Array* a, BoundaryComm * comm){

	

	printf("1\n");

	send(f, a, comm);
//printf("2\n");

	unpack(f, a, comm);
//printf("3\n");
	
//printf("4\n");
}
