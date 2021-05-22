  .intel_syntax noprefix
  .text
  .global my_sin

my_sin:
  push rbp
  mov rbp, rsp
  sub rsp, 32
  movq [rbp - 8], 1
  movq [rbp - 16], -1
  movq [rbp - 24], 0
  
  movsd xmm1, xmm0
  movsd xmm2, xmm0
  cvtsi2sdq xmm4, [rbp - 8]
  cvtsi2sdq xmm5, [rbp - 8] // 1
  cvtsi2sdq xmm6, [rbp - 16]  // -1
  cvtsi2sdq xmm7, [rbp - 24]  // 0
loop:
  mulsd xmm1, xmm6
  mulsd xmm1, xmm2
  mulsd xmm1, xmm2
  addsd xmm4, xmm5
  divsd xmm1, xmm4
  addsd xmm4, xmm5
  divsd xmm1, xmm4
  addsd xmm0, xmm1
  comisd xmm1, xmm7
  jne loop

  add rsp, 32
  pop rbp
  ret
