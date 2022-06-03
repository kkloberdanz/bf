global _start

section .data
tape:	resb	10000

section .text

print:
  push rcx
  mov rax, 1        	; write
  mov rdi, 1        	; STDOUT_FILENO
  lea rsi, [rcx+tape]	; from current cell
  mov rdx, 1			; write 1 byte
  syscall
  pop rcx
  ret

read:
  push rcx
  mov rax, 0        	; read
  mov rdi, 0        	; STDIN_FILENO
  lea rsi, [rcx+tape]	; to current cell
  mov rdx, 1        	; read 1 byte
  syscall
  pop rcx
  ret

_exit:
  mov rax, 60       	; exit
  mov rdi, 0			; 0 is success
  syscall				; finish

_start:
  xor rcx, rcx			; initialize tape pointer in rcx

  inc byte [rcx+tape]	; +
  inc byte [rcx+tape]	; +
  inc byte [rcx+tape]	; +
  inc byte [rcx+tape]	; +
  inc byte [rcx+tape]	; +
  inc byte [rcx+tape]	; +

  inc byte [rcx+tape]	; +
  inc byte [rcx+tape]	; +
  inc byte [rcx+tape]	; +
  inc byte [rcx+tape]	; +
  inc byte [rcx+tape]	; +
  inc byte [rcx+tape]	; +

  inc byte [rcx+tape]	; +
  inc byte [rcx+tape]	; +
  inc byte [rcx+tape]	; +
  inc byte [rcx+tape]	; +
  inc byte [rcx+tape]	; +
  inc byte [rcx+tape]	; +

  inc byte [rcx+tape]	; +
  inc byte [rcx+tape]	; +
  inc byte [rcx+tape]	; +
  inc byte [rcx+tape]	; +
  inc byte [rcx+tape]	; +
  inc byte [rcx+tape]	; +

  inc byte [rcx+tape]	; +
  inc byte [rcx+tape]	; +
  inc byte [rcx+tape]	; +
  inc byte [rcx+tape]	; +
  inc byte [rcx+tape]	; +
  inc byte [rcx+tape]	; +

  inc byte [rcx+tape]	; +
  inc byte [rcx+tape]	; +
  inc byte [rcx+tape]	; +
  inc byte [rcx+tape]	; +
  inc byte [rcx+tape]	; +
  inc byte [rcx+tape]	; +

  inc byte [rcx+tape]	; +

  call print			; .

  inc rcx				; >

  call read				; ,
  dec byte [rcx+tape]	; -

  call print			; .

  dec rcx				; <

  call print			; .

  inc byte [rcx+tape]	; +

  ; setup while loop
  inc rcx				; >
  inc rcx				; >
  inc byte [rcx+tape]	; +
  inc byte [rcx+tape]	; +
  inc byte [rcx+tape]	; +

  ; while loop 3 times
l1:						; [
  jz exit_l1			; [

  dec rcx				; <
  dec rcx				; <
  call print			; .
  inc rcx				; >
  inc rcx				; >
  dec byte [rcx+tape]	; -

  jmp l1				; ]
exit_l1:				; ]

  call _exit
