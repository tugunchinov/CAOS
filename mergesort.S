 .intel_syntax noprefix
  
  .text
  .global mergesort


merge:
  mov r9, r8
cond1:
  cmp rdi, r8
  jl cond2
  ret
cond2:
  cmp r9, rsi
  jl loop
  ret
loop:
  mov eax, [rcx + rdi * 4]
  cmp eax, [rcx + r9 * 4]
  jg greater
lesseq:
  inc rdi
  jmp cond1
greater:
  push rbx
  push r9

  mov ebx, [rcx + r9 * 4]
  push rbx
shift:
  dec r9
  mov ebx, [rcx + r9 * 4]
  inc r9
  mov [rcx + r9 * 4], ebx
  dec r9
  cmp r9, rdi
  jg shift

  pop rbx
  mov [rcx + rdi * 4], ebx
  pop r9
  pop rbx
  
  inc rdi
  inc r8
  inc r9
  jmp cond1

  // rdi: int from
  // rsi: int to
  // rdx: const int* in
  // rcx: int* out
mergesort:
  cmp rsi, rdi
  jle return
  mov r8, rdi
copy:
  mov eax, [rdx + r8 * 4]
  mov [rcx + r8 * 4], eax
  inc r8
  cmp r8, rsi
  jl copy
sort: 
  // l < r - 1
  mov rax, rsi
  sub rax, rdi
  sub rax, 1
  cmp rax, 0
  jle return

  push rdi
  push rsi

  // mergesort(from, (from + to)/2, in, out)
  add rsi, rdi
  shr rsi
  call sort

  pop rsi
  pop rdi
  push rdi
  push rsi

  // mergesort((from + to)/2 + 1, to, in, out) 
  add rdi, rsi
  shr rdi
  call sort
  
  pop rsi
  pop rdi
  push rdi
  push rsi
  mov r8, rdi
  add r8, rsi
  shr r8
  call merge

  pop rsi
  pop rdi

return: 
  ret
