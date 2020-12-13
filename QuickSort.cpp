#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void quicksort(int* number, int first, int last) {
	int i, j, pivot, temp;

	if (first < last) {
		pivot = first;
		i = first;
		j = last;

		while (i < j) {
			while (number[i] <= number[pivot] && i < last)
				i++;
			while (number[j] > number[pivot])
				j--;
			if (i < j) {
				temp = number[i];
				number[i] = number[j];
				number[j] = temp;
			}
		}

		temp = number[pivot];
		number[pivot] = number[j];
		number[j] = temp;
		quicksort(number, first, j - 1);
		quicksort(number, j + 1, last);

	}
}

int main(int argc, char** argv) {

	// Initialize the MPI environment
	MPI_Init(NULL, NULL);

	// Get the number of processes
	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	// Get the rank of the process
	int world_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	//FILE* file1;
	//file1 = fopen("bamm.txt", "r");
	int range = 1000000000;
	int nrElemSir = 20000000;
	//fscanf(file1, "%d", &nrElemSir);

	int* sir_elemente = NULL;
	int* sir_pivot = (int*)malloc(world_size * sizeof(int));
	int* ct = (int*)malloc(world_size * sizeof(int));
	int* sir_nou = NULL;
	int nr_elem_per_sir = 0;
	int* displ = NULL;   //gatherv
	double time = 0;
	if (world_rank == 0)
	{
		sir_elemente = (int*)malloc(nrElemSir * sizeof(int));
		sir_nou = (int*)malloc(nrElemSir * sizeof(int));
		for (int i = 0; i < nrElemSir; i++)
		{
			//fscanf(file1, "%d ", &sir_elemente[i]);
			sir_elemente[i] = (rand() * rand()) % range + 1;
		}
		time = MPI_Wtime();

		for (int i = 0; i < world_size; i++)
		{
			if (i == world_size - 1) {
				sir_pivot[i] = range;
			}
			else {
				sir_pivot[i] = range / world_size * (i + 1);
			}
		}

		int contor = 0;
		for (int pivot = 0; pivot < world_size; pivot++)  //fiecare pivot
		{
			int contor_local = 0;
			for (int i = 0; i < nrElemSir; i++)
			{
				if ((sir_elemente[i] < sir_pivot[pivot]) && (sir_elemente[i] != -1))
				{
					sir_nou[contor] = sir_elemente[i];
					contor_local++;
					contor++;
					sir_elemente[i] = -1;
				}
			}
			ct[pivot] = contor_local;
		}
	}

	MPI_Scatter(ct, 1, MPI_INT, &nr_elem_per_sir, 1, MPI_INT, 0, MPI_COMM_WORLD);
	int* subsir = (int*)malloc(nr_elem_per_sir * sizeof(int));
	if (world_rank == 0)
	{
		displ = (int*)malloc(world_size * sizeof(int));
		displ[0] = 0;
		for (int i = 1; i < world_size; i++)
		{
			displ[i] = ct[i - 1] + displ[i - 1];
		}

		for (int i = 0; i < nr_elem_per_sir; i++)
		{
			subsir[i] = sir_nou[i];
		}
		int iterator = ct[0];

		for (int i = 1; i < world_size; i++)
		{
			MPI_Send(&sir_nou[iterator], ct[i], MPI_INT, i, 0, MPI_COMM_WORLD);
			iterator += ct[i];
		}
	}
	else
	{
		MPI_Recv(subsir, nr_elem_per_sir, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
	}

	quicksort(subsir, 0, nr_elem_per_sir - 1);

	MPI_Gatherv(subsir, nr_elem_per_sir, MPI_INT, sir_elemente, ct, displ, MPI_INT, 0, MPI_COMM_WORLD);
	FILE* file2 = fopen("sirOrdonat.txt", "w");
	if (world_rank == 0)
	{
		time = MPI_Wtime()-time;
		printf("timp: %f",time);
		for (int i = 0; i < nrElemSir; i++)
		{
			if ((i % 15 == 0) && (i != 0))
			{
				fprintf(file2, "\n");
			}
			fprintf(file2, "%d ", sir_elemente[i]);
		}
	}




	MPI_Finalize();
}