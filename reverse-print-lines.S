  #include <sys/syscall.h>

  .intel_syntax noprefix
  .text
  .global _start

_start:
  mov rax, SYS_brk
  mov rdi, 0
  syscall  // rax -> address of data
  push rax 
  mov r12, rax  // ptr to block start
  add rax, 1024
  mov r13, rax // ptr to block end
  mov rax, SYS_brk
  mov rdi, r13
  syscall
  
read:
  mov rax, SYS_read
  mov rdi, 0
  mov rsi, r12
  mov rdx, 1024
  syscall
  mov r15, rax  // bytes read
  cmp rax, 1024  // bytes read < 1024 -> EOF
  jl write
  
get_memory:
  mov r12, r13
  add r13, 1024
  mov rax, SYS_brk
  mov rdi, r13
  syscall
  jmp read
  
write:
  mov r14, r12
  add r14, r15
  mov r15, r14  // r15 -> end of text
  pop rax
  cmp r15, rax
  je return  // empty text
  push rax
  dec r15
  mov r14, 1
  cmp BYTE PTR [r15], '\n'
  je find_line
  inc r15
  movb [r15], '\n'
 
find_line:
  pop rax
  cmp r15, rax
  je write_last_line
  push rax
  dec r15
  cmp BYTE PTR [r15], '\n'
  je write_line
  add r14, 1
  jmp find_line
  
write_line:
  mov rax, SYS_write
  mov rdi, 1
  inc r15
  mov rsi, r15
  mov rdx, r14
  syscall
  mov r14, 1
  dec r15
  jmp find_line

write_last_line:
  mov rax, SYS_write
  mov rdi, 1
  mov rsi, r15
  mov rdx, r14
  syscall

return:
  // free memory
  mov rax, SYS_brk
  mov rdi, r15
  syscall
  // exit
  mov rax, SYS_exit
  mov rdi, 0
  syscall
