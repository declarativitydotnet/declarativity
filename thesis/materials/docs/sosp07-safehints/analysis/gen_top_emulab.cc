#include <stdio.h>
#include <stdlib.h>





int main(int argc, char ** argv){
	int num_clients = 20;
	int num_servers = 46;
	int k = 0;
	for(int i = 0; i < num_clients; i++){
		for(int j = 0; j < num_servers; j++){
			printf("set link%d [ns duplex-link $node%d $node%d 1000Mb 10ms DropTail]\n", k, i, j);
			k++;
		}	


	}
	printf("tb-use-endnodeshaping 1\n");

  return 1;
}
