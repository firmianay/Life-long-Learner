# 自己动手构造编译系统


## 第一章 代码背后
```
$ gcc hello.c -o hello -static --verbose
```
- `cc1`：编译器
- `as`：汇编器
- `collect2`：链接器


## 第二章 编译系统设计
####编译系统的设计
1. 词法分析：源代码->词法分析（有限自动机）->词法记号
2. 语法分析：词法记号->语法分析（文法）->抽象语法树
3. 符号表管理：记录符号信息的数据结构
4. 语义分析：抽象语法树->语义分析（语义检查）->抽象语法树
5. 代码生成：抽象语法树->代码生成->汇编代码
6. 编译优化：中间代码表示

#### 汇编程序的设计
1. 汇编词法、语法分析
```
mov eax, [ebp-8]
```
8 个词法记号分别是：`mov`、`eax`、`,`、`[`、`ebp`、`-`、`8`、`]`。然后生成语法树。

需要注意的是汇编器需要对源文件进行两边扫描，因为汇编语言允许符号的后置定义。所以第一遍扫描获取符号信息，第二遍扫描使用符号信息。

2. 表信息生成
- 段表信息通过识别 section 语法时进行处理
  ```
  0  section .text
  2  mov eax,ebx
  1  inc eax
  0  section .data
  10 buffer times 10 db 0
  0  section .bss
  EOF
  ```
  ```
  段名   段偏移 段大小
  .text   0     3
  .data   4    10
  .bss   16     0
  ```
- 符号表来源于汇编语言定义的符号
  ```
     section .text
  0  global main
  0  ......
  11 je whileExit1
  17 call fun
  22 whileExit1:
  22 ......
  ```
  ```
    section .data
  0  global glb
  0  glb dd 100
  4  global var
  4  var dd 1
  8  ......
  ```
  ```
  符号名      所在段   段内偏移 类型
  main       .text     0     Global
  fun        未定义     0     Global
  whileExit1 .text    22     Local
  glb        .data     0     Global
  var        .data     4     Global
  ```
- 重定位表
  ```
     section .text
  0  fun:
  0  ......
  ```
  ```
     section .data
  0  ext dd 0
  4  ......
  ```
  ```
     section .text
  0  main:
  0  ......
  11 je whileExit1
  17 call fun
  22 whileExit1:
  22 mov eax,[ext]
  28 mov [var],eax
  34 ......
  ```
  ```
     section .data
  0  glb dd 100
  4  var dd 1
  8  ......
  ```
  ```
  重定位符号名 重定位位置所在段 重定位位置段内偏移 重定位类型
  fun           .text           18            相对
  ext           .text           24            绝对
  var           .text           30            绝对
  ```

3. 指令生成

指令名与操作码一般是一对多的关系，因此需要根据具体的操作数类型或长度绝对操作码的值。

#### 链接程序的设计
1. 地址空间分配

链接器按照目标文件的输入顺序扫描文件信息，从每个文件的段表中提取出各个文件的代码段和数据段的信息，然后将同类型段合并，按照代码段、数据段、.bss 段的顺序依次决定每个段的起始地址。

2. 符号解析

符号分为内部符号和外部符号，符号地址 = 符号所在段基址 + 符号所在段内偏移。链接器扫描到引用外部符号时，就将该外部符号的地址修改为正确的符号地址。

3. 重定位

重定位地址 = 重定位位置所在段基址 + 重定位位置的段内偏移
