# Binary Hacks

## 用 od 转储二进制文件

十六进制转存：

```sh
$ od -t x1 -A x /etc/ld.so.cache | head -5
000000 6c 64 2e 73 6f 2d 31 2e 37 2e 30 00 dc 0b 00 00
000010 03 03 00 00 b8 1c 01 00 cc 1c 01 00 03 03 00 00
000020 e9 1c 01 00 00 1d 01 00 03 03 00 00 20 1d 01 00
000030 36 1d 01 00 03 03 00 00 55 1d 01 00 65 1d 01 00
000040 03 03 00 00 7e 1d 01 00 8b 1d 01 00 03 03 00 00
```

显示 ASCII 码：

```sh
$ od -t x1z -A x /etc/ld.so.cache | head -5
000000 6c 64 2e 73 6f 2d 31 2e 37 2e 30 00 dc 0b 00 00  >ld.so-1.7.0.....<
000010 03 03 00 00 b8 1c 01 00 cc 1c 01 00 03 03 00 00  >................<
000020 e9 1c 01 00 00 1d 01 00 03 03 00 00 20 1d 01 00  >............ ...<
000030 36 1d 01 00 03 03 00 00 55 1d 01 00 65 1d 01 00  >6.......U...e...<
000040 03 03 00 00 7e 1d 01 00 8b 1d 01 00 03 03 00 00  >....~...........<
```

换行表示：

```sh
$ od -t x1c -A x /etc/ld.so.cache | head -5
000000  6c  64  2e  73  6f  2d  31  2e  37  2e  30  00  dc  0b  00  00
         l   d   .   s   o   -   1   .   7   .   0  \0 334  \v  \0  \0
000010  03  03  00  00  b8  1c  01  00  cc  1c  01  00  03  03  00  00
       003 003  \0  \0 270 034 001  \0 314 034 001  \0 003 003  \0  \0
000020  e9  1c  01  00  00  1d  01  00  03  03  00  00  20  1d  01  00
```

## 静态链接库和共享库

将多个目标文件归档为一个文件，称为静态链接库。静态链接库的编写通常如下：

```sh
$ cc -c -o foo.o foo.c
$ cc -c -o bar.o bar.c
$ ar rUuv libfoo.a libfoo.a foo.o bar.o
ar: creating libfoo.a
a - foo.o
a - bar.o
```

查看库的内容：

```sh
$ ar tv libfoo.a 
rw-r--r-- 1000/1000   1920 Oct 23 14:18 2017 foo.o
rw-r--r-- 1000/1000   1920 Oct 23 14:18 2017 bar.o
```

链接静态库时，连接器进行一下工作：先从其他目标文件中查找未定义的符号，再从指定的静态链接库中读取定义符号的目标文件的副本，加入到可执行文件中，完成链接。

静态库是多个目标文件的存档，而共享库则把多个目标文件复制成一个巨大的目标文件中进行共享。制作共享库的通常做法：

```sh
$ cc -fPIC -c -o foo.o foo.c
$ cc -fPIC -c -o bar.o bar.c
$ cc -shared -Wl,-soname,libfoo.so.0 -o libfoo.so foo.o bar.o
```

## 通过 ldd 查阅共享库的依赖关系

```sh
$ ldd /bin/ls
        linux-vdso.so.1 (0x00007ffe3d98a000)
        libcap.so.2 => /usr/lib/libcap.so.2 (0x00007f89f1cae000)
        libc.so.6 => /usr/lib/libc.so.6 (0x00007f89f18f7000)
        /lib64/ld-linux-x86-64.so.2 => /usr/lib64/ld-linux-x86-64.so.2 (0x00007f89f20d4000)
```

ldd 实际上仅是 shell 脚本，重点是环境变量 LD_TRACE_LOADED_OBJECTS，把它设为 1，则效果一样：

```sh
$ LD_TRACE_LOADED_OBJECTS=1 /bin/ls
        linux-vdso.so.1 (0x00007ffcfc316000)
        libcap.so.2 => /usr/lib/libcap.so.2 (0x00007f156098f000)
        libc.so.6 => /usr/lib/libc.so.6 (0x00007f15605d8000)
        /lib64/ld-linux-x86-64.so.2 (0x00007f1560b93000)
```

## 在链接 C 程序和 C++ 程序时要注意的问题

分别使用 gcc 和 g++ 对下面的函数进行编译：

```c
// dbg.c
#include<stdio.h>
void dbg(const char *s) {
        printf("Log: %s\n", s);
}
```

生成的符号名如下：

```sh
$ gcc -c -o dbg.o dbg.c
$ nm dbg.o | grep dbg
0000000000000000 T dbg
$ g++ -c -o dbg.o dbg.c
$ nm dbg.o | grep dbg
0000000000000000 T _Z3dbgPKc
```

在 C++ 编译器中，符号会包含函数所属的命名空间的信息和函数的参数类型信息。可以使用 C++filt 进行转储，即逆变换：

```sh
$ nm dbg.o | c++filt | grep dbg
0000000000000000 T dbg(char const*)
```

使用 C 编译器来编译 dbg.c，然后从 C++ 编写的函数中对其进行调用。下面是 sample.cpp：

```cpp
// sample.cpp
extern "C" void dbg(const char *s);
int main() {
        dbg("foo");
        return 0;
}
```

编译运行：

```sh
$ gcc -Wall -c dbg.c
$ g++ -Wall -c sample.cpp
$ g++ -o sample dbg.o sample.o
$ ./sample 
Log: foo
```

如果没有 `extern "C"`，或者 dbg 的类型写错了，都会产生错误。所以，你需要准备好下面这样的文件头：

```c
// dbg.h
#ifdef __cplusplus
  extern "C" {
#endif
  void dbg(const char *s);
#ifdef __cplusplus
  }
#endif
```

下面从 C 中调用 C++ 函数，要注意两点，第一是 C++ 函数要结合 C （extern "C"）进行编译，第二点是在链接时，要运行 g++ 而不是 gcc。

```c
// gcd.h
#ifdef __cplusplus
extern "C"
#endif
int gcd(int v1, int v2);
```

```cpp
// gcd.cpp
#include <boost/math/common_factor.hpp>
#include "gcd.h"

extern "C" {
    int gcd(int v1, int v2) {
        return boost::math::gcd(v1, v2);
    }
}
```

```c
// sample.c
#include <stdio.h>
#include "gcd.h"

int main() {
    printf("gcd(%d, %d) = %d\n", 14, 35, gcd(14, 35));
    return 0;
}
```

```sh
$ g++ -Wall -c gcd.cpp
$ gcc -Wall -c sample.c
$ g++ -o sample sample.o gcd.o
$ ./sample
```

但还有两点要注意，第一点是不允许 C++ 函数向 C 函数抛出异常，第二点是处理函数指针的 C 函数，在编译时加上 `-fexceptions`。

## 链接时的同名标识符冲突

对 `.o` 文件进行整理链接的情况：

```c
// a.c
#include <stdio.h>
void func() {
        printf("func() in a.c\n");
}
```

```c
// b.c
#include <stdio.h>
void func() {
        printf("func() in b.c\n");
}
```

```c
void func();
int main() {
        func();
        return 0;
}
```

分别进行编译并静态链接，因为有多个 `func()`，出现错误：

```sh
$ gcc -c a.c 
$ gcc -c b.c 
$ gcc -c main.c 
$ gcc -o main a.o b.o main.o
b.o: In function `func':
b.c:(.text+0x0): multiple definition of `func'
a.o:a.c:(.text+0x0): first defined here
collect2: error: ld returned 1 exit status
```

将 a.o 和 b.o 合并成一个共享库时发生冲突：

```sh
$ gcc -fPIC -c a.c
$ gcc -fPIC -c b.c
$ gcc -shared -o libfoo.so a.o b.o
b.o: In function `func':
b.c:(.text+0x0): multiple definition of `func'
a.o:a.c:(.text+0x0): first defined here
collect2: error: ld returned 1 exit status
```

生成库并进行链接时的情况：

将 a.o 和 b.o 使用 ar 生成静态链接库并进行链接时不会发生错误，这是因为 ar 不会检查符号冲突：

```sh
$ ar cr libfoo.a a.o b.o
$ gcc main.o libfoo.a 
$ ./a.out 
func() in a.c
$ rm libfoo.a 
$ ar cr libfoo.a b.o a.o
$ gcc main.o libfoo.a 
$ ./a.out 
func() in b.c
```

放在前面的 `.o` 会先被发现，所以会被调用。将 a.c 和 b.c 分别生成静态链接库的情况下，进行链接也不会发生冲突，同样顺序很重要：

```sh
$ ar cr liba.a a.o
$ ar cr libb.a b.o
$ gcc main.o liba.a libb.a 
$ ./a.out 
func() in a.c
```

同样，分别生成动态链接库并链接，也不会发生错误：

```sh
$ gcc -fPIC -shared -o a.so a.c
$ gcc -fPIC -shared -o b.so b.c
$ gcc -fPIC -shared -o main.so main.c
$ gcc -o main-shared a.so b.so main.so 
$ ./a.out 
func() in a.c
```
