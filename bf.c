/*
 * assemble:
 *     nasm -felf64 out.s
 *
 * link:
 *     ld out.o
 *
 * run:
 *     ./a.out
 */
#include <stdio.h>
#include <stdlib.h>

struct stack {
    size_t *data;
    size_t idx;
    size_t cap;
};

static int stack_init(struct stack *stack, size_t cap) {
    stack->data = calloc(cap, sizeof(stack->data));
    if (stack->data == NULL) {
        return -1;
    }
    stack->idx = 0;
    stack->cap = cap;
    return 0;
}

static void stack_push(struct stack *stack, size_t item) {
    if (stack->idx > stack->cap) {
        fprintf(stderr, "pushing to full stack\n");
        exit(EXIT_FAILURE);
    }
    stack->data[stack->idx] = item;
    stack->idx++;
}

static size_t stack_pop(struct stack *stack) {
    if (stack->idx == 0) {
        fprintf(stderr, "poping an empty stack\n");
        exit(EXIT_FAILURE);
    }
    stack->idx--;
    size_t item = stack->data[stack->idx];
    return item;
}

/*
 * Writes this section:
 *
 * 0000000000401000 <print>:
 *   401000:	51                   	push   rcx
 *   401001:	b8 01 00 00 00       	mov    eax,0x1
 *   401006:	bf 01 00 00 00       	mov    edi,0x1
 *   40100b:	48 8d b1 00 20 40 00 	lea    rsi,[rcx+0x402000]
 *   401012:	ba 01 00 00 00       	mov    edx,0x1
 *   401017:	0f 05                	syscall 
 *   401019:	59                   	pop    rcx
 *   40101a:	c3                   	ret    
 * 
 * 000000000040101b <read>:
 *   40101b:	51                   	push   rcx
 *   40101c:	b8 00 00 00 00       	mov    eax,0x0
 *   401021:	bf 00 00 00 00       	mov    edi,0x0
 *   401026:	48 8d b1 00 20 40 00 	lea    rsi,[rcx+0x402000]
 *   40102d:	ba 01 00 00 00       	mov    edx,0x1
 *   401032:	0f 05                	syscall 
 *   401034:	59                   	pop    rcx
 *   401035:	c3                   	ret    
 * 
 * 0000000000401036 <_exit>:
 *   401036:	b8 3c 00 00 00       	mov    eax,0x3c
 *   40103b:	bf 00 00 00 00       	mov    edi,0x0
 *   401040:	0f 05                	syscall 
 * 
 * 0000000000401042 <_start>:
 *   401042:	48 31 c9             	xor    rcx,rcx
 */
static void write_setup(FILE *fp) {
    const char * const msg =
        "global _start\n"
        "\n"
        "section .data\n"
        "tape:	resb	10000\n"
        "\n"
        "section .text\n"
        "\n"
        "write:\n"
        "  push rcx\n"
        "  mov rax, 1        	; write\n"
        "  mov rdi, 1        	; STDOUT_FILENO\n"
        "  lea rsi, [rcx+tape]	; from current cell\n"
        "  mov rdx, 1			; write 1 byte\n"
        "  syscall\n"
        "  pop rcx\n"
        "  ret\n"
        "\n"
        "read:\n"
        "  push rcx\n"
        "  mov rax, 0        	; read\n"
        "  mov rdi, 0        	; STDIN_FILENO\n"
        "  lea rsi, [rcx+tape]	; to current cell\n"
        "  mov rdx, 1        	; read 1 byte\n"
        "  syscall\n"
        "  pop rcx\n"
        "  ret\n"
        "\n"
        "_exit:\n"
        "  mov rax, 60       	; exit\n"
        "  mov rdi, 0			; 0 is success\n"
        "  syscall				; finish\n"
        "\n"
        "_start:\n"
        "  xor rcx, rcx			; initialize tape pointer in rcx\n"
        ";;; BEGIN ;;;\n";
    fprintf(fp, msg);
}

static void compile_bf(FILE *in_fp, FILE *out_fp) {
    int err;
    int c;
    struct stack loop_stack;
    size_t start_loop_label = 0;

    err = stack_init(&loop_stack, 10000);
    if (err) {
        fprintf(stderr, "failed to initialize stack\n");
        exit(EXIT_FAILURE);
    }
    while ((c = fgetc(in_fp)) != EOF) {
        switch (c) {
            case '+':
                fprintf(out_fp, "%s\n", "inc byte [rcx+tape]\t; +");
                break;

            case '-':
                fprintf(out_fp, "%s\n", "dec byte [rcx+tape]\t; -");
                break;

            case '>':
                fprintf(out_fp, "%s\n", "inc rcx\t\t\t\t; <");
                break;

            case '<':
                fprintf(out_fp, "%s\n", "dec rcx\t\t\t\t; <");
                break;

            case '.':
                fprintf(out_fp, "%s\n", "call write\t\t\t\t; .");
                break;

            case ',':
                fprintf(out_fp, "%s\n", "call read\t\t\t\t; ,");
                break;

            case '[':
                start_loop_label++;
                fprintf(out_fp, "l%zu:\t\t\t\t; [\n", start_loop_label);
                fprintf(out_fp, "jz exit_l%zu\n", start_loop_label);
                stack_push(&loop_stack, start_loop_label);
                break;

            case ']': {
                size_t exit_label = stack_pop(&loop_stack);
                fprintf(out_fp, "jmp l%zu\t\t\t\t; ]\n", exit_label);
                fprintf(out_fp, "exit_l%zu:\n", exit_label);
                break;
            }

            default:
                continue;
        }
    }
    fprintf(out_fp, "call _exit\n");
}

static int assemble(void) {
    return system("nasm -felf64 out.s");
}

static int link(void) {
    return system("ld out.o");
}

int main(void) {
    FILE *in_fp = stdin;

    FILE *out_fp = fopen("out.s", "w");
    if (out_fp == NULL) {
        exit(EXIT_FAILURE);
    }

    write_setup(out_fp);

    compile_bf(in_fp, out_fp);

    fclose(out_fp);

    assemble();
    link();

    fclose(in_fp);
    return 0;
}
