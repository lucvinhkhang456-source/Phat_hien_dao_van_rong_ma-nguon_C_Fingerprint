#include <iostream>

using namespace std;

int findLargest(int numbers[], int size)
{
    int largest = numbers[0];

    for (int i = 1; i < size; i++)
    {
        if (numbers[i] > largest)
        {
            largest = numbers[i];
        }
    }

    return largest;
}

int main()
{
    int numbers[] = {12, 45, 7, 23, 31};

    int size = 5;

    int result = findLargest(numbers, size);

    cout << "So lon nhat: " << result;

    return 0;
}