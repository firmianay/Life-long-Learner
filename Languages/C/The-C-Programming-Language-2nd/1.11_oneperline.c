#include <stdio.h>

int main(void) {
	int c, preblank;

	preblank = 0;

	while ((c = getchar()) != EOF) {
		if (c == ' ' || c == '\t' || c == '\n') {
			if (!preblank) {
				putchar ('\n');
				preblank = 1;
			}
		} else {
			putchar (c);
			preblank = 0;
		}
	}

	return 0;
}

/*
$ ./a.out 
asdf asdf       asdf
asdf
asdf
asdf
*/
