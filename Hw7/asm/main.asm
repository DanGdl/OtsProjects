    bits 64
    extern malloc, puts, printf, fflush, abort, free
    global main

    section   .data
empty_str: db 0x0
int_format: db "%ld ", 0x0
data: dq 4, 8, 15, 16, 23, 42
data_length: equ ($-data) / 8

    section   .text
;;; print_int proc
print_int:
    mov rsi, rdi
    mov rdi, int_format
    xor rax, rax
    call printf

    xor rdi, rdi
    call fflush

    ret

;;; p proc
p:
    mov rax, rdi
    and rax, 1
    ret

;;; add_element proc
add_element:
    push rbp                ; save value from rbp to stack
    push rbx                ; save value from rbx to stack

    mov rbp, rdi            ; set rbp = rdi (rdi = element from db)
    mov rbx, rsi            ; set rbx = rsi (e.g. 0)

    mov rdi, 16             ; set rdi = 16
    call malloc
    test rax, rax           ; if rax == NULL
    jz abort                ; exit

    mov [rax], rbp          ; set *rax = rbp
    mov [rax + 8], rbx      ; set [rax + 8] = rbx => rax[1] = 16

    pop rbx                 ; restore rbx
    pop rbp                 ; restore rbp

    ret

;;; m proc
m:
    test rdi, rdi
    jz outm

    push rbp                ; save value from rbp to stack
    push rbx                ; save value from rbx to stack

    mov rbx, rdi
    mov rbp, rsi            ; save value from rsi to rbp

    mov rdi, [rdi]          ; extract value from pointer
    call rsi                ; call printf

    mov rdi, [rbx + 8]      ; get next value from pointer
    mov rsi, rbp            ; restore rsi from rbp
    call m

    pop rbx                 ; restore rbx
    pop rbp                 ; restore rbx

outm:
    ret

;;; f proc
f:
    mov rax, rsi            ; rax = 0

    test rdi, rdi           ; is 0?
    jz outf

    push rbx
    push r12
    push r13

    mov rbx, rdi
    mov r12, rsi
    mov r13, rdx

    mov rdi, [rdi]
    call rdx                ; call p
    test rax, rax           ; is result of p == 0?
    jz z

    mov rdi, [rbx]
    mov rsi, r12
    call add_element
    mov rsi, rax
    jmp ff

z:
    mov rsi, r12

ff:
    mov rdi, [rbx + 8]
    mov rdx, r13
    call f

    pop r13
    pop r12
    pop rbx

outf:
    ret

;;; main proc
main:
    push rbx                        ; save rbx

    xor rax, rax                    ; set rax to 0
    mov rbx, data_length            ; set rbx = data_length
adding_loop:
    mov rdi, [data - 8 + rbx * 8]   ; set last item from dq to rdi
    mov rsi, rax                    ; set rsi = rax (e.g. 0)
    call add_element                ;
    dec rbx                         ; rbx--
    jnz adding_loop                 ; if rbx != 0 -> goto adding_loop

    mov rbx, rax                    ; rbx = rax (e.g. 0)

    mov rdi, rax                    ; rdi = rax (e.g. 0)
    mov rsi, print_int
    call m

    mov rdi, empty_str              ;
    call puts                       ; print empty string (rdi)

    mov rdx, p                      ; rdx = p
    xor rsi, rsi                    ; rsi = 0
    mov rdi, rbx                    ; rdi = rbx
    call f

    mov rdi, rax
    mov rsi, print_int
    call m

    mov rdi, empty_str
    call puts

    pop rbx

    xor rax, rax
    ret
