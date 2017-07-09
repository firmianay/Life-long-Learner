# IOLI crackme Writeup

Those basic crackmes are [here](https://github.com/Maijin/Workshop2015/tree/master/IOLI-crackme).

## Crackme 0x00
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
```
bin-linux$ radare2 -w crackme0x00

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
```
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

## Crackme0x01
This time, the method 1 below got failed.
```
bin-linux$ ./crackme0x01
IOLI Crackme Level 0x01
Password: 123456
Invalid Password!
bin-linux$ rabin2 -z crackme0x01
vaddr=0x08048528 paddr=0x00000528 ordinal=000 sz=25 len=24 section=.rodata type=ascii string=IOLI Crackme Level 0x01\n
vaddr=0x08048541 paddr=0x00000541 ordinal=001 sz=11 len=10 section=.rodata type=ascii string=Password:
vaddr=0x0804854f paddr=0x0000054f ordinal=002 sz=19 len=18 section=.rodata type=ascii string=Invalid Password!\n
vaddr=0x08048562 paddr=0x00000562 ordinal=003 sz=16 len=15 section=.rodata type=ascii string=Password OK :)\n
```

So, lets see the code again.
```
bin-linux$ radare2 -w crackme0x01
[0x08048330]> aa
[x] Analyze all flags starting with sym. and entry0 (aa)
[0x08048330]> pdf @ sym.main
            ;-- main:
/ (fcn) main 113
|   main ();
|           ; var int local_4h @ ebp-0x4
|           ; var int local_4h_2 @ esp+0x4
|              ; DATA XREF from 0x08048347 (entry0)
|           0x080483e4      55             push ebp
|           0x080483e5      89e5           mov ebp, esp
|           0x080483e7      83ec18         sub esp, 0x18
|           0x080483ea      83e4f0         and esp, 0xfffffff0
|           0x080483ed      b800000000     mov eax, 0
|           0x080483f2      83c00f         add eax, 0xf
|           0x080483f5      83c00f         add eax, 0xf
|           0x080483f8      c1e804         shr eax, 4
|           0x080483fb      c1e004         shl eax, 4
|           0x080483fe      29c4           sub esp, eax
|           0x08048400      c70424288504.  mov dword [esp], str.IOLI_Crackme_Level_0x01_n ; [0x8048528:4]=0x494c4f49 ; "IOLI Crackme Level 0x01." @ 0x8048528
|           0x08048407      e810ffffff     call sym.imp.printf        ; int printf(const char *format)
|           0x0804840c      c70424418504.  mov dword [esp], str.Password: ; [0x8048541:4]=0x73736150 ; "Password: " @ 0x8048541
|           0x08048413      e804ffffff     call sym.imp.printf        ; int printf(const char *format)
|           0x08048418      8d45fc         lea eax, dword [local_4h]
|           0x0804841b      89442404       mov dword [local_4h_2], eax
|           0x0804841f      c704244c8504.  mov dword [esp], 0x804854c  ; [0x804854c:4]=0x49006425 ; "%d"
|           0x08048426      e8e1feffff     call sym.imp.scanf         ; int scanf(const char *format)
|           0x0804842b      817dfc9a1400.  cmp dword [local_4h], 0x149a ; [0x149a:4]=0x2ec0804
|       ,=< 0x08048432      740e           je 0x8048442
|       |   0x08048434      c704244f8504.  mov dword [esp], str.Invalid_Password__n ; [0x804854f:4]=0x61766e49 ; "Invalid Password!." @ 0x804854f
|       |   0x0804843b      e8dcfeffff     call sym.imp.printf        ; int printf(const char *format)
|      ,==< 0x08048440      eb0c           jmp 0x804844e
|      |`-> 0x08048442      c70424628504.  mov dword [esp], str.Password_OK_:__n ; [0x8048562:4]=0x73736150 ; "Password OK :)." @ 0x8048562
|      |    0x08048449      e8cefeffff     call sym.imp.printf        ; int printf(const char *format)
|      |       ; JMP XREF from 0x08048440 (main)
|      `--> 0x0804844e      b800000000     mov eax, 0
|           0x08048453      c9             leave
\           0x08048454      c3             ret
```
> `aa` tells radare2 to analyse the whole binary.
>
> `pdf` is short of Print Disassemble Function

We also found a `je` at `0x08048432` and if we change it to `jmp`, it will be successful. But this time, we try another method to found its password. The function at `0x0804842b` is a `cmp` instruction, with a constant `0x149a`. The `0x149a` is a hexadecimal number and we can change it to others.
```
[0x08048330]> ? 0x149a
5274 0x149a 012232 5.2K 0000:049a 5274 "\x9a\x14" 0001010010011010 5274.0 5274.000000f 5274.000000
```
>The `?` command is used to perform math operations and returns the answer in a wide variety of formats.

Maybe the password is just one of them.
```
bin-linux$ ./crackme0x01
IOLI Crackme Level 0x01
Password: 5274
Password OK :)
```

## Crackme0x02
This time also a "compare-jump" program, but the destination is "Invalid_Password__n" instead "Password_OK_".
```
bin-linux$ radare2 -w crackme0x02
[0x08048330]> aa
[x] Analyze all flags starting with sym. and entry0 (aa)
[0x08048330]> pdf @ sym.main
            ;-- main:
/ (fcn) main 144
|   main ();
|           ; var int local_ch @ ebp-0xc
|           ; var int local_8h @ ebp-0x8
|           ; var int local_4h @ ebp-0x4
|           ; var int local_4h_2 @ esp+0x4
|              ; DATA XREF from 0x08048347 (entry0)
|           0x080483e4      55             push ebp
|           0x080483e5      89e5           mov ebp, esp
|           0x080483e7      83ec18         sub esp, 0x18
|           0x080483ea      83e4f0         and esp, 0xfffffff0
|           0x080483ed      b800000000     mov eax, 0
|           0x080483f2      83c00f         add eax, 0xf
|           0x080483f5      83c00f         add eax, 0xf
|           0x080483f8      c1e804         shr eax, 4
|           0x080483fb      c1e004         shl eax, 4
|           0x080483fe      29c4           sub esp, eax
|           0x08048400      c70424488504.  mov dword [esp], str.IOLI_Crackme_Level_0x02_n ; [0x8048548:4]=0x494c4f49 ; "IOLI Crackme Level 0x02." @ 0x8048548
|           0x08048407      e810ffffff     call sym.imp.printf        ; int printf(const char *format)
|           0x0804840c      c70424618504.  mov dword [esp], str.Password: ; [0x8048561:4]=0x73736150 ; "Password: " @ 0x8048561
|           0x08048413      e804ffffff     call sym.imp.printf        ; int printf(const char *format)
|           0x08048418      8d45fc         lea eax, dword [local_4h]
|           0x0804841b      89442404       mov dword [local_4h_2], eax
|           0x0804841f      c704246c8504.  mov dword [esp], 0x804856c  ; [0x804856c:4]=0x50006425 ; "%d"
|           0x08048426      e8e1feffff     call sym.imp.scanf         ; int scanf(const char *format)
|           0x0804842b      c745f85a0000.  mov dword [local_8h], 0x5a  ; 'Z'
|           0x08048432      c745f4ec0100.  mov dword [local_ch], 0x1ec
|           0x08048439      8b55f4         mov edx, dword [local_ch]
|           0x0804843c      8d45f8         lea eax, dword [local_8h]
|           0x0804843f      0110           add dword [eax], edx
|           0x08048441      8b45f8         mov eax, dword [local_8h]
|           0x08048444      0faf45f8       imul eax, dword [local_8h]
|           0x08048448      8945f4         mov dword [local_ch], eax
|           0x0804844b      8b45fc         mov eax, dword [local_4h]
|           0x0804844e      3b45f4         cmp eax, dword [local_ch]
|       ,=< 0x08048451      750e           jne 0x8048461
|       |   0x08048453      c704246f8504.  mov dword [esp], str.Password_OK_:__n ; [0x804856f:4]=0x73736150 ; "Password OK :)." @ 0x804856f
|       |   0x0804845a      e8bdfeffff     call sym.imp.printf        ; int printf(const char *format)
|      ,==< 0x0804845f      eb0c           jmp 0x804846d
|      |`-> 0x08048461      c704247f8504.  mov dword [esp], str.Invalid_Password__n ; [0x804857f:4]=0x61766e49 ; "Invalid Password!." @ 0x804857f
|      |    0x08048468      e8affeffff     call sym.imp.printf        ; int printf(const char *format)
|      |       ; JMP XREF from 0x0804845f (main)
|      `--> 0x0804846d      b800000000     mov eax, 0
|           0x08048472      c9             leave
\           0x08048473      c3             ret
```

So, we can use a `nop` instruction to replace the jump instruction at `0x08048451`.
```
[0x08048330]> s 0x08048451
[0x08048451]> wx 90
[0x08048451]> pd 9
|           0x08048451      90             nop
|           0x08048452      0e             push cs
|           0x08048453      c704246f8504.  mov dword [esp], str.Password_OK_:__n ; [0x804856f:4]=0x73736150 ; "Password OK :)." @ 0x804856f
|           0x0804845a      e8bdfeffff     call sym.imp.printf        ; int printf(const char *format)
|       ,=< 0x0804845f      eb0c           jmp 0x804846d
|       |   0x08048461      c704247f8504.  mov dword [esp], str.Invalid_Password__n ; [0x804857f:4]=0x61766e49 ; "Invalid Password!." @ 0x804857f
|       |   0x08048468      e8affeffff     call sym.imp.printf        ; int printf(const char *format)
|       |      ; JMP XREF from 0x0804845f (main)
|       `-> 0x0804846d      b800000000     mov eax, 0
|           0x08048472      c9             leave
```
```
bin-linux$ ./crackme0x02
IOLI Crackme Level 0x02
Password: 123456
Password OK :)
```

## Crackme0x03
The challenge becomes a little more difficult, there is no jump instruction.
```
bin-linux$ radare2 -w crackme0x03
[0x08048360]> aa
[x] Analyze all flags starting with sym. and entry0 (aa)
[0x08048360]> pdf @ sym.main
            ;-- main:
/ (fcn) sym.main 128
|   sym.main ();
|           ; var int local_ch @ ebp-0xc
|           ; var int local_8h @ ebp-0x8
|           ; var int local_4h @ ebp-0x4
|           ; var int local_4h_2 @ esp+0x4
|              ; DATA XREF from 0x08048377 (entry0)
|           0x08048498      55             push ebp
|           0x08048499      89e5           mov ebp, esp
|           0x0804849b      83ec18         sub esp, 0x18
|           0x0804849e      83e4f0         and esp, 0xfffffff0
|           0x080484a1      b800000000     mov eax, 0
|           0x080484a6      83c00f         add eax, 0xf
|           0x080484a9      83c00f         add eax, 0xf
|           0x080484ac      c1e804         shr eax, 4
|           0x080484af      c1e004         shl eax, 4
|           0x080484b2      29c4           sub esp, eax
|           0x080484b4      c70424108604.  mov dword [esp], str.IOLI_Crackme_Level_0x03_n ; [0x8048610:4]=0x494c4f49 ; "IOLI Crackme Level 0x03." @ 0x8048610
|           0x080484bb      e890feffff     call sym.imp.printf        ; int printf(const char *format)
|           0x080484c0      c70424298604.  mov dword [esp], str.Password: ; [0x8048629:4]=0x73736150 ; "Password: " @ 0x8048629
|           0x080484c7      e884feffff     call sym.imp.printf        ; int printf(const char *format)
|           0x080484cc      8d45fc         lea eax, dword [local_4h]
|           0x080484cf      89442404       mov dword [local_4h_2], eax
|           0x080484d3      c70424348604.  mov dword [esp], 0x8048634  ; [0x8048634:4]=0x6425 ; "%d"
|           0x080484da      e851feffff     call sym.imp.scanf         ; int scanf(const char *format)
|           0x080484df      c745f85a0000.  mov dword [local_8h], 0x5a  ; 'Z'
|           0x080484e6      c745f4ec0100.  mov dword [local_ch], 0x1ec
|           0x080484ed      8b55f4         mov edx, dword [local_ch]
|           0x080484f0      8d45f8         lea eax, dword [local_8h]
|           0x080484f3      0110           add dword [eax], edx
|           0x080484f5      8b45f8         mov eax, dword [local_8h]
|           0x080484f8      0faf45f8       imul eax, dword [local_8h]
|           0x080484fc      8945f4         mov dword [local_ch], eax
|           0x080484ff      8b45f4         mov eax, dword [local_ch]
|           0x08048502      89442404       mov dword [local_4h_2], eax
|           0x08048506      8b45fc         mov eax, dword [local_4h]
|           0x08048509      890424         mov dword [esp], eax
|           0x0804850c      e85dffffff     call sym.test
|           0x08048511      b800000000     mov eax, 0
|           0x08048516      c9             leave
\           0x08048517      c3             ret
```

We found a function called `sym.test`, let's see the detail.
```
[0x08048360]> pdf @ sym.test
/ (fcn) sym.test 42
|   sym.test (int arg_8h, int arg_ch);
|           ; arg int arg_8h @ ebp+0x8
|           ; arg int arg_ch @ ebp+0xc
|              ; CALL XREF from 0x0804850c (sym.main)
|           0x0804846e      55             push ebp
|           0x0804846f      89e5           mov ebp, esp
|           0x08048471      83ec08         sub esp, 8
|           0x08048474      8b4508         mov eax, dword [arg_8h]     ; [0x8:4]=0
|           0x08048477      3b450c         cmp eax, dword [arg_ch]     ; [0xc:4]=0
|       ,=< 0x0804847a      740e           je 0x804848a
|       |   0x0804847c      c70424ec8504.  mov dword [esp], str.Lqydolg_Sdvvzrug_ ; [0x80485ec:4]=0x6479714c ; "Lqydolg#Sdvvzrug$" @ 0x80485ec
|       |   0x08048483      e88cffffff     call sym.shift
|      ,==< 0x08048488      eb0c           jmp 0x8048496
|      |`-> 0x0804848a      c70424fe8504.  mov dword [esp], str.Sdvvzrug_RN______ ; [0x80485fe:4]=0x76766453 ; "Sdvvzrug#RN$$$#=," @ 0x80485fe
|      |    0x08048491      e87effffff     call sym.shift
|      |       ; JMP XREF from 0x08048488 (sym.test)
|      `--> 0x08048496      c9             leave
\           0x08048497      c3             ret
```

In the below disassembly code, there are two strings seems to be encrypted, just like `Invalid_Password__n` and `Password_OK_` we found before.

### Method 1
Of course, we can replace `je` to `cmp` at `0x0804847a` in function `sym.test`.
```
[0x08048360]> s 0x0804847a
[0x0804847a]> wx eb
[0x0804847a]> pdf @ sym.test
/ (fcn) sym.test 42
|   sym.test (int arg_8h, int arg_ch);
|           ; arg int arg_8h @ ebp+0x8
|           ; arg int arg_ch @ ebp+0xc
|              ; CALL XREF from 0x0804850c (sym.main)
|           0x0804846e      55             push ebp
|           0x0804846f      89e5           mov ebp, esp
|           0x08048471      83ec08         sub esp, 8
|           0x08048474      8b4508         mov eax, dword [arg_8h]     ; [0x8:4]=0
|           0x08048477      3b450c         cmp eax, dword [arg_ch]     ; [0xc:4]=0
|       ,=< 0x0804847a      eb0e           jmp 0x804848a
|       |   0x0804847c      c70424ec8504.  mov dword [esp], str.Lqydolg_Sdvvzrug_ ; [0x80485ec:4]=0x6479714c ; "Lqydolg#Sdvvzrug$" @ 0x80485ec
|       |   0x08048483      e88cffffff     call sym.shift
|      ,==< 0x08048488      eb0c           jmp 0x8048496
|      |`-> 0x0804848a      c70424fe8504.  mov dword [esp], str.Sdvvzrug_RN______ ; [0x80485fe:4]=0x76766453 ; "Sdvvzrug#RN$$$#=," @ 0x80485fe
|      |    0x08048491      e87effffff     call sym.shift
|      |       ; JMP XREF from 0x08048488 (sym.test)
|      `--> 0x08048496      c9             leave
\           0x08048497      c3             ret
```
```
bin-linux$ ./crackme0x03
IOLI Crackme Level 0x03
Password: 123456
Password OK!!! :)
```

### Method 2
We focus on these lines in function `sym.main`, I have made some annotations.
```
|           0x080484df      c745f85a0000.  mov dword [local_8h], 0x5a  ; 'Z'
|           0x080484e6      c745f4ec0100.  mov dword [local_ch], 0x1ec
|           0x080484ed      8b55f4         mov edx, dword [local_ch]          ; edx = 0x1ec
|           0x080484f0      8d45f8         lea eax, dword [local_8h]          ; eax -> ebp-0x8
|           0x080484f3      0110           add dword [eax], edx               ; ebp-0x8 = (0x5a + 0x1ec)
|           0x080484f5      8b45f8         mov eax, dword [local_8h]          ; eax = 0x5a + 0x1ec = 0x246
|           0x080484f8      0faf45f8       imul eax, dword [local_8h]         ; eax = 0x246 * 0x246 = 0x52b24
|           0x080484fc      8945f4         mov dword [local_ch], eax          ; ebp-0xc = 0x52b24
|           0x080484ff      8b45f4         mov eax, dword [local_ch]          ; eax = 0x52b24
|           0x08048502      89442404       mov dword [local_4h_2], eax        ; esp+0x4 = eax
```

So, The value of `eax` is `0x52b24`, and in function `sym.test`:
```
|           0x08048474      8b4508         mov eax, dword [arg_8h]     ; [0x8:4]=0
|           0x08048477      3b450c         cmp eax, dword [arg_ch]     ; [0xc:4]=0
```
Although we know the `eax` in `sym.main` is different from in `sym.test`. We just try password with the decimal value of `0x52b24`.
```
[0x08048360]> ? 0x52b24
338724 0x52b24 01225444 330.8K 5000:0b24 338724 "$+\x05" 000001010010101100100100 338724.0 338724.000000f 338724.000000
```
```
bin-linux$ ./crackme0x03
IOLI Crackme Level 0x03
Password: 338724
Password OK!!! :)
```
Oh, surprise!

From the name of functin `sym.shift`, I guess the encryption algorithm to be transposition cipher.
```
[0x08048360]> pdf @ sym.shift
/ (fcn) sym.shift 90
|   sym.shift (int arg_8h);
|           ; var int local_7ch @ ebp-0x7c
|           ; var int local_78h @ ebp-0x78
|           ; arg int arg_8h @ ebp+0x8
|           ; var int local_4h @ esp+0x4
|              ; CALL XREF from 0x08048491 (sym.test)
|              ; CALL XREF from 0x08048483 (sym.test)
|           0x08048414      55             push ebp
|           0x08048415      89e5           mov ebp, esp
|           0x08048417      81ec98000000   sub esp, 0x98
|           0x0804841d      c74584000000.  mov dword [local_7ch], 0
|              ; JMP XREF from 0x0804844e (sym.shift)
|       .-> 0x08048424      8b4508         mov eax, dword [arg_8h]     ; [0x8:4]=0
|       |   0x08048427      890424         mov dword [esp], eax
|       |   0x0804842a      e811ffffff     call sym.imp.strlen        ; size_t strlen(const char *s)
|       |   0x0804842f      394584         cmp dword [local_7ch], eax  ; [0x13:4]=256
|      ,==< 0x08048432      731c           jae 0x8048450
|      ||   0x08048434      8d4588         lea eax, dword [local_78h]
|      ||   0x08048437      89c2           mov edx, eax
|      ||   0x08048439      035584         add edx, dword [local_7ch]
|      ||   0x0804843c      8b4584         mov eax, dword [local_7ch]
|      ||   0x0804843f      034508         add eax, dword [arg_8h]
|      ||   0x08048442      0fb600         movzx eax, byte [eax]
|      ||   0x08048445      2c03           sub al, 3
|      ||   0x08048447      8802           mov byte [edx], al
|      ||   0x08048449      8d4584         lea eax, dword [local_7ch]
|      ||   0x0804844c      ff00           inc dword [eax]
|      |`=< 0x0804844e      ebd4           jmp 0x8048424
|      `--> 0x08048450      8d4588         lea eax, dword [local_78h]
|           0x08048453      034584         add eax, dword [local_7ch]
|           0x08048456      c60000         mov byte [eax], 0
|           0x08048459      8d4588         lea eax, dword [local_78h]
|           0x0804845c      89442404       mov dword [local_4h], eax
|           0x08048460      c70424e88504.  mov dword [esp], 0x80485e8  ; [0x80485e8:4]=0xa7325 ; "%s."
|           0x08048467      e8e4feffff     call sym.imp.printf        ; int printf(const char *format)
|           0x0804846c      c9             leave
\           0x0804846d      c3             ret
```

Through the analysis, we can write the following script to decrypted.
```
In [1]: print(''.join([chr(ord(i)-0x3) for i in 'Sdvvzrug#RN$$$#=']))
Password OK!!! :)

In [2]: print(''.join([chr(ord(i)-0x3) for i in 'Lqydolg#Sdvvzrug$']))
Invalid Password!
```

## Crackme0x04
```
bin-linux$ radare2 -w crackme0x04
[0x080483d0]> aa
[x] Analyze all flags starting with sym. and entry0 (aa)
[0x080483d0]> pdf @ sym.main
            ;-- main:
/ (fcn) sym.main 92
|   sym.main ();
|           ; var int local_78h @ ebp-0x78
|           ; var int local_4h @ esp+0x4
|              ; DATA XREF from 0x080483e7 (entry0)
|           0x08048509      55             push ebp
|           0x0804850a      89e5           mov ebp, esp
|           0x0804850c      81ec88000000   sub esp, 0x88
|           0x08048512      83e4f0         and esp, 0xfffffff0
|           0x08048515      b800000000     mov eax, 0
|           0x0804851a      83c00f         add eax, 0xf
|           0x0804851d      83c00f         add eax, 0xf
|           0x08048520      c1e804         shr eax, 4
|           0x08048523      c1e004         shl eax, 4
|           0x08048526      29c4           sub esp, eax
|           0x08048528      c704245e8604.  mov dword [esp], str.IOLI_Crackme_Level_0x04_n ; [0x804865e:4]=0x494c4f49 ; "IOLI Crackme Level 0x04." @ 0x804865e
|           0x0804852f      e860feffff     call sym.imp.printf        ; int printf(const char *format)
|           0x08048534      c70424778604.  mov dword [esp], str.Password: ; [0x8048677:4]=0x73736150 ; "Password: " @ 0x8048677
|           0x0804853b      e854feffff     call sym.imp.printf        ; int printf(const char *format)
|           0x08048540      8d4588         lea eax, dword [local_78h]
|           0x08048543      89442404       mov dword [local_4h], eax
|           0x08048547      c70424828604.  mov dword [esp], 0x8048682  ; [0x8048682:4]=0x7325 ; "%s"
|           0x0804854e      e821feffff     call sym.imp.scanf         ; int scanf(const char *format)
|           0x08048553      8d4588         lea eax, dword [local_78h]
|           0x08048556      890424         mov dword [esp], eax
|           0x08048559      e826ffffff     call sym.check
|           0x0804855e      b800000000     mov eax, 0
|           0x08048563      c9             leave
\           0x08048564      c3             ret
[0x080483d0]> pdf @ sym.check
/ (fcn) sym.check 133
|   sym.check (int arg_8h);
|           ; var int local_dh @ ebp-0xd
|           ; var int local_ch @ ebp-0xc
|           ; var int local_8h @ ebp-0x8
|           ; var int local_4h @ ebp-0x4
|           ; arg int arg_8h @ ebp+0x8
|           ; var int local_4h_2 @ esp+0x4
|           ; var int local_8h_2 @ esp+0x8
|              ; CALL XREF from 0x08048559 (sym.main)
|           0x08048484      55             push ebp
|           0x08048485      89e5           mov ebp, esp
|           0x08048487      83ec28         sub esp, 0x28               ; '('
|           0x0804848a      c745f8000000.  mov dword [local_8h], 0
|           0x08048491      c745f4000000.  mov dword [local_ch], 0
|              ; JMP XREF from 0x080484f9 (sym.check)
|       .-> 0x08048498      8b4508         mov eax, dword [arg_8h]     ; [0x8:4]=0
|       |   0x0804849b      890424         mov dword [esp], eax
|       |   0x0804849e      e8e1feffff     call sym.imp.strlen        ; size_t strlen(const char *s)
|       |   0x080484a3      3945f4         cmp dword [local_ch], eax   ; [0x13:4]=256  counter > strlen ?
|      ,==< 0x080484a6      7353           jae 0x80484fb
|      ||   0x080484a8      8b45f4         mov eax, dword [local_ch]
|      ||   0x080484ab      034508         add eax, dword [arg_8h]
|      ||   0x080484ae      0fb600         movzx eax, byte [eax]
|      ||   0x080484b1      8845f3         mov byte [local_dh], al
|      ||   0x080484b4      8d45fc         lea eax, dword [local_4h]
|      ||   0x080484b7      89442408       mov dword [local_8h_2], eax
|      ||   0x080484bb      c74424043886.  mov dword [local_4h_2], 0x8048638 ; [0x8048638:4]=0x50006425 ; "%d"
|      ||   0x080484c3      8d45f3         lea eax, dword [local_dh]
|      ||   0x080484c6      890424         mov dword [esp], eax
|      ||   0x080484c9      e8d6feffff     call sym.imp.sscanf        ; int sscanf(const char *s,
|      ||   0x080484ce      8b55fc         mov edx, dword [local_4h]
|      ||   0x080484d1      8d45f8         lea eax, dword [local_8h]
|      ||   0x080484d4      0110           add dword [eax], edx
|      ||   0x080484d6      837df80f       cmp dword [local_8h], 0xf   ; [0xf:4]=0x3000200
|     ,===< 0x080484da      7518           jne 0x80484f4
|     |||   0x080484dc      c704243b8604.  mov dword [esp], str.Password_OK__n ; [0x804863b:4]=0x73736150 ; "Password OK!." @ 0x804863b
|     |||   0x080484e3      e8acfeffff     call sym.imp.printf        ; int printf(const char *format)
|     |||   0x080484e8      c70424000000.  mov dword [esp], 0
|     |||   0x080484ef      e8c0feffff     call sym.imp.exit          ; void exit(int status)
|     `---> 0x080484f4      8d45f4         lea eax, dword [local_ch]
|      ||   0x080484f7      ff00           inc dword [eax]
|      |`=< 0x080484f9      eb9d           jmp 0x8048498
|      `--> 0x080484fb      c70424498604.  mov dword [esp], str.Password_Incorrect__n ; [0x8048649:4]=0x73736150 ; "Password Incorrect!." @ 0x8048649
|           0x08048502      e88dfeffff     call sym.imp.printf        ; int printf(const char *format)
|           0x08048507      c9             leave
\           0x08048508      c3             ret
```

With the `man sscanf` command, we know that `sscanf()` reads its input from the character string pointed to by `str`.
```
int sscanf(const char *str, const char *format, ...);
```
The functin `strlen()` get the length of the password we input. Everytime function `sscanf()` get a character from our password as "`%s`", and then transform it to "`%d`". With the loop for *len* times, which *len* is the length of our password, these number add together, and compare with `0xf`(aka 15). If equals, jump to output the successful message.
```
bin-linux$ ./crackme0x04
IOLI Crackme Level 0x04
Password: 555
Password OK!
bin-linux$ ./crackme0x04
IOLI Crackme Level 0x04
Password: 12345
Password OK!
```

## crackme0x05
```
bin-linux$ radare2 -w crackme0x05
[0x080483d0]> aa
[x] Analyze all flags starting with sym. and entry0 (aa)
[0x080483d0]> pdf @ sym.main
            ;-- main:
/ (fcn) sym.main 92
|   sym.main ();
|           ; var int local_78h @ ebp-0x78
|           ; var int local_4h @ esp+0x4
|              ; DATA XREF from 0x080483e7 (entry0)
|           0x08048540      55             push ebp
|           0x08048541      89e5           mov ebp, esp
|           0x08048543      81ec88000000   sub esp, 0x88
|           0x08048549      83e4f0         and esp, 0xfffffff0
|           0x0804854c      b800000000     mov eax, 0
|           0x08048551      83c00f         add eax, 0xf
|           0x08048554      83c00f         add eax, 0xf
|           0x08048557      c1e804         shr eax, 4
|           0x0804855a      c1e004         shl eax, 4
|           0x0804855d      29c4           sub esp, eax
|           0x0804855f      c704248e8604.  mov dword [esp], str.IOLI_Crackme_Level_0x05_n ; [0x804868e:4]=0x494c4f49 ; "IOLI Crackme Level 0x05." @ 0x804868e
|           0x08048566      e829feffff     call sym.imp.printf        ; int printf(const char *format)
|           0x0804856b      c70424a78604.  mov dword [esp], str.Password: ; [0x80486a7:4]=0x73736150 ; "Password: " @ 0x80486a7
|           0x08048572      e81dfeffff     call sym.imp.printf        ; int printf(const char *format)
|           0x08048577      8d4588         lea eax, dword [local_78h]
|           0x0804857a      89442404       mov dword [local_4h], eax
|           0x0804857e      c70424b28604.  mov dword [esp], 0x80486b2  ; [0x80486b2:4]=0x7325 ; "%s"
|           0x08048585      e8eafdffff     call sym.imp.scanf         ; int scanf(const char *format)
|           0x0804858a      8d4588         lea eax, dword [local_78h]
|           0x0804858d      890424         mov dword [esp], eax
|           0x08048590      e833ffffff     call sym.check
|           0x08048595      b800000000     mov eax, 0
|           0x0804859a      c9             leave
\           0x0804859b      c3             ret
[0x080483d0]> pdf @ sym.check
/ (fcn) sym.check 120
|   sym.check (int arg_8h);
|           ; var int local_dh @ ebp-0xd
|           ; var int local_ch @ ebp-0xc
|           ; var int local_8h @ ebp-0x8
|           ; var int local_4h @ ebp-0x4
|           ; arg int arg_8h @ ebp+0x8
|           ; var int local_4h_2 @ esp+0x4
|           ; var int local_8h_2 @ esp+0x8
|              ; CALL XREF from 0x08048590 (sym.main)
|           0x080484c8      55             push ebp
|           0x080484c9      89e5           mov ebp, esp
|           0x080484cb      83ec28         sub esp, 0x28               ; '('
|           0x080484ce      c745f8000000.  mov dword [local_8h], 0
|           0x080484d5      c745f4000000.  mov dword [local_ch], 0
|              ; JMP XREF from 0x08048530 (sym.check)
|       .-> 0x080484dc      8b4508         mov eax, dword [arg_8h]     ; [0x8:4]=0
|       |   0x080484df      890424         mov dword [esp], eax
|       |   0x080484e2      e89dfeffff     call sym.imp.strlen        ; size_t strlen(const char *s)
|       |   0x080484e7      3945f4         cmp dword [local_ch], eax   ; [0x13:4]=256
|      ,==< 0x080484ea      7346           jae 0x8048532
|      ||   0x080484ec      8b45f4         mov eax, dword [local_ch]
|      ||   0x080484ef      034508         add eax, dword [arg_8h]
|      ||   0x080484f2      0fb600         movzx eax, byte [eax]
|      ||   0x080484f5      8845f3         mov byte [local_dh], al
|      ||   0x080484f8      8d45fc         lea eax, dword [local_4h]
|      ||   0x080484fb      89442408       mov dword [local_8h_2], eax
|      ||   0x080484ff      c74424046886.  mov dword [local_4h_2], 0x8048668 ; [0x8048668:4]=0x50006425 ; "%d"
|      ||   0x08048507      8d45f3         lea eax, dword [local_dh]
|      ||   0x0804850a      890424         mov dword [esp], eax
|      ||   0x0804850d      e892feffff     call sym.imp.sscanf        ; int sscanf(const char *s,
|      ||   0x08048512      8b55fc         mov edx, dword [local_4h]
|      ||   0x08048515      8d45f8         lea eax, dword [local_8h]
|      ||   0x08048518      0110           add dword [eax], edx
|      ||   0x0804851a      837df810       cmp dword [local_8h], 0x10  ; [0x10:4]=0x30002
|     ,===< 0x0804851e      750b           jne 0x804852b
|     |||   0x08048520      8b4508         mov eax, dword [arg_8h]     ; [0x8:4]=0
|     |||   0x08048523      890424         mov dword [esp], eax
|     |||   0x08048526      e859ffffff     call sym.parell
|     `---> 0x0804852b      8d45f4         lea eax, dword [local_ch]
|      ||   0x0804852e      ff00           inc dword [eax]
|      |`=< 0x08048530      ebaa           jmp 0x80484dc
|      `--> 0x08048532      c70424798604.  mov dword [esp], str.Password_Incorrect__n ; [0x8048679:4]=0x73736150 ; "Password Incorrect!." @ 0x8048679
|           0x08048539      e856feffff     call sym.imp.printf        ; int printf(const char *format)
|           0x0804853e      c9             leave
\           0x0804853f      c3             ret
[0x080483d0]> pdf @ sym.parell
/ (fcn) sym.parell 68
|   sym.parell (int arg_8h);
|           ; var int local_4h @ ebp-0x4
|           ; arg int arg_8h @ ebp+0x8
|           ; var int local_4h_2 @ esp+0x4
|           ; var int local_8h @ esp+0x8
|              ; CALL XREF from 0x08048526 (sym.check)
|           0x08048484      55             push ebp
|           0x08048485      89e5           mov ebp, esp
|           0x08048487      83ec18         sub esp, 0x18
|           0x0804848a      8d45fc         lea eax, dword [local_4h]
|           0x0804848d      89442408       mov dword [local_8h], eax
|           0x08048491      c74424046886.  mov dword [local_4h_2], 0x8048668 ; [0x8048668:4]=0x50006425 ; "%d"
|           0x08048499      8b4508         mov eax, dword [arg_8h]     ; [0x8:4]=0
|           0x0804849c      890424         mov dword [esp], eax
|           0x0804849f      e800ffffff     call sym.imp.sscanf        ; int sscanf(const char *s,
|           0x080484a4      8b45fc         mov eax, dword [local_4h]
|           0x080484a7      83e001         and eax, 1
|           0x080484aa      85c0           test eax, eax
|       ,=< 0x080484ac      7518           jne 0x80484c6
|       |   0x080484ae      c704246b8604.  mov dword [esp], str.Password_OK__n ; [0x804866b:4]=0x73736150 ; "Password OK!." @ 0x804866b
|       |   0x080484b5      e8dafeffff     call sym.imp.printf        ; int printf(const char *format)
|       |   0x080484ba      c70424000000.  mov dword [esp], 0
|       |   0x080484c1      e8eefeffff     call sym.imp.exit          ; void exit(int status)
|       `-> 0x080484c6      c9             leave
\           0x080484c7      c3             ret
[0x080483d0]>
```
This time, if we want to modify instructions to get success, there are three places need to modify, `0x080484ea`, `0x0804851e`, and `0x080484ac`.

Next, we use a normal method. Same function as the previous crackme, but this time, it's not compared to 15, but to 16. And instead of a printf("Password OK!").
```
bin-linux$ ./crackme0x05
IOLI Crackme Level 0x05
Password: 97
Password Incorrect!
bin-linux$ ./crackme0x05
IOLI Crackme Level 0x05
Password: 970
Password OK!
```

## crackme0x06
```
[0x08048400]> aa
[x] Analyze all flags starting with sym. and entry0 (aa)
[0x08048400]> pdf @ sym.main
            ;-- main:
/ (fcn) sym.main 99
|   sym.main (int arg_10h);
|           ; var int local_78h @ ebp-0x78
|           ; arg int arg_10h @ ebp+0x10
|           ; var int local_4h @ esp+0x4
|              ; DATA XREF from 0x08048417 (entry0)
|           0x08048607      55             push ebp
|           0x08048608      89e5           mov ebp, esp
|           0x0804860a      81ec88000000   sub esp, 0x88
|           0x08048610      83e4f0         and esp, 0xfffffff0
|           0x08048613      b800000000     mov eax, 0
|           0x08048618      83c00f         add eax, 0xf
|           0x0804861b      83c00f         add eax, 0xf
|           0x0804861e      c1e804         shr eax, 4
|           0x08048621      c1e004         shl eax, 4
|           0x08048624      29c4           sub esp, eax
|           0x08048626      c70424638704.  mov dword [esp], str.IOLI_Crackme_Level_0x06_n ; [0x8048763:4]=0x494c4f49 ; "IOLI Crackme Level 0x06." @ 0x8048763
|           0x0804862d      e886fdffff     call sym.imp.printf        ; int printf(const char *format)
|           0x08048632      c704247c8704.  mov dword [esp], str.Password: ; [0x804877c:4]=0x73736150 ; "Password: " @ 0x804877c
|           0x08048639      e87afdffff     call sym.imp.printf        ; int printf(const char *format)
|           0x0804863e      8d4588         lea eax, dword [local_78h]
|           0x08048641      89442404       mov dword [local_4h], eax
|           0x08048645      c70424878704.  mov dword [esp], 0x8048787  ; [0x8048787:4]=0x7325 ; "%s"
|           0x0804864c      e847fdffff     call sym.imp.scanf         ; int scanf(const char *format)
|           0x08048651      8b4510         mov eax, dword [arg_10h]    ; [0x10:4]=0x30002
|           0x08048654      89442404       mov dword [local_4h], eax
|           0x08048658      8d4588         lea eax, dword [local_78h]
|           0x0804865b      890424         mov dword [esp], eax
|           0x0804865e      e825ffffff     call sym.check
|           0x08048663      b800000000     mov eax, 0
|           0x08048668      c9             leave
\           0x08048669      c3             ret
[0x08048400]> pdf @ sym.check
/ (fcn) sym.check 127
|   sym.check (int arg_8h, int arg_ch);
|           ; var int local_dh @ ebp-0xd
|           ; var int local_ch @ ebp-0xc
|           ; var int local_8h @ ebp-0x8
|           ; var int local_4h @ ebp-0x4
|           ; arg int arg_8h @ ebp+0x8
|           ; arg int arg_ch @ ebp+0xc
|           ; var int local_4h_2 @ esp+0x4
|           ; var int local_8h_2 @ esp+0x8
|              ; CALL XREF from 0x0804865e (sym.main)
|           0x08048588      55             push ebp
|           0x08048589      89e5           mov ebp, esp
|           0x0804858b      83ec28         sub esp, 0x28               ; '('
|           0x0804858e      c745f8000000.  mov dword [local_8h], 0
|           0x08048595      c745f4000000.  mov dword [local_ch], 0
|              ; JMP XREF from 0x080485f7 (sym.check)
|       .-> 0x0804859c      8b4508         mov eax, dword [arg_8h]     ; [0x8:4]=0
|       |   0x0804859f      890424         mov dword [esp], eax
|       |   0x080485a2      e801feffff     call sym.imp.strlen        ; size_t strlen(const char *s)
|       |   0x080485a7      3945f4         cmp dword [local_ch], eax   ; [0x13:4]=256
|      ,==< 0x080485aa      734d           jae 0x80485f9
|      ||   0x080485ac      8b45f4         mov eax, dword [local_ch]
|      ||   0x080485af      034508         add eax, dword [arg_8h]
|      ||   0x080485b2      0fb600         movzx eax, byte [eax]
|      ||   0x080485b5      8845f3         mov byte [local_dh], al
|      ||   0x080485b8      8d45fc         lea eax, dword [local_4h]
|      ||   0x080485bb      89442408       mov dword [local_8h_2], eax
|      ||   0x080485bf      c74424043d87.  mov dword [local_4h_2], 0x804873d ; [0x804873d:4]=0x50006425 ; "%d"
|      ||   0x080485c7      8d45f3         lea eax, dword [local_dh]
|      ||   0x080485ca      890424         mov dword [esp], eax
|      ||   0x080485cd      e8f6fdffff     call sym.imp.sscanf        ; int sscanf(const char *s,
|      ||   0x080485d2      8b55fc         mov edx, dword [local_4h]
|      ||   0x080485d5      8d45f8         lea eax, dword [local_8h]
|      ||   0x080485d8      0110           add dword [eax], edx
|      ||   0x080485da      837df810       cmp dword [local_8h], 0x10  ; [0x10:4]=0x30002
|     ,===< 0x080485de      7512           jne 0x80485f2
|     |||   0x080485e0      8b450c         mov eax, dword [arg_ch]     ; [0xc:4]=0
|     |||   0x080485e3      89442404       mov dword [local_4h_2], eax
|     |||   0x080485e7      8b4508         mov eax, dword [arg_8h]     ; [0x8:4]=0
|     |||   0x080485ea      890424         mov dword [esp], eax
|     |||   0x080485ed      e828ffffff     call sym.parell
|     `---> 0x080485f2      8d45f4         lea eax, dword [local_ch]
|      ||   0x080485f5      ff00           inc dword [eax]
|      |`=< 0x080485f7      eba3           jmp 0x804859c
|      `--> 0x080485f9      c704244e8704.  mov dword [esp], str.Password_Incorrect__n ; [0x804874e:4]=0x73736150 ; "Password Incorrect!." @ 0x804874e
|           0x08048600      e8b3fdffff     call sym.imp.printf        ; int printf(const char *format)
|           0x08048605      c9             leave
\           0x08048606      c3             ret
[0x08048400]> pdf @ sym.parell
/ (fcn) sym.parell 110
|   sym.parell (int arg_8h, int arg_ch);
|           ; var int local_8h_2 @ ebp-0x8
|           ; var int local_4h @ ebp-0x4
|           ; arg int arg_8h @ ebp+0x8
|           ; arg int arg_ch @ ebp+0xc
|           ; var int local_4h_2 @ esp+0x4
|           ; var int local_8h @ esp+0x8
|              ; CALL XREF from 0x080485ed (sym.check)
|           0x0804851a      55             push ebp
|           0x0804851b      89e5           mov ebp, esp
|           0x0804851d      83ec18         sub esp, 0x18
|           0x08048520      8d45fc         lea eax, dword [local_4h]
|           0x08048523      89442408       mov dword [local_8h], eax
|           0x08048527      c74424043d87.  mov dword [local_4h_2], 0x804873d ; [0x804873d:4]=0x50006425 ; "%d"
|           0x0804852f      8b4508         mov eax, dword [arg_8h]     ; [0x8:4]=0
|           0x08048532      890424         mov dword [esp], eax
|           0x08048535      e88efeffff     call sym.imp.sscanf        ; int sscanf(const char *s,
|           0x0804853a      8b450c         mov eax, dword [arg_ch]     ; [0xc:4]=0
|           0x0804853d      89442404       mov dword [local_4h_2], eax
|           0x08048541      8b45fc         mov eax, dword [local_4h]
|           0x08048544      890424         mov dword [esp], eax
|           0x08048547      e868ffffff     call sym.dummy
|           0x0804854c      85c0           test eax, eax
|       ,=< 0x0804854e      7436           je 0x8048586
|       |   0x08048550      c745f8000000.  mov dword [local_8h_2], 0
|       |      ; JMP XREF from 0x08048584 (sym.parell)
|      .--> 0x08048557      837df809       cmp dword [local_8h_2], 9   ; [0x9:4]=0
|     ,===< 0x0804855b      7f29           jg 0x8048586
|     |||   0x0804855d      8b45fc         mov eax, dword [local_4h]
|     |||   0x08048560      83e001         and eax, 1
|     |||   0x08048563      85c0           test eax, eax
|    ,====< 0x08048565      7518           jne 0x804857f
|    ||||   0x08048567      c70424408704.  mov dword [esp], str.Password_OK__n ; [0x8048740:4]=0x73736150 ; "Password OK!." @ 0x8048740
|    ||||   0x0804856e      e845feffff     call sym.imp.printf        ; int printf(const char *format)
|    ||||   0x08048573      c70424000000.  mov dword [esp], 0
|    ||||   0x0804857a      e869feffff     call sym.imp.exit          ; void exit(int status)
|    `----> 0x0804857f      8d45f8         lea eax, dword [local_8h_2]
|     |||   0x08048582      ff00           inc dword [eax]
|     |`==< 0x08048584      ebd1           jmp 0x8048557
|     `-`-> 0x08048586      c9             leave
\           0x08048587      c3             ret
[0x08048400]> pdf @ sym.dummy
/ (fcn) sym.dummy 102
|   sym.dummy (int arg_ch);
|           ; var int local_8h_2 @ ebp-0x8
|           ; var int local_4h @ ebp-0x4
|           ; arg int arg_ch @ ebp+0xc
|           ; var int local_4h_2 @ esp+0x4
|           ; var int local_8h @ esp+0x8
|              ; CALL XREF from 0x08048547 (sym.parell)
|           0x080484b4      55             push ebp
|           0x080484b5      89e5           mov ebp, esp
|           0x080484b7      83ec18         sub esp, 0x18
|           0x080484ba      c745fc000000.  mov dword [local_4h], 0
|       .-> 0x080484c1      8b45fc         mov eax, dword [local_4h]
|       |   0x080484c4      8d1485000000.  lea edx, dword [eax*4]
|       |   0x080484cb      8b450c         mov eax, dword [arg_ch]     ; [0xc:4]=0
|       |   0x080484ce      833c0200       cmp dword [edx + eax], 0
|      ,==< 0x080484d2      743a           je 0x804850e
|      ||   0x080484d4      8b45fc         mov eax, dword [local_4h]
|      ||   0x080484d7      8d0c85000000.  lea ecx, dword [eax*4]
|      ||   0x080484de      8b550c         mov edx, dword [arg_ch]     ; [0xc:4]=0
|      ||   0x080484e1      8d45fc         lea eax, dword [local_4h]
|      ||   0x080484e4      ff00           inc dword [eax]
|      ||   0x080484e6      c74424080300.  mov dword [local_8h], 3
|      ||   0x080484ee      c74424043887.  mov dword [local_4h_2], str.LOLO ; [0x8048738:4]=0x4f4c4f4c ; "LOLO" @ 0x8048738
|      ||   0x080484f6      8b0411         mov eax, dword [ecx + edx]
|      ||   0x080484f9      890424         mov dword [esp], eax
|      ||   0x080484fc      e8d7feffff     call sym.imp.strncmp       ; int strncmp(const char *s1, const char *s2, size_t n)
|      ||   0x08048501      85c0           test eax, eax
|      |`=< 0x08048503      75bc           jne 0x80484c1
|      |    0x08048505      c745f8010000.  mov dword [local_8h_2], 1
|      |,=< 0x0804850c      eb07           jmp 0x8048515
|      `--> 0x0804850e      c745f8000000.  mov dword [local_8h_2], 0
|       |      ; JMP XREF from 0x0804850c (sym.dummy)
|       `-> 0x08048515      8b45f8         mov eax, dword [local_8h_2]
|           0x08048518      c9             leave
\           0x08048519      c3             ret
[0x08048400]> pdf @ sym.imp.sscanf
/ (fcn) sym.imp.sscanf 6
|   sym.imp.sscanf ();
|              ; CALL XREF from 0x080485cd (sym.check)
|              ; CALL XREF from 0x08048535 (sym.parell)
\           0x080483c8      ff2510a00408   jmp dword [reloc.sscanf_16] ; reloc.sscanf_16
```

Everything is same as before except the parameters of `sym.check`.
```
;before    check(int passwork)
|   sym.check (int arg_8h);

;now       check(int password, char* argv[])
|   sym.check (int arg_8h, int arg_ch);
```
Then the environment pointer is passed to `sym.parell`
```
|   sym.parell (int arg_8h, int arg_ch);
```
There is a new function in `sym.parell`, named `sym.dummy`. Here are something interest:
```
|      ||   0x080484ee      c74424043887.  mov dword [local_4h_2], str.LOLO ; [0x8048738:4]=0x4f4c4f4c ; "LOLO" @ 0x8048738

|      ||   0x080484fc      e8d7feffff     call sym.imp.strncmp       ; int strncmp(const char *s1, const char *s2, size_t n)
```
Looks like the binary wants the same things as the previous one, plus an environment variable named `LOLO`.
```
bin-linux$ LOLO= ./crackme0x06
IOLI Crackme Level 0x06
Password: 97
Password Incorrect!
bin-linux$ LOLO= ./crackme0x06
IOLI Crackme Level 0x06
Password: 970
Password OK!
```

## crackme0x07
```
[0x08048400]> aa
[x] Analyze all flags starting with sym. and entry0 (aa)
[0x08048400]> pdf @ sym.main
Invalid address (sym.main)
|ERROR| Invalid command 'pdf @ sym.main' (0x70)
```
The symbols seems have some error. `rabin2` with `-I`, we can see the binary info.
```
bin-linux$ rabin2 -I crackme0x07
arch     x86
binsz    4780
bintype  elf
bits     32
canary   false
class    ELF32
crypto   false
endian   little
havecode true
intrp    /lib/ld-linux.so.2
lang     c
linenum  false
lsyms    false
machine  Intel 80386
maxopsz  16
minopsz  1
nx       true
os       linux
pcalign  0
pic      false
relocs   false
relro    partial relro
rpath    NONE
static   false
stripped true
subsys   linux
va       true
```
Noted that `stripped true`, so there have no more symbols.

```
[0x08048400]> pdf
            ;-- section..text:
/ (fcn) entry0 33
|   entry0 ();
|           0x08048400      31ed           xor ebp, ebp                ; section 13 va=0x08048400 pa=0x00000400 sz=900 vsz=900 rwx=--r-x .text
|           0x08048402      5e             pop esi
|           0x08048403      89e1           mov ecx, esp
|           0x08048405      83e4f0         and esp, 0xfffffff0
|           0x08048408      50             push eax
|           0x08048409      54             push esp
|           0x0804840a      52             push edx
|           0x0804840b      6850870408     push 0x8048750              ; "U......$........U..S.........................u.X[]..U..S."
|           0x08048410      68e0860408     push 0x80486e0
|           0x08048415      51             push ecx
|           0x08048416      56             push esi
|           0x08048417      687d860408     push main                   ; "U....." @ 0x804867d
\           0x0804841c      e867ffffff     call sym.imp.__libc_start_main; int __libc_start_main(func main, int argc, char **ubp_av, func init, func fini, func rtld_fini, void *stack_end)
```
Since this is GCC-produced code, the main is likely at `0x804867d` (the last push before `imp.__libc_start_main`)
```
[0x08048400]> pdf @ main
/ (fcn) main 99
|   main (int arg_10h);
|           ; var int local_78h @ ebp-0x78
|           ; arg int arg_10h @ ebp+0x10
|           ; var int local_4h @ esp+0x4
|              ; DATA XREF from 0x08048417 (entry0)
|           0x0804867d      55             push ebp
|           0x0804867e      89e5           mov ebp, esp
|           0x08048680      81ec88000000   sub esp, 0x88
|           0x08048686      83e4f0         and esp, 0xfffffff0
|           0x08048689      b800000000     mov eax, 0
|           0x0804868e      83c00f         add eax, 0xf
|           0x08048691      83c00f         add eax, 0xf
|           0x08048694      c1e804         shr eax, 4
|           0x08048697      c1e004         shl eax, 4
|           0x0804869a      29c4           sub esp, eax
|           0x0804869c      c70424d98704.  mov dword [esp], str.IOLI_Crackme_Level_0x07_n ; [0x80487d9:4]=0x494c4f49 ; "IOLI Crackme Level 0x07." @ 0x80487d9
|           0x080486a3      e810fdffff     call sym.imp.printf        ; int printf(const char *format)
|           0x080486a8      c70424f28704.  mov dword [esp], str.Password: ; [0x80487f2:4]=0x73736150 ; "Password: " @ 0x80487f2
|           0x080486af      e804fdffff     call sym.imp.printf        ; int printf(const char *format)
|           0x080486b4      8d4588         lea eax, dword [local_78h]
|           0x080486b7      89442404       mov dword [local_4h], eax
|           0x080486bb      c70424fd8704.  mov dword [esp], 0x80487fd  ; [0x80487fd:4]=0x7325 ; "%s"
|           0x080486c2      e8d1fcffff     call sym.imp.scanf         ; int scanf(const char *format)
|           0x080486c7      8b4510         mov eax, dword [arg_10h]    ; [0x10:4]=0x30002
|           0x080486ca      89442404       mov dword [local_4h], eax
|           0x080486ce      8d4588         lea eax, dword [local_78h]
|           0x080486d1      890424         mov dword [esp], eax
|           0x080486d4      e8e0feffff     call fcn.080485b9
|           0x080486d9      b800000000     mov eax, 0
|           0x080486de      c9             leave
\           0x080486df      c3             ret
[0x08048400]> pdf @ fcn.080485b9
/ (fcn) fcn.080485b9 196
|   fcn.080485b9 (int arg_8h, int arg_ch);
|           ; var int local_dh @ ebp-0xd
|           ; var int local_ch @ ebp-0xc
|           ; var int local_8h @ ebp-0x8
|           ; var int local_4h @ ebp-0x4
|           ; arg int arg_8h @ ebp+0x8
|           ; arg int arg_ch @ ebp+0xc
|           ; var int local_4h_2 @ esp+0x4
|           ; var int local_8h_2 @ esp+0x8
|              ; CALL XREF from 0x080486d4 (main)
|           0x080485b9      55             push ebp
|           0x080485ba      89e5           mov ebp, esp
|           0x080485bc      83ec28         sub esp, 0x28               ; '('
|           0x080485bf      c745f8000000.  mov dword [local_8h], 0
|           0x080485c6      c745f4000000.  mov dword [local_ch], 0
|              ; JMP XREF from 0x08048628 (fcn.080485b9)
|       .-> 0x080485cd      8b4508         mov eax, dword [arg_8h]     ; [0x8:4]=0
|       |   0x080485d0      890424         mov dword [esp], eax
|       |   0x080485d3      e8d0fdffff     call sym.imp.strlen        ; size_t strlen(const char *s)
|       |   0x080485d8      3945f4         cmp dword [local_ch], eax   ; [0x13:4]=256
|      ,==< 0x080485db      734d           jae 0x804862a
|      ||   0x080485dd      8b45f4         mov eax, dword [local_ch]
|      ||   0x080485e0      034508         add eax, dword [arg_8h]
|      ||   0x080485e3      0fb600         movzx eax, byte [eax]
|      ||   0x080485e6      8845f3         mov byte [local_dh], al
|      ||   0x080485e9      8d45fc         lea eax, dword [local_4h]
|      ||   0x080485ec      89442408       mov dword [local_8h_2], eax
|      ||   0x080485f0      c7442404c287.  mov dword [local_4h_2], 0x80487c2 ; [0x80487c2:4]=0x50006425 ; "%d"
|      ||   0x080485f8      8d45f3         lea eax, dword [local_dh]
|      ||   0x080485fb      890424         mov dword [esp], eax
|      ||   0x080485fe      e8c5fdffff     call sym.imp.sscanf        ; int sscanf(const char *s,
|      ||   0x08048603      8b55fc         mov edx, dword [local_4h]
|      ||   0x08048606      8d45f8         lea eax, dword [local_8h]
|      ||   0x08048609      0110           add dword [eax], edx
|      ||   0x0804860b      837df810       cmp dword [local_8h], 0x10  ; [0x10:4]=0x30002
|     ,===< 0x0804860f      7512           jne 0x8048623
|     |||   0x08048611      8b450c         mov eax, dword [arg_ch]     ; [0xc:4]=0
|     |||   0x08048614      89442404       mov dword [local_4h_2], eax
|     |||   0x08048618      8b4508         mov eax, dword [arg_8h]     ; [0x8:4]=0
|     |||   0x0804861b      890424         mov dword [esp], eax
|     |||   0x0804861e      e81fffffff     call 0x8048542
|     `---> 0x08048623      8d45f4         lea eax, dword [local_ch]
|      ||   0x08048626      ff00           inc dword [eax]
|      |`=< 0x08048628      eba3           jmp 0x80485cd
|      `--> 0x0804862a      e8f5feffff     call 0x8048524
|           0x0804862f      8b450c         mov eax, dword [arg_ch]     ; [0xc:4]=0
|           0x08048632      89442404       mov dword [local_4h_2], eax
|           0x08048636      8b45fc         mov eax, dword [local_4h]
|           0x08048639      890424         mov dword [esp], eax
|           0x0804863c      e873feffff     call 0x80484b4
|           0x08048641      85c0           test eax, eax
|       ,=< 0x08048643      7436           je 0x804867b
|       |   0x08048645      c745f4000000.  mov dword [local_ch], 0
|       |      ; JMP XREF from 0x08048679 (fcn.080485b9)
|      .--> 0x0804864c      837df409       cmp dword [local_ch], 9     ; [0x9:4]=0
|     ,===< 0x08048650      7f29           jg 0x804867b
|     |||   0x08048652      8b45fc         mov eax, dword [local_4h]
|     |||   0x08048655      83e001         and eax, 1
|     |||   0x08048658      85c0           test eax, eax
|    ,====< 0x0804865a      7518           jne 0x8048674
|    ||||   0x0804865c      c70424d38704.  mov dword [esp], str.wtf__n ; [0x80487d3:4]=0x3f667477 ; "wtf?." @ 0x80487d3
|    ||||   0x08048663      e850fdffff     call sym.imp.printf        ; int printf(const char *format)
|    ||||   0x08048668      c70424000000.  mov dword [esp], 0
|    ||||   0x0804866f      e874fdffff     call sym.imp.exit          ; void exit(int status)
|    `----> 0x08048674      8d45f4         lea eax, dword [local_ch]
|     |||   0x08048677      ff00           inc dword [eax]
|     |`==< 0x08048679      ebd1           jmp 0x804864c
|     `-`-> 0x0804867b      c9             leave
\           0x0804867c      c3             ret
```
The program logic is all the same.
```
s = 0
for i in password:
    s += i
    if s == 0x10:
        sym.parell()
print "Invalid"
```
In function `0x8048542`, there are something interest.
```
.--> 0x0804857f      837df809       cmp dword [ebp - 8], 9      ; [0x9:4]=0
,===< 0x08048583      7f32           jg 0x80485b7
```
So, `9` is the maximum length of password.
```
bin-linux$ LOLO= ./crackme0x07
IOLI Crackme Level 0x07
Password: 111111118
Password OK!
bin-linux$ LOLO= ./crackme0x07
IOLI Crackme Level 0x07
Password: 1111111117
Password Incorrect!
```

## Crackme0x08
```
[0x08048400]> aa
[x] Analyze all flags starting with sym. and entry0 (aa)
[0x08048400]> pdf @ sym.main
            ;-- main:
/ (fcn) sym.main 99
|   sym.main (int arg_10h);
|           ; var int local_78h @ ebp-0x78
|           ; arg int arg_10h @ ebp+0x10
|           ; var int local_4h @ esp+0x4
|              ; DATA XREF from 0x08048417 (entry0)
|           0x0804867d      55             push ebp
|           0x0804867e      89e5           mov ebp, esp
|           0x08048680      81ec88000000   sub esp, 0x88
|           0x08048686      83e4f0         and esp, 0xfffffff0
|           0x08048689      b800000000     mov eax, 0
|           0x0804868e      83c00f         add eax, 0xf
|           0x08048691      83c00f         add eax, 0xf
|           0x08048694      c1e804         shr eax, 4
|           0x08048697      c1e004         shl eax, 4
|           0x0804869a      29c4           sub esp, eax
|           0x0804869c      c70424d98704.  mov dword [esp], str.IOLI_Crackme_Level_0x08_n ; [0x80487d9:4]=0x494c4f49 ; "IOLI Crackme Level 0x08." @ 0x80487d9
|           0x080486a3      e810fdffff     call sym.imp.printf        ; int printf(const char *format)
|           0x080486a8      c70424f28704.  mov dword [esp], str.Password: ; [0x80487f2:4]=0x73736150 ; "Password: " @ 0x80487f2
|           0x080486af      e804fdffff     call sym.imp.printf        ; int printf(const char *format)
|           0x080486b4      8d4588         lea eax, dword [local_78h]
|           0x080486b7      89442404       mov dword [local_4h], eax
|           0x080486bb      c70424fd8704.  mov dword [esp], 0x80487fd  ; [0x80487fd:4]=0x7325 ; "%s"
|           0x080486c2      e8d1fcffff     call sym.imp.scanf         ; int scanf(const char *format)
|           0x080486c7      8b4510         mov eax, dword [arg_10h]    ; [0x10:4]=0x30002
|           0x080486ca      89442404       mov dword [local_4h], eax
|           0x080486ce      8d4588         lea eax, dword [local_78h]
|           0x080486d1      890424         mov dword [esp], eax
|           0x080486d4      e8e0feffff     call sym.check
|           0x080486d9      b800000000     mov eax, 0
|           0x080486de      c9             leave
\           0x080486df      c3             ret
[0x08048400]> pdf @ sym.check
/ (fcn) sym.check 196
|   sym.check (int arg_8h, int arg_ch);
|           ; var int local_dh @ ebp-0xd
|           ; var int local_ch @ ebp-0xc
|           ; var int local_8h @ ebp-0x8
|           ; var int local_4h @ ebp-0x4
|           ; arg int arg_8h @ ebp+0x8
|           ; arg int arg_ch @ ebp+0xc
|           ; var int local_4h_2 @ esp+0x4
|           ; var int local_8h_2 @ esp+0x8
|              ; CALL XREF from 0x080486d4 (sym.main)
|           0x080485b9      55             push ebp
|           0x080485ba      89e5           mov ebp, esp
|           0x080485bc      83ec28         sub esp, 0x28               ; '('
|           0x080485bf      c745f8000000.  mov dword [local_8h], 0
|           0x080485c6      c745f4000000.  mov dword [local_ch], 0
|              ; JMP XREF from 0x08048628 (sym.check)
|       .-> 0x080485cd      8b4508         mov eax, dword [arg_8h]     ; [0x8:4]=0
|       |   0x080485d0      890424         mov dword [esp], eax
|       |   0x080485d3      e8d0fdffff     call sym.imp.strlen        ; size_t strlen(const char *s)
|       |   0x080485d8      3945f4         cmp dword [local_ch], eax   ; [0x13:4]=256
|      ,==< 0x080485db      734d           jae 0x804862a
|      ||   0x080485dd      8b45f4         mov eax, dword [local_ch]
|      ||   0x080485e0      034508         add eax, dword [arg_8h]
|      ||   0x080485e3      0fb600         movzx eax, byte [eax]
|      ||   0x080485e6      8845f3         mov byte [local_dh], al
|      ||   0x080485e9      8d45fc         lea eax, dword [local_4h]
|      ||   0x080485ec      89442408       mov dword [local_8h_2], eax
|      ||   0x080485f0      c7442404c287.  mov dword [local_4h_2], 0x80487c2 ; [0x80487c2:4]=0x50006425 ; "%d"
|      ||   0x080485f8      8d45f3         lea eax, dword [local_dh]
|      ||   0x080485fb      890424         mov dword [esp], eax
|      ||   0x080485fe      e8c5fdffff     call sym.imp.sscanf        ; int sscanf(const char *s,
|      ||   0x08048603      8b55fc         mov edx, dword [local_4h]
|      ||   0x08048606      8d45f8         lea eax, dword [local_8h]
|      ||   0x08048609      0110           add dword [eax], edx
|      ||   0x0804860b      837df810       cmp dword [local_8h], 0x10  ; [0x10:4]=0x30002
|     ,===< 0x0804860f      7512           jne 0x8048623
|     |||   0x08048611      8b450c         mov eax, dword [arg_ch]     ; [0xc:4]=0
|     |||   0x08048614      89442404       mov dword [local_4h_2], eax
|     |||   0x08048618      8b4508         mov eax, dword [arg_8h]     ; [0x8:4]=0
|     |||   0x0804861b      890424         mov dword [esp], eax
|     |||   0x0804861e      e81fffffff     call sym.parell
|     `---> 0x08048623      8d45f4         lea eax, dword [local_ch]
|      ||   0x08048626      ff00           inc dword [eax]
|      |`=< 0x08048628      eba3           jmp 0x80485cd
|      `--> 0x0804862a      e8f5feffff     call sym.che
|           0x0804862f      8b450c         mov eax, dword [arg_ch]     ; [0xc:4]=0
|           0x08048632      89442404       mov dword [local_4h_2], eax
|           0x08048636      8b45fc         mov eax, dword [local_4h]
|           0x08048639      890424         mov dword [esp], eax
|           0x0804863c      e873feffff     call sym.dummy
|           0x08048641      85c0           test eax, eax
|       ,=< 0x08048643      7436           je 0x804867b
|       |   0x08048645      c745f4000000.  mov dword [local_ch], 0
|       |      ; JMP XREF from 0x08048679 (sym.check)
|      .--> 0x0804864c      837df409       cmp dword [local_ch], 9     ; [0x9:4]=0
|     ,===< 0x08048650      7f29           jg 0x804867b
|     |||   0x08048652      8b45fc         mov eax, dword [local_4h]
|     |||   0x08048655      83e001         and eax, 1
|     |||   0x08048658      85c0           test eax, eax
|    ,====< 0x0804865a      7518           jne 0x8048674
|    ||||   0x0804865c      c70424d38704.  mov dword [esp], str.wtf__n ; [0x80487d3:4]=0x3f667477 ; "wtf?." @ 0x80487d3
|    ||||   0x08048663      e850fdffff     call sym.imp.printf        ; int printf(const char *format)
|    ||||   0x08048668      c70424000000.  mov dword [esp], 0
|    ||||   0x0804866f      e874fdffff     call sym.imp.exit          ; void exit(int status)
|    `----> 0x08048674      8d45f4         lea eax, dword [local_ch]
|     |||   0x08048677      ff00           inc dword [eax]
|     |`==< 0x08048679      ebd1           jmp 0x804864c
|     `-`-> 0x0804867b      c9             leave
\           0x0804867c      c3             ret
[0x08048400]> pdf @ sym.che
/ (fcn) sym.che 30
|   sym.che ();
|              ; CALL XREF from 0x0804862a (sym.check)
|           0x08048524      55             push ebp
|           0x08048525      89e5           mov ebp, esp
|           0x08048527      83ec08         sub esp, 8
|           0x0804852a      c70424ad8704.  mov dword [esp], str.Password_Incorrect__n ; [0x80487ad:4]=0x73736150 ; "Password Incorrect!." @ 0x80487ad
|           0x08048531      e882feffff     call sym.imp.printf        ; int printf(const char *format)
|           0x08048536      c70424000000.  mov dword [esp], 0
\           0x0804853d      e8a6feffff     call sym.imp.exit          ; void exit(int status)
[0x08048400]> pdf @ sym.parell
/ (fcn) sym.parell 119
|   sym.parell (int arg_8h, int arg_ch);
|           ; var int local_8h_2 @ ebp-0x8
|           ; var int local_4h @ ebp-0x4
|           ; arg int arg_8h @ ebp+0x8
|           ; arg int arg_ch @ ebp+0xc
|           ; var int local_4h_2 @ esp+0x4
|           ; var int local_8h @ esp+0x8
|              ; CALL XREF from 0x0804861e (sym.check)
|           0x08048542      55             push ebp
|           0x08048543      89e5           mov ebp, esp
|           0x08048545      83ec18         sub esp, 0x18
|           0x08048548      8d45fc         lea eax, dword [local_4h]
|           0x0804854b      89442408       mov dword [local_8h], eax
|           0x0804854f      c7442404c287.  mov dword [local_4h_2], 0x80487c2 ; [0x80487c2:4]=0x50006425 ; "%d"
|           0x08048557      8b4508         mov eax, dword [arg_8h]     ; [0x8:4]=0
|           0x0804855a      890424         mov dword [esp], eax
|           0x0804855d      e866feffff     call sym.imp.sscanf        ; int sscanf(const char *s,
|           0x08048562      8b450c         mov eax, dword [arg_ch]     ; [0xc:4]=0
|           0x08048565      89442404       mov dword [local_4h_2], eax
|           0x08048569      8b45fc         mov eax, dword [local_4h]
|           0x0804856c      890424         mov dword [esp], eax
|           0x0804856f      e840ffffff     call sym.dummy
|           0x08048574      85c0           test eax, eax
|       ,=< 0x08048576      743f           je 0x80485b7
|       |   0x08048578      c745f8000000.  mov dword [local_8h_2], 0
|       |      ; JMP XREF from 0x080485b5 (sym.parell)
|      .--> 0x0804857f      837df809       cmp dword [local_8h_2], 9   ; [0x9:4]=0
|     ,===< 0x08048583      7f32           jg 0x80485b7
|     |||   0x08048585      8b45fc         mov eax, dword [local_4h]
|     |||   0x08048588      83e001         and eax, 1
|     |||   0x0804858b      85c0           test eax, eax
|    ,====< 0x0804858d      7521           jne 0x80485b0
|    ||||   0x0804858f      833d2ca00408.  cmp dword obj.LOL, 1        ; [0x1:4]=0x1464c45
|   ,=====< 0x08048596      750c           jne 0x80485a4
|   |||||   0x08048598      c70424c58704.  mov dword [esp], str.Password_OK__n ; [0x80487c5:4]=0x73736150 ; "Password OK!." @ 0x80487c5
|   |||||   0x0804859f      e814feffff     call sym.imp.printf        ; int printf(const char *format)
|   `-----> 0x080485a4      c70424000000.  mov dword [esp], 0
|    ||||   0x080485ab      e838feffff     call sym.imp.exit          ; void exit(int status)
|    `----> 0x080485b0      8d45f8         lea eax, dword [local_8h_2]
|     |||   0x080485b3      ff00           inc dword [eax]
|     |`==< 0x080485b5      ebc8           jmp 0x804857f
|     `-`-> 0x080485b7      c9             leave
\           0x080485b8      c3             ret
[0x08048400]> pdf @ sym.dummy
/ (fcn) sym.dummy 112
|   sym.dummy (int arg_ch);
|           ; var int local_8h_2 @ ebp-0x8
|           ; var int local_4h @ ebp-0x4
|           ; arg int arg_ch @ ebp+0xc
|           ; var int local_4h_2 @ esp+0x4
|           ; var int local_8h @ esp+0x8
|              ; CALL XREF from 0x0804863c (sym.check)
|              ; CALL XREF from 0x0804856f (sym.parell)
|           0x080484b4      55             push ebp
|           0x080484b5      89e5           mov ebp, esp
|           0x080484b7      83ec18         sub esp, 0x18
|           0x080484ba      c745fc000000.  mov dword [local_4h], 0
|       .-> 0x080484c1      8b45fc         mov eax, dword [local_4h]
|       |   0x080484c4      8d1485000000.  lea edx, dword [eax*4]
|       |   0x080484cb      8b450c         mov eax, dword [arg_ch]     ; [0xc:4]=0
|       |   0x080484ce      833c0200       cmp dword [edx + eax], 0
|      ,==< 0x080484d2      7444           je 0x8048518
|      ||   0x080484d4      8b45fc         mov eax, dword [local_4h]
|      ||   0x080484d7      8d0c85000000.  lea ecx, dword [eax*4]
|      ||   0x080484de      8b550c         mov edx, dword [arg_ch]     ; [0xc:4]=0
|      ||   0x080484e1      8d45fc         lea eax, dword [local_4h]
|      ||   0x080484e4      ff00           inc dword [eax]
|      ||   0x080484e6      c74424080300.  mov dword [local_8h], 3
|      ||   0x080484ee      c7442404a887.  mov dword [local_4h_2], str.LOLO ; [0x80487a8:4]=0x4f4c4f4c ; "LOLO" @ 0x80487a8
|      ||   0x080484f6      8b0411         mov eax, dword [ecx + edx]
|      ||   0x080484f9      890424         mov dword [esp], eax
|      ||   0x080484fc      e8d7feffff     call sym.imp.strncmp       ; int strncmp(const char *s1, const char *s2, size_t n)
|      ||   0x08048501      85c0           test eax, eax
|      |`=< 0x08048503      75bc           jne 0x80484c1
|      |    0x08048505      c7052ca00408.  mov dword obj.LOL, 1        ; [0x804a02c:4]=0x4728203a ; ": (GNU) 3.4.6 (Gentoo 3.4.6-r2, ssp-3.4.6-1.0, pie-8.7.10)" @ 0x804a02c
|      |    0x0804850f      c745f8010000.  mov dword [local_8h_2], 1
|      |,=< 0x08048516      eb07           jmp 0x804851f
|      `--> 0x08048518      c745f8000000.  mov dword [local_8h_2], 0
|       |      ; JMP XREF from 0x08048516 (sym.dummy)
|       `-> 0x0804851f      8b45f8         mov eax, dword [local_8h_2]
|           0x08048522      c9             leave
\           0x08048523      c3             ret
```
It just like a unstripped version of crackme0x07.
```
bin-linux$ LOLO= ./crackme0x08
IOLI Crackme Level 0x08
Password: 111111118
Password OK!
bin-linux$ LOLO= ./crackme0x08
IOLI Crackme Level 0x08
Password: 1111111117
Password Incorrect!
```

## crackme0x09
```
bin-linux$ rabin2 -I crackme0x09
arch     x86
binsz    4780
bintype  elf
bits     32
canary   false
class    ELF32
crypto   false
endian   little
havecode true
intrp    /lib/ld-linux.so.2
lang     c
linenum  false
lsyms    false
machine  Intel 80386
maxopsz  16
minopsz  1
nx       true
os       linux
pcalign  0
pic      false
relocs   false
relro    partial relro
rpath    NONE
static   false
stripped true
subsys   linux
va       true
```
It is stripped.
```
[0x08048420]> pdf @ main
/ (fcn) main 120
|   main (int arg_10h);
|           ; var int local_78h @ ebp-0x78
|           ; var int local_4h_2 @ ebp-0x4
|           ; arg int arg_10h @ ebp+0x10
|           ; var int local_4h @ esp+0x4
|              ; DATA XREF from 0x08048437 (entry0)
|           0x080486ee      55             push ebp
|           0x080486ef      89e5           mov ebp, esp
|           0x080486f1      53             push ebx
|           0x080486f2      81ec84000000   sub esp, 0x84
|           0x080486f8      e869000000     call fcn.08048766
|           0x080486fd      81c3f7180000   add ebx, 0x18f7
|           0x08048703      83e4f0         and esp, 0xfffffff0
|           0x08048706      b800000000     mov eax, 0
|           0x0804870b      83c00f         add eax, 0xf
|           0x0804870e      83c00f         add eax, 0xf
|           0x08048711      c1e804         shr eax, 4
|           0x08048714      c1e004         shl eax, 4
|           0x08048717      29c4           sub esp, eax
|           0x08048719      8d8375e8ffff   lea eax, dword [ebx - 0x178b]
|           0x0804871f      890424         mov dword [esp], eax
|           0x08048722      e8b9fcffff     call sym.imp.printf        ; int printf(const char *format)
|           0x08048727      8d838ee8ffff   lea eax, dword [ebx - 0x1772]
|           0x0804872d      890424         mov dword [esp], eax
|           0x08048730      e8abfcffff     call sym.imp.printf        ; int printf(const char *format)
|           0x08048735      8d4588         lea eax, dword [local_78h]
|           0x08048738      89442404       mov dword [local_4h], eax
|           0x0804873c      8d8399e8ffff   lea eax, dword [ebx - 0x1767]
|           0x08048742      890424         mov dword [esp], eax
|           0x08048745      e876fcffff     call sym.imp.scanf         ; int scanf(const char *format)
|           0x0804874a      8b4510         mov eax, dword [arg_10h]    ; [0x10:4]=0x30002
|           0x0804874d      89442404       mov dword [local_4h], eax
|           0x08048751      8d4588         lea eax, dword [local_78h]
|           0x08048754      890424         mov dword [esp], eax
|           0x08048757      e8bafeffff     call fcn.08048616
|           0x0804875c      b800000000     mov eax, 0
|           0x08048761      8b5dfc         mov ebx, dword [local_4h_2]
|           0x08048764      c9             leave
\           0x08048765      c3             ret
[0x08048420]> pdf @ fcn.08048766
/ (fcn) fcn.08048766 4
|   fcn.08048766 ();
|              ; CALL XREF from 0x080486f8 (main)
|              ; CALL XREF from 0x0804861d (fcn.08048616)
|           0x08048766      8b1c24         mov ebx, dword [esp]
\           0x08048769      c3             ret
[0x08048420]> pdf @ fcn.08048616
/ (fcn) fcn.08048616 216
|   fcn.08048616 (int arg_8h, int arg_ch);
|           ; var int local_11h @ ebp-0x11
|           ; var int local_10h @ ebp-0x10
|           ; var int local_ch @ ebp-0xc
|           ; var int local_8h @ ebp-0x8
|           ; arg int arg_8h @ ebp+0x8
|           ; arg int arg_ch @ ebp+0xc
|           ; var int local_4h @ esp+0x4
|           ; var int local_8h_2 @ esp+0x8
|              ; CALL XREF from 0x08048757 (main)
|           0x08048616      55             push ebp
|           0x08048617      89e5           mov ebp, esp
|           0x08048619      53             push ebx
|           0x0804861a      83ec24         sub esp, 0x24               ; '$'
|           0x0804861d      e844010000     call fcn.08048766
|           0x08048622      81c3d2190000   add ebx, 0x19d2
|           0x08048628      c745f4000000.  mov dword [local_ch], 0
|           0x0804862f      c745f0000000.  mov dword [local_10h], 0
|              ; JMP XREF from 0x08048693 (fcn.08048616)
|       .-> 0x08048636      8b4508         mov eax, dword [arg_8h]     ; [0x8:4]=0
|       |   0x08048639      890424         mov dword [esp], eax
|       |   0x0804863c      e88ffdffff     call sym.imp.strlen        ; size_t strlen(const char *s)
|       |   0x08048641      3945f0         cmp dword [local_10h], eax  ; [0x13:4]=256
|      ,==< 0x08048644      734f           jae 0x8048695
|      ||   0x08048646      8b45f0         mov eax, dword [local_10h]
|      ||   0x08048649      034508         add eax, dword [arg_8h]
|      ||   0x0804864c      0fb600         movzx eax, byte [eax]
|      ||   0x0804864f      8845ef         mov byte [local_11h], al
|      ||   0x08048652      8d45f8         lea eax, dword [local_8h]
|      ||   0x08048655      89442408       mov dword [local_8h_2], eax
|      ||   0x08048659      8d835ee8ffff   lea eax, dword [ebx - 0x17a2]
|      ||   0x0804865f      89442404       mov dword [local_4h], eax
|      ||   0x08048663      8d45ef         lea eax, dword [local_11h]
|      ||   0x08048666      890424         mov dword [esp], eax
|      ||   0x08048669      e882fdffff     call sym.imp.sscanf        ; int sscanf(const char *s,
|      ||   0x0804866e      8b55f8         mov edx, dword [local_8h]
|      ||   0x08048671      8d45f4         lea eax, dword [local_ch]
|      ||   0x08048674      0110           add dword [eax], edx
|      ||   0x08048676      837df410       cmp dword [local_ch], 0x10  ; [0x10:4]=0x30002
|     ,===< 0x0804867a      7512           jne 0x804868e
|     |||   0x0804867c      8b450c         mov eax, dword [arg_ch]     ; [0xc:4]=0
|     |||   0x0804867f      89442404       mov dword [local_4h], eax
|     |||   0x08048683      8b4508         mov eax, dword [arg_8h]     ; [0x8:4]=0
|     |||   0x08048686      890424         mov dword [esp], eax
|     |||   0x08048689      e8fbfeffff     call 0x8048589
|     `---> 0x0804868e      8d45f0         lea eax, dword [local_10h]
|      ||   0x08048691      ff00           inc dword [eax]
|      |`=< 0x08048693      eba1           jmp 0x8048636
|      `--> 0x08048695      e8c3feffff     call 0x804855d
|           0x0804869a      8b450c         mov eax, dword [arg_ch]     ; [0xc:4]=0
|           0x0804869d      89442404       mov dword [local_4h], eax
|           0x080486a1      8b45f8         mov eax, dword [local_8h]
|           0x080486a4      890424         mov dword [esp], eax
|           0x080486a7      e828feffff     call 0x80484d4
|           0x080486ac      85c0           test eax, eax
|       ,=< 0x080486ae      7438           je 0x80486e8
|       |   0x080486b0      c745f0000000.  mov dword [local_10h], 0
|       |      ; JMP XREF from 0x080486e6 (fcn.08048616)
|      .--> 0x080486b7      837df009       cmp dword [local_10h], 9    ; [0x9:4]=0
|     ,===< 0x080486bb      7f2b           jg 0x80486e8
|     |||   0x080486bd      8b45f8         mov eax, dword [local_8h]
|     |||   0x080486c0      83e001         and eax, 1
|     |||   0x080486c3      85c0           test eax, eax
|    ,====< 0x080486c5      751a           jne 0x80486e1
|    ||||   0x080486c7      8d836fe8ffff   lea eax, dword [ebx - 0x1791]
|    ||||   0x080486cd      890424         mov dword [esp], eax
|    ||||   0x080486d0      e80bfdffff     call sym.imp.printf        ; int printf(const char *format)
|    ||||   0x080486d5      c70424000000.  mov dword [esp], 0
|    ||||   0x080486dc      e82ffdffff     call sym.imp.exit          ; void exit(int status)
|    `----> 0x080486e1      8d45f0         lea eax, dword [local_10h]
|     |||   0x080486e4      ff00           inc dword [eax]
|     |`==< 0x080486e6      ebcf           jmp 0x80486b7
|     `-`-> 0x080486e8      83c424         add esp, 0x24               ; '$'
|           0x080486eb      5b             pop ebx
|           0x080486ec      5d             pop ebp
\           0x080486ed      c3             ret
```
```
bin-linux$ LOLO= ./crackme0x09
IOLI Crackme Level 0x09
Password: 111111118
Password OK!
bin-linux$ LOLO= ./crackme0x09
IOLI Crackme Level 0x09
Password: 1111111117
Password Incorrect!
```
