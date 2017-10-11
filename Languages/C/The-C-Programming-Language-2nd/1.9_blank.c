#include <stdio.h>

int main(void) {
    int c, preblank;

    preblank = 0;
    while ((c = getchar()) != EOF) {
        if (c == ' ') {
            if (!preblank) {
                preblank = 1;
                putchar (c);
            }
        } else {
            preblank = 0;
            putchar (c);
        }
    }

    return 0;
}

/*
$ ./a.out 
asdf asdf  asdf   asdf          asdf
asdf asdf asdf asdf             asdf
*/
