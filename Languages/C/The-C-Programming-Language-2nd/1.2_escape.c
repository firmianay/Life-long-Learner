/*
 * \a   alert(bell) character
 * \b   backspace
 * \f   formfeed
 * \n   newline
 * \r   carriage return
 * \t   horizontal tab
 * \v   vertical tab
 * \\   backslash
 * \?   question mark
 * \'   single quote
 * \"   double quote
 * \ooo octal number
 * \xhh hexadecimal number
 */

#include <stdio.h>

int main(void) {
    printf ("alert(bell) character  BEL   \\a   %d\n", '\a');
    printf ("ABCD\aEFGH\n");
    printf ("backspace              BS    \\b   %d\n", '\b');
    printf ("ABCD\bEFGH\n");
    printf ("formfeed               FF    \\f   %d\n", '\f');
    printf ("ABCD\fEFGH\n");
    printf ("newline                LF    \\n   %d\n", '\n');
    printf ("ABCD\nEFGH\n");
    printf ("carriage return        CR    \\r   %d\n", '\r');
    printf ("abcdABCD\rEFGH\n");
    printf ("horizontal tab         HT    \\t   %d\n", '\t');
    printf ("ABCD\tEFGH\n");
    printf ("vertical tab           VT    \\v   %d\n", '\v');
    printf ("ABCD\vEFGH\n");
    printf ("backslash              \\     \\\\   %d\n", '\\');
    printf ("ABCD\\EFGH\n");
    printf ("question mark          \?     \\?   %d\n", '\?');
    printf ("ABCD\?EFGH\n");
    printf ("single quote           \'     \\\'   %d\n", '\'');
    printf ("ABCD\'EFGH\n");
    printf ("double quote           \"     \\\"   %d\n", '\"');
    printf ("ABCD\"EFGH\n");
    printf ("octal number           0%o   \\52  %d\n", '\52', '\52');
    printf ("hexadecimal number     0x%X  \\x2A %d\n", '\x2A', '\x2A');

    return 0;
}

/*
$ ./a.out
alert(bell) character  BEL   \a   7
ABCDEFGH
backspace              BS    \b   8
ABCEFGH
formfeed               FF    \f   12
ABCD
    EFGH
newline                LF    \n   10
ABCD
EFGH
carriage return        CR    \r   13
EFGHABCD
horizontal tab         HT    \t   9
ABCD    EFGH
vertical tab           VT    \v   11
ABCD
    EFGH
backslash              \     \\   92
ABCD\EFGH
question mark          ?     \?   63
ABCD?EFGH
single quote           '     \'   39
ABCD'EFGH
double quote           "     \"   34
ABCD"EFGH
octal number           052   \52  42
hexadecimal number     0x2A  \x2A 42
*/
