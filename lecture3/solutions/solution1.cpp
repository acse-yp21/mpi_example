#include <mpi.h>
#include <iostream>
#include <cstdlib>
#include <time.h>

using namespace std;

int id, p;

void bubble_sort(double* list, int* index, int num)
{
    double temp;
    bool swap;

    do{
        swap = false;

        for(int i=0;i<num-1;i++)
        {
            if(list[i] > list[i+1])
            {
                temp = list[i];
                list[i] = list[i+1];
                list[i+1] = temp;

                temp = index[i];
                index[i] = index[i+1];
                index[i+1] = temp;
                swap = true;
            }
        }
    }while(swap);
}

int main(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	srand(time(NULL) + id * 1257);

	double id_rand = (double)rand() /RAND_MAX *100.0;
    cout << "Process " << id << " produced number " << id_rand << endl;

    double *recv_list = nullptr;
    if(id == 0)
        recv_list = new double[p];
    
    MPI_Gather(&id_rand,1,MPI_DOUBLE,recv_list,1,MPI_DOUBLE,0,MPI_COMM_WORLD);

    int *position_in_list = nullptr;

    if(id == 0)
    {
        cout << "Items on process 0: ";
        for(int i=0;i<p;i++)
            cout << "\t" << recv_list[i];
        cout<<endl;

        int *index_list = new int[p];
        for(int i=0;i<p;i++)
        {
            index_list[i] = i;
        }
        position_in_list = new int[p];

        bubble_sort(recv_list,index_list,p);
        cout << "sorted list and index: ";
        for(int i=0;i<p;i++)
            cout << "\t" << recv_list[i] << "(" << index_list[i] << ")";
        cout<<endl;

        for(int i=0;i<p;i++)
        {
            position_in_list[index_list[i]]=i;
        }


    }

    int id_position;
    MPI_Scatter(position_in_list,1,MPI_INT,&id_position,1,MPI_INT,0,MPI_COMM_WORLD);

    cout << id << " is at location " << id_position << endl;
	MPI_Finalize();
}