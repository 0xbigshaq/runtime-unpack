#include<stdio.h>

int func(int a, int b)  // function definition
{
    printf("a = %d \n", a);
    printf("b = %d \n", b);
}

int main()
{
    printf("\n\n\t\t Function pointers test \n");

    // function pointer
    int(*fptr)(int , int);

    // assign address to function pointer
    fptr = func;

    // function calling
    func(2,3);
    fptr(2,3);  // calling a function referring to pointer to a function

    printf("\nExiting..\n");
    return 0;
}