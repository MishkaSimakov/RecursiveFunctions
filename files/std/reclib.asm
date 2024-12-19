.p2align 4
.global _print
_print:
stp x29, x30, [sp, #-16]!
mov x29, sp

sub sp, sp, #16
str x0, [sp]
adrp x0, format@PAGE
add x0, x0, format@PAGEOFF
mov x1, sp
bl _printf
add sp, sp, #16

ldp x29, x30, [sp], #16
ret

.data
format: .asciz "%i\n"
