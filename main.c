/*
 * TTimer - timer, which prints time to terminal
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
	int secs = 0;
	if(argc < 2 || (secs = atoi(argv[1])) <= 0)
		secs = 60;

	printf("Timer set to: %d sec\n", secs);
	printf("Seconds: ");

	for(int i = 0; i < secs; ++i){
		int n = printf("%d", i);
		fflush(stdout);
		sleep(1);
		while(--n >= 0)
			putchar('\b'); /* backspace printed number*/
	}

	printf("%d\n\a", secs);

	return 0;
}

