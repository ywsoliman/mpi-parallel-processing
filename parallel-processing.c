#include <stdio.h>
#include <string.h>
#include "mpi.h"


int main(int argc , char * argv[])
{
    int id[50];
    int grade[50];
    int my_id[20]; //temp for each process
    int my_grade[20]; //temp for each process
    FILE *file;
    int my_rank;		/* rank of process	*/
	int p;			/* number of process	*/
	int students_per_process;
    int n_students_recieved;
    MPI_Status status;

    /* Start up MPI */
	MPI_Init( &argc , &argv );

	/* Find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	/* Find out number of process */
	MPI_Comm_size(MPI_COMM_WORLD, &p);

    if (my_rank == 0)
    {

        int studentIndex = 0;
        //Reading from file
        if (file = fopen("/shared/students.txt", "r"))
        {
            while (fscanf(file, "%d %d", &id[studentIndex], &grade[studentIndex]) != EOF)
                studentIndex++;
            fclose(file);
        }

		students_per_process = studentIndex / (p-1);
        int index, i;

		// check if more than 1 processes are run
		if (p > 1)
        {
            int f;
			for (i = 0; i < p - 2; i++)
			{
			    f = i+1;
				index = i * students_per_process;

				MPI_Send(&students_per_process,
						1, MPI_INT, f, 0,
						MPI_COMM_WORLD);
				MPI_Send(&id[index],
						students_per_process,
						MPI_INT, f, 0,
						MPI_COMM_WORLD);
				MPI_Send(&grade[index],
						students_per_process,
						MPI_INT, f, 0,
						MPI_COMM_WORLD);

			}
            f++;
			// last process adds remaining sudents
			index = i * students_per_process;
			int elements_left = studentIndex - index;

			MPI_Send(&elements_left,
					1, MPI_INT,
					f, 0,
					MPI_COMM_WORLD);
			MPI_Send(&id[index],
					elements_left,
					MPI_INT, f, 0,
					MPI_COMM_WORLD);
			MPI_Send(&grade[index],
					elements_left,
					MPI_INT, f, 0,
					MPI_COMM_WORLD);
		}

		int passed = 0;

		// collects passed students from processes
		int tmp;
		for (i = 1; i < p; i++)
        {
			MPI_Recv(&tmp, 1, MPI_INT,
					MPI_ANY_SOURCE, 0,
					MPI_COMM_WORLD,
					&status);
			int sender = status.MPI_SOURCE;

			passed += tmp;
		}

		//print number of passed students
		printf("Total number of students passed the exam is %d out of %d\n", passed, studentIndex);
	}
	// slave processes
	else
    {
        int i;
		MPI_Recv(&n_students_recieved,
				1, MPI_INT, 0, 0,
				MPI_COMM_WORLD,
				&status);

		MPI_Recv(&my_id, n_students_recieved,
				MPI_INT, 0, 0,
				MPI_COMM_WORLD,
				&status);
		MPI_Recv(&my_grade, n_students_recieved,
				MPI_INT, 0, 0,
				MPI_COMM_WORLD,
				&status);

		// calculates its passed Sudents
		int passedSudents = 0;
        for (i = 0; i < n_students_recieved; i++)
        {
            if(my_grade[i]>60)
            {
                printf("%d  Passed The Exam \n", my_id[i]);
                passedSudents++;
            }
            else
            {
                printf("%d   Failed. Please Repeat The Exam \n", my_id[i]);
            }
        }

		// sends the number of passed Sudents to the root process
		MPI_Send(&passedSudents, 1, MPI_INT,
				0, 0, MPI_COMM_WORLD);
	}

	// cleans up all MPI state before exit of process
	MPI_Finalize();

    return 0;
}
