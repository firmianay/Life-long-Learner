#include <stdio.h>

int main(void) {
	int c;

	while ((c = getchar()) != EOF) {
		if (c == '\t')
			printf ("\\t");
		else if (c == '\b')
			printf ("\\b");
		else if (c == '\\')
			printf ("\\\\");
		else
			putchar (c);
	}

	return 0;
}

/*
$ ./a.out 
asdf            asdf \ asdf
asdf\t\tasdf \\ asdf
*/
