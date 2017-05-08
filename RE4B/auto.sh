#!/bin/sh

#Automatic processing of C language code, to facilitate the next study

dir="$1_dir"
mkdir $dir
mv $1 $dir
cd $dir

gcc $1 -o "x64"
gcc -m32 $1 -o "x86"

gcc -g $1 -o "x64_gdb"
gcc -g -m32 $1 -o "x86_gdb"

gcc $1 -S -o "x64.s"
gcc -m32 $1 -S -o "x86.s"

gcc $1 -S -masm=intel -o "x64_intel.s"
gcc -m32 $1 -S -masm=intel -o "x86_intel.s"
