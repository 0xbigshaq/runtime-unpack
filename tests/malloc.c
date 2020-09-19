#include <stdio.h>
#include <stdlib.h>

int main()
{
	printf("allocating...\n");
	char *x = malloc(40);
	printf("allocated 40 bytes at: %p\n", x);


	return 0;
}
