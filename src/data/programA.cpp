#include <iostream>

using namespace std;

int sum(int a, int b)
{
    int result = a + b;

    return result;
}

int main()
{
    int x = 10;
    int y = 20;

    cout << sum(x, y);

    return 0;
}