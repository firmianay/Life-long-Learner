#include <stdio.h>

int main(void) {
    printf ("Enter a char:\n");
    printf ("getchar() != EOF : %d\n", getchar() != EOF);

    return 0;
}

/*
$ ./a.out
Enter a char:
a
getchar() != EOF : 1
*/
