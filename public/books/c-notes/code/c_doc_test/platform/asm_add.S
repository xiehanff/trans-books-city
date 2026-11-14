.text
.globl doc_asm_add
.type doc_asm_add, @function
doc_asm_add:
    leal (%rdi,%rsi), %eax
    ret
.size doc_asm_add, .-doc_asm_add
