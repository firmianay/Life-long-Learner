# Noise in stack

Often in this book "noise" or "grabage" values in the stack or memory are mentioned. These are what was left in there after other functions' executions.

A example was compiled by MSVC 2010 and MSVC 2013 in the book. Now we attempt to compile it in GCC to get 32-bit and 64-bit's versions, and then looking for their differences.

```c
// noise_stack.c
#include<stdio.h>

void f1(){
  int a=5, b=6, c=7;
}

void f2() {
  int a, b, c;
  printf("%d %d %d\n", a, b, c);
}

int main(){
  f1();
  f2();
}
```

#### x86-64
```sh
[firmy@MiWiFi-R1CL-srv RE4B]$ gcc noise_stack.c -o v64
[firmy@MiWiFi-R1CL-srv RE4B]$ ./v64
5 6 7
[firmy@MiWiFi-R1CL-srv RE4B]$ objdump -d v64
```
```c
00000000004004f6 <_Z2f1v>:
  4004f6:	55                   	push   %rbp
  4004f7:	48 89 e5             	mov    %rsp,%rbp
  4004fa:	c7 45 fc 05 00 00 00 	movl   $0x5,-0x4(%rbp)
  400501:	c7 45 f8 06 00 00 00 	movl   $0x6,-0x8(%rbp)
  400508:	c7 45 f4 07 00 00 00 	movl   $0x7,-0xc(%rbp)
  40050f:	90                   	nop
  400510:	5d                   	pop    %rbp
  400511:	c3                   	retq   

0000000000400512 <_Z2f2v>:
  400512:	55                   	push   %rbp
  400513:	48 89 e5             	mov    %rsp,%rbp
  400516:	48 83 ec 10          	sub    $0x10,%rsp
  40051a:	8b 4d f4             	mov    -0xc(%rbp),%ecx
  40051d:	8b 55 f8             	mov    -0x8(%rbp),%edx
  400520:	8b 45 fc             	mov    -0x4(%rbp),%eax
  400523:	89 c6                	mov    %eax,%esi
  400525:	bf d4 05 40 00       	mov    $0x4005d4,%edi
  40052a:	b8 00 00 00 00       	mov    $0x0,%eax
  40052f:	e8 bc fe ff ff       	callq  4003f0 <printf@plt>
  400534:	90                   	nop
  400535:	c9                   	leaveq
  400536:	c3                   	retq   

0000000000400537 <main>:
  400537:	55                   	push   %rbp
  400538:	48 89 e5             	mov    %rsp,%rbp
  40053b:	e8 b6 ff ff ff       	callq  4004f6 <_Z2f1v>
  400540:	e8 cd ff ff ff       	callq  400512 <_Z2f2v>
  400545:	b8 00 00 00 00       	mov    $0x0,%eax
  40054a:	5d                   	pop    %rbp
  40054b:	c3                   	retq   
  40054c:	0f 1f 40 00          	nopl   0x0(%rax)
```
We did not set any variables in `f2()`. These are “ghosts” values, which are still in the stack. In x86-64, `a`, `b` and `c` of `f2()` are located at the same addresses!(`-0x4(%rbp) -0x8(%rbp) -0xc(%rbp)`) No one has overwritten the values yet, so at that point they are still untouched.

#### x86
But in x86, we can see something different.
```sh
[firmy@MiWiFi-R1CL-srv RE4B]$ gcc -m32 noise_stack.c -o v32
[firmy@MiWiFi-R1CL-srv RE4B]$ ./v32
7 -145652752 2
[firmy@MiWiFi-R1CL-srv RE4B]$ objdump -d v32
```
```c
0804840b <_Z2f1v>:
 804840b:	55                   	push   %ebp
 804840c:	89 e5                	mov    %esp,%ebp
 804840e:	83 ec 10             	sub    $0x10,%esp
 8048411:	c7 45 fc 05 00 00 00 	movl   $0x5,-0x4(%ebp)
 8048418:	c7 45 f8 06 00 00 00 	movl   $0x6,-0x8(%ebp)
 804841f:	c7 45 f4 07 00 00 00 	movl   $0x7,-0xc(%ebp)
 8048426:	90                   	nop
 8048427:	c9                   	leave  
 8048428:	c3                   	ret    

08048429 <_Z2f2v>:
 8048429:	55                   	push   %ebp
 804842a:	89 e5                	mov    %esp,%ebp
 804842c:	83 ec 18             	sub    $0x18,%esp
 804842f:	ff 75 ec             	pushl  -0x14(%ebp)
 8048432:	ff 75 f0             	pushl  -0x10(%ebp)
 8048435:	ff 75 f4             	pushl  -0xc(%ebp)
 8048438:	68 00 85 04 08       	push   $0x8048500
 804843d:	e8 9e fe ff ff       	call   80482e0 <printf@plt>
 8048442:	83 c4 10             	add    $0x10,%esp
 8048445:	90                   	nop
 8048446:	c9                   	leave  
 8048447:	c3                   	ret    

08048448 <main>:
 8048448:	8d 4c 24 04          	lea    0x4(%esp),%ecx
 804844c:	83 e4 f0             	and    $0xfffffff0,%esp
 804844f:	ff 71 fc             	pushl  -0x4(%ecx)
 8048452:	55                   	push   %ebp
 8048453:	89 e5                	mov    %esp,%ebp
 8048455:	51                   	push   %ecx
 8048456:	83 ec 04             	sub    $0x4,%esp
 8048459:	e8 ad ff ff ff       	call   804840b <_Z2f1v>
 804845e:	e8 c6 ff ff ff       	call   8048429 <_Z2f2v>
 8048463:	b8 00 00 00 00       	mov    $0x0,%eax
 8048468:	83 c4 04             	add    $0x4,%esp
 804846b:	59                   	pop    %ecx
 804846c:	5d                   	pop    %ebp
 804846d:	8d 61 fc             	lea    -0x4(%ecx),%esp
 8048470:	c3                   	ret    
 8048471:	66 90                	xchg   %ax,%ax
 8048473:	66 90                	xchg   %ax,%ax
 8048475:	66 90                	xchg   %ax,%ax
 8048477:	66 90                	xchg   %ax,%ax
 8048479:	66 90                	xchg   %ax,%ax
 804847b:	66 90                	xchg   %ax,%ax
 804847d:	66 90                	xchg   %ax,%ax
 804847f:	90                   	nop
```
`a`, `b` and `c` in `f1()` are different from those in `f2()`. This is because the location between `f1()` and `f2()` are different, in `f1()` there are `-0x4(%ebp) -0x8(%ebp) -0xc(%ebp)`, but in `f2()` there are `-0xc(%ebp) -0x10(%ebp) -0x14(%ebp)`, so `a` in `f2()` is same as `c` in `f1()`, but `b` and `c` in `f2()` are not.

Summarizing, all values in the stack (and memory cells in general) have values left there from previous function executions. They are not random in the strict sense, but rather have unpredictable values.
