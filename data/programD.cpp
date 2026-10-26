#include <iostream>

using namespace std;

bool isPrime(int number)
{
    if (number < 2)
    {
        return false;
    }

    for (int i = 2; i * i <= number; i++)
    {
        if (number % i == 0)
        {
            return false;
        }
    }

    return true;
}

int main()
{
    int number = 29;

    if (isPrime(number))
    {
        cout << "La so nguyen to";
    }
    else
    {
        cout << "Khong phai so nguyen to";
    }

    return 0;
}