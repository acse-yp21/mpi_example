#include <iostream>
#include <cstdlib>
#include <time.h>

using namespace std;

double* list;
int num_items = 100;

void quick_sort(int list_start, int list_end, double *list) //list_end inclusive
{
    if(list_end>list_start)
    {
        double pivot = list[list_end]; //first item as pivot
        int num_bottom = list_start;

        for(int i=list_start; i<list_end;i++)
        {
            if(list[i] <= pivot)
            {
                double temp = list[num_bottom];
                list[num_bottom] = list[i];
                list[i] = temp;
                num_bottom++;
            }
        }
        double temp = list[num_bottom];
        list[num_bottom] = list[list_end];
        list[list_end] = temp;

        quick_sort(list_start,num_bottom-1,list);
        quick_sort(num_bottom, list_end,list);

    }

}

int main()
{
    srand(time(NULL));

    list = new double[num_items];

    cout << "Input list: ";
    for(int i=0; i<num_items;i++)
    {
        list[i] = (double) rand()/RAND_MAX *100.0;
        cout << list[i] << " ";
    }
    cout << endl;

    quick_sort(0,num_items-1,list);

    cout << "Output list: ";
    for(int i=0; i<num_items;i++)
    {
        cout << list[i] << " ";
    }
    cout << endl;

}