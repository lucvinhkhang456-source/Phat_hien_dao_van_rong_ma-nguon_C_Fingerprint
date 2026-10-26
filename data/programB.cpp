#include <iostream>

using namespace std;

int addNumbers(int x, int y)
{
    int total = x + y;

    return total;
}

int main()
{
    int first = 10;
    int second = 20;

    cout << addNumbers(first, second);

    return 0;
}