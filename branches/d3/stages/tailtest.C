#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
	srand(0);

	while(1)
	{
		int n = (int)(1.0 + (double)rand() * 3.0 / (double)RAND_MAX );
		int i;
		for (i=0; i<n ; i++)
		{ 
			printf("%d, %d, %d, %d\n", rand(), rand(), rand(), rand());
			fflush(stdout);
		}
		sleep((int)( (double)rand() * 5.0 /(double)RAND_MAX) );
	}
}






