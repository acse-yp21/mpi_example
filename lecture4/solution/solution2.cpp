#include <mpi.h>
#include <iostream>
#include <cstdlib>
#include <time.h>
#include <cmath>
#include <vector>

using namespace std;


class CMatrix
{
public:
    CMatrix()
    {
        array_1D = nullptr;
        array_2D = nullptr;
        imax = 0;
        jmax = 0;
    }
    CMatrix(int isize, int jsize);

    ~CMatrix()
    {
        delete[] array_2D;
        delete[] array_1D;
    }

    double* array_1D;
    double** array_2D;

    int imax, jmax;

    MPI_Datatype Left_Type, Right_Type, Top_Type, Bottom_Type;

    void CreateArray(int isize, int jsize);
    void FillArray();
    void CreateMPI_Types(void);
};

CMatrix::CMatrix(int isize, int jsize)
{
    CreateArray(isize,jsize);
}

void CMatrix::CreateArray(int isize, int jsize)
{
    imax = isize;
    jmax = jsize;
    array_1D = new double[imax*jmax];
    array_2D = new double* [imax];
    for(int i=0;i<imax;i++)
        array_2D[i] = array_1D[i*jmax]; // map 1d array to 2d array
}


void CMatrix::FillArray()
{
    for(int i=0;i<imax;i++)
        for(int j=0;j<jmax;j++)
            array_2D[i][j] = (double)rand() / RAND_MAX;
}

void CMatrix::CreateMPI_Types()
{
	vector <int> block_lengths;
	vector <MPI_Aint> displacements;
    MPI_Aint add_start;
	vector <MPI_Datatype> typelist;

    block_lengths.resize(imax);
    displacements.resize(jmax);
    typelist.resize(imax);

    MPI_Get_address(this, &add_start);

    // left
    for(int i=0;i<imax;i++)
    {
        typelist[i] = MPI_DOUBLE;
        block_lengths[i] = 1;
        MPI_Get_address(&array_2D[i][0], &displacements[i]);
        displacements[i] -= add_start;

    }

	MPI_Type_create_struct(4, block_lengths.data(), displacements.data(), typelist.data(), &Left_Type);
	MPI_Type_commit(&Left_Type);

    // right
    for(int i=0;i<imax;i++)
    {
        typelist[i] = MPI_DOUBLE;
        block_lengths[i] = 1;
        MPI_Get_address(&array_2D[i][jmax-1], &displacements[i]);
        displacements[i] -= add_start;

    }

	MPI_Type_create_struct(4, block_lengths.data(), displacements.data(), typelist.data(), &Right_Type);
	MPI_Type_commit(&Right_Type);


    // TOP CONTINUOUS
    typelist[0] = MPI_DOUBLE;
    block_lengths[0] = jmax;
    MPI_Get_address(&array_2D[0], &displacements[0]);
    displacements[0] -= add_start;

    MPI_Type_create_struct(4, block_lengths.data(), displacements.data(), typelist.data(), &Top_Type);
	MPI_Type_commit(&Top_Type);

    // Bottom CONTINUOUS
    typelist[0] = MPI_DOUBLE;
    block_lengths[0] = jmax;
    MPI_Get_address(&array_2D[imax-1], &displacements[0]);
    displacements[0] -= add_start;

    MPI_Type_create_struct(4, block_lengths.data(), displacements.data(), typelist.data(), &Bottom_Type);
	MPI_Type_commit(&Bottom_Type);

}


int id, p;
int tag_num = 1;

int main(int argc, char* argv[])
{
	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	srand(time(NULL) + id * 1257);

	CMatrix A(10,10);

    A.FillArray();
    A.CreateMPI_Types();

    if(id == 0)
    {
        MPI_Request *request_list = new MPI_Request[(p-1)*4];

        int cnt = 0;
        for(int i=1;i<p;i++)
        {
            MPI_Isend(&A,1,A.Left_Type,i,tag_num,MPI_COMM_WORLD,&request_list[cnt]);
            cnt++;
            MPI_Isend(&A,1,A.Right_Type,i,tag_num,MPI_COMM_WORLD,&request_list[cnt]);
            cnt++;
            MPI_Isend(&A,1,A.Top_Type,i,tag_num,MPI_COMM_WORLD,&request_list[cnt]);
            cnt++;
            MPI_Isend(&A,1,A.Bottom_Type,i,tag_num,MPI_COMM_WORLD,&request_list[cnt]);
            cnt++;

        }

        MPI_Waitall(cnt,request_list,MPI_STATUSES_IGNORE);
    }
    else
    {
        MPI_Request *request_list = new MPI_Request[4];

        MPI_Irecv(&A,1,A.Left_Type,0,tag_num,MPI_COMM_WORLD,&request_list[cnt]);
        cnt++;
        MPI_Irecv(&A,1,A.Right_Type,0,tag_num,MPI_COMM_WORLD,&request_list[cnt]);
        cnt++;
        MPI_Irecv(&A,1,A.Top_Type,0,tag_num,MPI_COMM_WORLD,&request_list[cnt]);
        cnt++;
        MPI_Irecv(&A,1,A.Bottom_Type,0,tag_num,MPI_COMM_WORLD,&request_list[cnt]);
        cnt++;

        MPI_Waitall(cnt,request_list,MPI_STATUSES_IGNORE);
    }

    cout << id << ": ";
    for(int i=0;i<A.imax;i++)
    {
        for(int j=0;j<A.jmax;j++)
        {
            cout << A.array_2D[i][j] << " ";
        }
        cout << ";";

    }

    cout << endl;
	MPI_Finalize();
}