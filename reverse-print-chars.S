  .text
  .global main

main: 
  push {r4, r5, lr}  

  mov r0, #0
  mov r1, #8
  bl realloc  // char str[8]
  mov r2, r0  // ptr
  mov r3, #8  // capacity
  mov r4, #0  // size
  mov r5, #2
  b read
  
read:
  ldr r0, .Lstdin
  ldr r0, [r0]
  push {r2, r3}
  bl fgetc  // fgetc(stdin)
  pop {r2, r3}
  cmp r0, #-1  // if (fgetc(stdin) == EOF)
  beq write   // return
  str r0, [r2, r4]  // str[i] = ch
  add r4, #1  // ++size
  cmp r3, r4  // if (size >= capacity)
  ble get_memory
  b read
  
get_memory:
  mov r0, r2
  mul r3, r5
  mov r1, r3
  push {r3}
  bl realloc
  pop {r3}
  mov r2, r0
  b read

write:
  cmp r4, #0
  beq free_memory
  sub r4, #1
  ldr r1, .Lstdout
  ldr r1, [r1]
  ldr r0, [r2, r4]
  push {r2, r3}
  bl fputc
  pop {r2, r3}
  b write

free_memory:  
  mov r0, r2
  bl free
  b return
  
return:
  pop {r4, r5, lr}  // return
  bx lr

.Lstdin:
  .word stdin
.Lstdout:
  .word stdout
