.file   "atomic-x86_64.S"
.text

.global stamped_ref_cas
.type   stamped_ref_cas, @function

.global stamped_ref_get
.type   stamped_ref_get, @function

# uint32_t stamped_ref_cas(
#     stamped_ref_t *sr,
#     void *old_ref,
#     uint32_t old_stamp,
#     void *new_ref,
#     uint32_t new_stamp
# );
# 
# void *stamped_ref_get(stamped_ref_t *sr, uint32_t *stamp);

stamped_ref_cas:
  subq  $0x10, %rsp
  movq  %rbx, 0x8(%rsp)
  movq  %rsi, (%rsp)

  # load old ref/stamp into %rdx:%rax
  # (old stamp is already in %rdx)
  movq  %rsi, %rax

  # load new ref/stamp into %rcx:%rbx
  movq  %rcx, %rbx
  movq  %r8, %rcx

  lock cmpxchg16b (%rdi)

  # test ZF for whether we succeeded
  jz 1f
  movq  $0x0, %rax
  jmp 2f
1:
  movq  $0x1, %rax
2:
  movq  (%rsp), %rsi
  movq  0x8(%rsp), %rbx
  addq  $0x10, %rsp
  ret

.size stamped_ref_cas, .-stamped_ref_cas

stamped_ref_get:
  subq    $0x8,   %rsp
  lddqu   (%rdi), %xmm0
  movhps  %xmm0,  (%rsp)
  movl    (%rsp), %eax
  movl    %eax,   (%rsi)
  movq    %xmm0,  %rax
  addq    $0x8,   %rsp
  ret

.size stamped_ref_get, .-stamped_ref_get

