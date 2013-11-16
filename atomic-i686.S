.file   "atomic-i686.S"
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
  pushl %ebp
  movl  %esp, %ebp
  subl  $0x8, %esp
  movl  %ebx, 0x4(%esp)
  movl  %esi, (%esp)

  # load old ref/stamp into %edx:%eax
  movl  0xc(%ebp), %edx
  movl  0x10(%ebp), %eax

  # load new ref/stamp into %ecx:%ebx
  movl  0x14(%ebp), %ecx
  movl  0x18(%ebp), %ebx

  movl  0x8(%ebp), %esi
  lock cmpxchg8b (%esi)

  # test ZF for whether we succeeded
  jz cas_succeeded
  movl  $0x0, %eax
  jmp cas_failed
cas_succeeded:
  movl  $0x1, %eax
cas_failed:
  movl  0x4(%esp), %ebx
  movl  (%esp), %esi
  movl  %ebp, %esp
  popl  %ebp
  ret

.size stamped_ref_cas, .-stamped_ref_cas

stamped_ref_get:
  pushl %ebp
  movl  %esp, %ebp
  subl  $0x8, %esp

  movq  0x8(%ebp), %mm0
  movq  %mm0, (%esp)
  emms
  movl  (%esp), %eax
  movl  0x4(%esp), %edx
  movl  0xc(%ebp), %ecx
  movl  %edx, (%ecx)
  
  movl  %ebp, %esp
  popl  %ebp
  ret

.size stamped_ref_get, .-stamped_ref_get
