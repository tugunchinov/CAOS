

  .text
  .global main

main: 
  push {lr}  

loop: 
  ldr r0, .Lstdin
  ldr r0, [r0]
  bl fgetc  // fgetc(stdin)
  cmp r0, #-1  // if (fgetc(stdin) == EOF)
  beq return   // return
  cmp r0, #48  // if (fgetc(stdin) < '0')
  blo loop  // continue
  cmp r0, #57  // if (fgetc(stdin) > '9')
  bhi loop  // continue
  ldr r1, .Lstdout  
  ldr r1, [r1]
  bl fputc  // fputc(fgetc(stdin), stdout)
  b loop
  
return:
  pop {lr}  // return
  bx lr

.Lstdin:
  .word stdin
.Lstdout:
  .word stdout
