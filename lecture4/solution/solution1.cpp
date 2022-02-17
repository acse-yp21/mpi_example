#include <mpi.h>
#include <iostream>
#include <cstdlib>
#include <time.h>
#include <cmath>
#include <vector>

using namespace std;


class CPoints
{
public:
	CPoints()
	{
		next = nullptr;
		prev = nullptr;

		vx = 0.0;
		vy = 0.0;
		x = 0.0;
		y = 0.0;
	}

	void set_random_location();

	double x, y;
	double vx, vy;


	CPoints* next, * prev;

	static double x_max, y_max;

	static MPI_Datatype MPI_Type;
	static void CreateType();
};

void CPoints::set_random_location()
{
	x = (double)rand() / RAND_MAX * x_max;
	y = (double)rand() / RAND_MAX * y_max;
}

void CPoints::CreateType()
{
	int block_lengths[4];
	MPI_Aint displacements[4];
	MPI_Aint addresses[4], add_start;
	MPI_Datatype typelist[4];

	CPoints temp;

	typelist[0] = MPI_DOUBLE;
	block_lengths[0] = 1;
	MPI_Get_address(&temp.x, &addresses[0]);
	typelist[1] = MPI_DOUBLE;
	block_lengths[1] = 1;
	MPI_Get_address(&temp.y, &addresses[1]);
	typelist[2] = MPI_DOUBLE;
	block_lengths[2] = 1;
	MPI_Get_address(&temp.vx, &addresses[2]);
	typelist[3] = MPI_DOUBLE;
	block_lengths[3] = 1;
	MPI_Get_address(&temp.vy, &addresses[3]);


	MPI_Get_address(&temp, &add_start);
	for (int i = 0; i < 4; i++) displacements[i] = addresses[i] - add_start;

	MPI_Type_create_struct(4, block_lengths, displacements, typelist, &MPI_Type);
	MPI_Type_commit(&MPI_Type);
}





int id, p;
int tag_num = 1;


CPoints* Point_Start = nullptr;
const int num_items = 10000;
double CPoints::x_max = 10.0, CPoints::y_max = 10.0;
MPI_Datatype CPoints::MPI_Type;

double width;



int main(int argc, char* argv[])
{
	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	srand(time(NULL) + id * 1257);

	CPoints::CreateType();
	width = CPoints::x_max / p;

	//add item to linkedlist
	if(id == 0)
	{
		vector <MPI_Request> request_list(num_items+p);

		// create linked list
		for (int i = 0; i < num_items; i++)
		{
			CPoints* new_item = new CPoints;
			new_item->set_random_location();

			if (Point_Start != nullptr)
				Point_Start->prev = new_item;
			new_item->next = Point_Start;

			Point_Start = new_item; 
		}


		CPoints* current = Point_Start;
		CPoints* Send_list = nullptr;

		int cnt = 0;
		/*cout << "Items: " <<endl;
		while (current != nullptr)
		{
			cout << current->x << "\t" << current->y << endl;
			current = current->next;
		}*/

		while(current != nullptr)
		{
			CPoints* temp = current->next;

			int proc_dest = (int) (current->x/width);

			if(proc_dest == p) proc_dest = p-1;

			if(proc_dest != id)
			{
				MPI_Isend(current,1, CPoints::MPI_Type,proc_dest,tag_num,MPI_COMM_WORLD,&request_list[cnt]);
				cnt++;
				
				//cut off current item and reconnect
				// if currrent is the begining of the list
				if(current->prev == nullptr)
				{
					Point_Start = current->next;
				}
				else
				{
					current->prev->next = current->next;
				}

				// if currrent is the end of the list
				if(current->next != nullptr)
					current->next->prev = current->prev;


				// add item to the begining of the list
				if(Send_list != nullptr)
				{
					Send_list->prev = current;
				}
				current->prev = nullptr;
				current->next = Send_list;
				Send_list = current;
			}
			current  = temp;
		}

		for(int i=1;i<p;i++)
		{
			MPI_Isend(nullptr,0, CPoints::MPI_Type,i,tag_num,MPI_COMM_WORLD,&request_list[cnt]);
			cnt++;
		}

		MPI_Waitall(cnt,request_list.data(),MPI_STATUSES_IGNORE);

		// free memory
		current = Send_list;
		while (current != nullptr)
		{
			CPoints* temp = current->next;
			delete current;
			current = temp;
		}


		
	}
	else
	{
		vector <MPI_Request> request_list(num_items+p);

		bool flag = true; // flag stop

		do
		{
			CPoints* new_item = new CPoints;
			MPI_Status status;

			MPI_Recv(new_item,1,CPoints::MPI_Type,tag_num,0,MPI_COMM_WORLD,&status);

			int size;
			MPI_Get_count(&status,CPoints::MPI_Type,&size);

			if(size == 0)
			{
				flag = false;
				delete new_item;
			}
			// add to linked list
			else
			{

				if (Point_Start != nullptr)
					Point_Start->prev = new_item;
				new_item->next = Point_Start;

				Point_Start = new_item; 
			}
		}
		while (flag);

	}

	int num_items = 0;
	CPoints* current = Point_Start;
	while (current != nullptr)
	{
		current = current->next;
		num_items++;
	}

	cout << id << " has " << num_items << "assigned to it " << endl;

	// free memory
	current = Point_Start;
	while (current != nullptr)
	{
		CPoints* temp = current->next;
		delete current;
		current = temp;
	}




	MPI_Type_free(&CPoints::MPI_Type);

	MPI_Finalize();
}