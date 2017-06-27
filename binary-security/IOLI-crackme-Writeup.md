# IOLO crackme Writeup

Those basic crackme is [here](https://github.com/Maijin/Workshop2015/tree/master/IOLI-crackme).

## crackme 0x00
Let's run it first.
```
bin-linux$ ./crackme0x00
IOLI Crackme Level 0x00
Password: 123456
Invalid Password!
```

### Method 1
Maybe the password is a strings so use `strings` to find it.
```
bin-linux$ strings crackme0x00
IOLI Crackme Level 0x00
Password:
250382
Invalid Password!
Password OK :)
```
There is a strange number `250382`, try it.
```
bin-linux$ ./crackme0x00
IOLI Crackme Level 0x00
Password: 250382
Password OK :)
```
Or, use `rabin2`, the "binary program info extractor" from radare2.
```
bin-linux$ rabin2 -z ./crackme0x00
vaddr=0x08048568 paddr=0x00000568 ordinal=000 sz=25 len=24 section=.rodata type=ascii string=IOLI Crackme Level 0x00\n
vaddr=0x08048581 paddr=0x00000581 ordinal=001 sz=11 len=10 section=.rodata type=ascii string=Password:
vaddr=0x0804858f paddr=0x0000058f ordinal=002 sz=7 len=6 section=.rodata type=ascii string=250382
vaddr=0x08048596 paddr=0x00000596 ordinal=003 sz=19 len=18 section=.rodata type=ascii string=Invalid Password!\n
vaddr=0x080485a9 paddr=0x000005a9 ordinal=004 sz=16 len=15 section=.rodata type=ascii string=Password OK :)\n
```
> `-z` let you get strings (from data section)


### Method 2
Modify the program logic so that any password can be OK.

I opened the binary with `radare2` using `-w` to be in *write* mode, allowing radare2 to write data to the file.
```assembly_x86
bin-linux$ radare2 -d crackme0x00

[0xf7777b30]> s main
[0x08048414]> pd 32
            ;-- main:
            ;-- main:
            0x08048414      55             push ebp
            0x08048415      89e5           mov ebp, esp
            0x08048417      83ec28         sub esp, 0x28               ; '('
            0x0804841a      83e4f0         and esp, 0xfffffff0
            0x0804841d      b800000000     mov eax, 0
            0x08048422      83c00f         add eax, 0xf
            0x08048425      83c00f         add eax, 0xf
            0x08048428      c1e804         shr eax, 4
            0x0804842b      c1e004         shl eax, 4
            0x0804842e      29c4           sub esp, eax
            0x08048430      c70424688504.  mov dword [esp], str.IOLI_Crackme_Level_0x00_n ; [0x8048568:4]=0x494c4f49 ; "IOLI Crackme Level 0x00." @ 0x8048568
            0x08048437      e804ffffff     call sym.imp.printf
            0x0804843c      c70424818504.  mov dword [esp], str.Password: ; [0x8048581:4]=0x73736150 ; "Password: " @ 0x8048581
            0x08048443      e8f8feffff     call sym.imp.printf
            0x08048448      8d45e8         lea eax, dword [ebp - 0x18]
            0x0804844b      89442404       mov dword [esp + 4], eax
            0x0804844f      c704248c8504.  mov dword [esp], 0x804858c  ; [0x804858c:4]=0x32007325
            0x08048456      e8d5feffff     call sym.imp.scanf
            0x0804845b      8d45e8         lea eax, dword [ebp - 0x18]
            0x0804845e      c74424048f85.  mov dword [esp + 4], str.250382 ; [0x804858f:4]=0x33303532 ; "250382" @ 0x804858f
            0x08048466      890424         mov dword [esp], eax
            0x08048469      e8e2feffff     call sym.imp.strcmp
            0x0804846e      85c0           test eax, eax
        ,=< 0x08048470      740e           je 0x8048480
        |   0x08048472      c70424968504.  mov dword [esp], str.Invalid_Password__n ; [0x8048596:4]=0x61766e49 ; "Invalid Password!." @ 0x8048596
        |   0x08048479      e8c2feffff     call sym.imp.printf
       ,==< 0x0804847e      eb0c           jmp 0x804848c
       |`-> 0x08048480      c70424a98504.  mov dword [esp], str.Password_OK_:__n ; [0x80485a9:4]=0x73736150 ; "Password OK :)." @ 0x80485a9
       |    0x08048487      e8b4feffff     call sym.imp.printf
       `--> 0x0804848c      b800000000     mov eax, 0
            0x08048491      c9             leave
            0x08048492      c3             ret
```
>`s` lets you seek to an address (or symbol)
>
>`pd #` lets you print disassembly of # instructions (from current seek)

Above is the disassembly output of the `main` function. I found the function at `0x08048470` is a conditionally jump `je`, so we change it to `jmp`, which is an unconditionally jump.
```assembly_x86
[0x08048414]> s 0x08048470
[0x08048470]> wx eb
[0x08048470]> pd 9
        ,=< 0x08048470      eb0e           jmp 0x8048480
        |   0x08048472      c70424968504.  mov dword [esp], str.Invalid_Password__n ; [0x8048596:4]=0x61766e49 ; "Invalid Password!." @ 0x8048596
        |   0x08048479      e8c2feffff     call sym.imp.printf
       ,==< 0x0804847e      eb0c           jmp 0x804848c
       |`-> 0x08048480      c70424a98504.  mov dword [esp], str.Password_OK_:__n ; [0x80485a9:4]=0x73736150 ; "Password OK :)." @ 0x80485a9
       |    0x08048487      e8b4feffff     call sym.imp.printf
       `--> 0x0804848c      b800000000     mov eax, 0
            0x08048491      c9             leave
            0x08048492      c3             ret
```
>`wx` is short for Write heX, and allows for writing raw bytes to an offset specificly.

Now we can get successful message whatever the password is.
```
bin-linux$ ./crackme0x00
IOLI Crackme Level 0x00
Password: 123456
Password OK :)
```

## crackme0x01
