#!/bin/sh

nasm -felf64 bf.s || exit 1
ld bf.o || exit 1
./a.out 
echo "\nEXIT CODE: $?"
