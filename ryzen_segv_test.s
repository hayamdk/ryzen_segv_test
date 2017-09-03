	.file	"ryzen_segv_test.c"
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"lock inconsistency!!\n"
	.text
	.p2align 4,,15
	.type	lock_leave, @function
lock_leave:
.LFB80:
	.cfi_startproc
	xorl	%eax, %eax
	xchgl	locked(%rip), %eax
	testl	%eax, %eax
	je	.L4
	rep ret
	.p2align 4,,10
	.p2align 3
.L4:
	movq	stderr(%rip), %rcx
	leaq	.LC0(%rip), %rdi
	movl	$21, %edx
	movl	$1, %esi
	jmp	fwrite@PLT
	.cfi_endproc
.LFE80:
	.size	lock_leave, .-lock_leave
	.section	.rodata.str1.1
.LC1:
	.string	"mismatch!!!!!!!!!!! %u %u\n"
	.text
	.p2align 4,,15
	.globl	thread1
	.type	thread1, @function
thread1:
.LFB83:
	.cfi_startproc
	pushq	%r13
	.cfi_def_cfa_offset 16
	.cfi_offset 13, -16
	pushq	%r12
	.cfi_def_cfa_offset 24
	.cfi_offset 12, -24
	movq	%rdi, %r12
	pushq	%rbp
	.cfi_def_cfa_offset 32
	.cfi_offset 6, -32
	pushq	%rbx
	.cfi_def_cfa_offset 40
	.cfi_offset 3, -40
	xorl	%ebp, %ebp
	subq	$24, %rsp
	.cfi_def_cfa_offset 64
	movq	%fs:40, %rax
	movq	%rax, 8(%rsp)
	xorl	%eax, %eax
	cmpq	$0, (%rdi)
	je	.L12
	.p2align 4,,10
	.p2align 3
.L16:
	movl	$1, %edx
	.p2align 4,,10
	.p2align 3
.L7:
	movl	%edx, %eax
	xchgl	locked(%rip), %eax
	testl	%eax, %eax
	jne	.L7
#APP
# 152 "ryzen_segv_test.c" 1
	mfence
# 0 "" 2
# 163 "ryzen_segv_test.c" 1
	cpuid
# 0 "" 2
#NO_APP
	movq	func_set(%rip), %rdi
	movzbl	571(%rdi), %eax
	movl	572(%rdi), %ebx
	leaq	64(%rdi,%rax), %rax
	call	*%rax
	movl	%eax, %r13d
	xorl	%eax, %eax
	call	lock_leave
	movl	%ebx, %eax
	sall	$13, %eax
	xorl	%ebx, %eax
	movl	%eax, %edx
	shrl	$17, %edx
	xorl	%edx, %eax
	movl	%eax, %edx
	sall	$5, %edx
	xorl	%edx, %eax
	cmpl	$-958040967, %eax
	leal	1697253807(%rax), %r8d
	ja	.L9
	xorl	$-958040966, %eax
	movl	%eax, %r8d
	sall	$13, %eax
	xorl	%eax, %r8d
	movl	%r8d, %eax
	shrl	$17, %eax
	xorl	%eax, %r8d
	movl	%r8d, %eax
	sall	$5, %eax
	xorl	%eax, %r8d
.L9:
	cmpl	%r13d, %r8d
	je	.L10
	movq	stderr(%rip), %rdi
	leaq	.LC1(%rip), %rdx
	movl	%r13d, %ecx
	movl	$1, %esi
	xorl	%eax, %eax
	call	__fprintf_chk@PLT
.L10:
	movq	(%r12), %rax
	addq	$1, %rbp
	cmpq	%rbp, %rax
	jg	.L16
	shrq	$63, %rax
	testb	%al, %al
	jne	.L16
.L12:
	movl	$0, flg(%rip)
	mfence
	movq	8(%rsp), %rax
	xorq	%fs:40, %rax
	jne	.L20
	addq	$24, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 40
	popq	%rbx
	.cfi_def_cfa_offset 32
	popq	%rbp
	.cfi_def_cfa_offset 24
	popq	%r12
	.cfi_def_cfa_offset 16
	popq	%r13
	.cfi_def_cfa_offset 8
	ret
.L20:
	.cfi_restore_state
	call	__stack_chk_fail@PLT
	.cfi_endproc
.LFE83:
	.size	thread1, .-thread1
	.section	.rodata.str1.1
.LC2:
	.string	"mmap returns MAP_FAILED!\n"
	.text
	.p2align 4,,15
	.globl	threadx
	.type	threadx, @function
threadx:
.LFB84:
	.cfi_startproc
	pushq	%r12
	.cfi_def_cfa_offset 16
	.cfi_offset 12, -16
	pushq	%rbp
	.cfi_def_cfa_offset 24
	.cfi_offset 6, -24
	xorl	%r12d, %r12d
	pushq	%rbx
	.cfi_def_cfa_offset 32
	.cfi_offset 3, -32
	movl	flg(%rip), %eax
	testq	%rdi, %rdi
	setne	%r12b
	leaq	func_base(%rip), %rbx
	testl	%eax, %eax
	je	.L32
	.p2align 4,,10
	.p2align 3
.L27:
	call	random@PLT
	cqto
	shrq	$56, %rdx
	leaq	(%rax,%rdx), %rbp
	movzbl	%bpl, %ebp
	subq	%rdx, %rbp
	call	random@PLT
	testl	%r12d, %r12d
	movq	%rax, %rdx
	jne	.L23
	movl	$1, %ecx
	.p2align 4,,10
	.p2align 3
.L24:
	movl	%ecx, %eax
	xchgl	locked(%rip), %eax
	testl	%eax, %eax
	jne	.L24
	movq	func_set(%rip), %rax
.L26:
	leaq	8(%rax), %rdi
	movq	$0, 568(%rax)
	movq	$0, (%rax)
	andq	$-8, %rdi
	subq	%rdi, %rax
	leal	576(%rax), %ecx
	xorl	%eax, %eax
	shrl	$3, %ecx
	rep stosq
	movzbl	%bpl, %edi
	movq	func_set(%rip), %rax
	movq	(%rbx), %rsi
	addq	%rax, %rdi
	movq	%rsi, 64(%rdi)
	movq	243+func_base(%rip), %rsi
	leaq	64(%rdi), %rcx
	addq	$72, %rdi
	movq	%rsi, 235(%rdi)
	andq	$-8, %rdi
	movq	%rbx, %rsi
	subq	%rdi, %rcx
	subq	%rcx, %rsi
	addl	$251, %ecx
	shrl	$3, %ecx
	rep movsq
	movb	%bpl, 571(%rax)
	movl	%edx, 572(%rax)
#APP
# 152 "ryzen_segv_test.c" 1
	mfence
# 0 "" 2
#NO_APP
	xorl	%eax, %eax
	xorl	%r12d, %r12d
	call	lock_leave
	movl	flg(%rip), %eax
	testl	%eax, %eax
	jne	.L27
.L32:
	popq	%rbx
	.cfi_remember_state
	.cfi_def_cfa_offset 24
	popq	%rbp
	.cfi_def_cfa_offset 16
	popq	%r12
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L23:
	.cfi_restore_state
	movq	func_set(%rip), %rax
	cmpq	$-1, %rax
	jne	.L26
	popq	%rbx
	.cfi_def_cfa_offset 24
	popq	%rbp
	.cfi_def_cfa_offset 16
	popq	%r12
	.cfi_def_cfa_offset 8
	movq	stderr(%rip), %rcx
	leaq	.LC2(%rip), %rdi
	movl	$25, %edx
	movl	$1, %esi
	jmp	fwrite@PLT
	.cfi_endproc
.LFE84:
	.size	threadx, .-threadx
	.section	.rodata.str1.1
.LC3:
	.string	"PID:%d CPU:%d\n"
	.section	.text.startup,"ax",@progbits
	.p2align 4,,15
	.globl	main
	.type	main, @function
main:
.LFB85:
	.cfi_startproc
	pushq	%r12
	.cfi_def_cfa_offset 16
	.cfi_offset 12, -16
	pushq	%rbp
	.cfi_def_cfa_offset 24
	.cfi_offset 6, -24
	movq	%rsi, %r12
	pushq	%rbx
	.cfi_def_cfa_offset 32
	.cfi_offset 3, -32
	movl	%edi, %ebx
	subq	$176, %rsp
	.cfi_def_cfa_offset 208
	movq	%fs:40, %rax
	movq	%rax, 168(%rsp)
	xorl	%eax, %eax
	call	getpid@PLT
	cmpl	$1, %ebx
	movl	%eax, %ebp
	jle	.L34
	movq	8(%r12), %rdi
	movl	$10, %edx
	xorl	%esi, %esi
	call	strtoll@PLT
	movq	%rax, (%rsp)
.L35:
	movl	$84, %edi
	call	sysconf@PLT
	xorl	%r9d, %r9d
	movl	$-1, %r8d
	movl	$34, %ecx
	movl	$7, %edx
	movl	$576, %esi
	xorl	%edi, %edi
	movl	%eax, n_cpus(%rip)
	call	mmap@PLT
	movq	%rax, func_set(%rip)
	xorl	%edi, %edi
	movl	$1, flg(%rip)
	mfence
	movl	$1, locked(%rip)
	mfence
	call	time@PLT
	leal	0(%rbp,%rax), %edi
	call	srandom@PLT
#APP
# 152 "ryzen_segv_test.c" 1
	mfence
# 0 "" 2
#NO_APP
	leaq	thread1(%rip), %rdx
	leaq	8(%rsp), %rdi
	xorl	%esi, %esi
	movq	%rsp, %rcx
	call	pthread_create@PLT
	leaq	16(%rsp), %rdi
	leaq	threadx(%rip), %rdx
	xorl	%esi, %esi
	movl	$1, %ecx
	call	pthread_create@PLT
	leaq	24(%rsp), %rdi
	leaq	threadx(%rip), %rdx
	xorl	%ecx, %ecx
	xorl	%esi, %esi
	call	pthread_create@PLT
	call	random@PLT
	movslq	n_cpus(%rip), %rbx
	cqto
	movl	$16, %ecx
	idivq	%rbx
	xorl	%eax, %eax
	movq	%rdx, %rbx
	leaq	32(%rsp), %rdx
	cmpq	$1023, %rbx
	movq	%rdx, %rdi
	rep stosq
	ja	.L36
	movq	%rbx, %rsi
	movl	$1, %eax
	movl	%ebx, %ecx
	shrq	$6, %rsi
	salq	%cl, %rax
	orq	%rax, (%rdx,%rsi,8)
.L36:
	movl	$128, %esi
	movl	%ebp, %edi
	call	sched_setaffinity@PLT
	movq	stderr(%rip), %rdi
	leaq	.LC3(%rip), %rdx
	movl	%ebx, %r8d
	movl	%ebp, %ecx
	movl	$1, %esi
	xorl	%eax, %eax
	call	__fprintf_chk@PLT
	movq	8(%rsp), %rdi
	xorl	%esi, %esi
	call	pthread_join@PLT
	movq	16(%rsp), %rdi
	xorl	%esi, %esi
	call	pthread_join@PLT
	movq	24(%rsp), %rdi
	xorl	%esi, %esi
	call	pthread_join@PLT
	xorl	%eax, %eax
	movq	168(%rsp), %rbx
	xorq	%fs:40, %rbx
	jne	.L39
	addq	$176, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 32
	popq	%rbx
	.cfi_def_cfa_offset 24
	popq	%rbp
	.cfi_def_cfa_offset 16
	popq	%r12
	.cfi_def_cfa_offset 8
	ret
.L34:
	.cfi_restore_state
	movq	$-1, (%rsp)
	jmp	.L35
.L39:
	call	__stack_chk_fail@PLT
	.cfi_endproc
.LFE85:
	.size	main, .-main
	.globl	n_cpus
	.data
	.align 4
	.type	n_cpus, @object
	.size	n_cpus, 4
n_cpus:
	.long	16
	.globl	func_base
	.align 32
	.type	func_base, @object
	.size	func_base, 251
func_base:
	.byte	-117
	.byte	-105
	.byte	60
	.byte	2
	.byte	0
	.byte	0
	.byte	-119
	.byte	-48
	.byte	-63
	.byte	-32
	.byte	13
	.byte	49
	.byte	-62
	.byte	-119
	.byte	-48
	.byte	-63
	.byte	-24
	.byte	17
	.byte	49
	.byte	-62
	.byte	-119
	.byte	-48
	.byte	-63
	.byte	-32
	.byte	5
	.byte	49
	.byte	-62
	.byte	-127
	.byte	-6
	.byte	121
	.byte	116
	.byte	-27
	.byte	-58
	.byte	-115
	.byte	-126
	.byte	-81
	.byte	9
	.byte	42
	.byte	101
	.byte	118
	.byte	7
	.byte	-13
	.byte	-61
	.byte	15
	.byte	31
	.byte	68
	.byte	0
	.byte	0
	.byte	-119
	.byte	-48
	.byte	53
	.byte	122
	.byte	116
	.byte	-27
	.byte	-58
	.byte	-119
	.byte	-62
	.byte	-63
	.byte	-30
	.byte	13
	.byte	49
	.byte	-48
	.byte	-119
	.byte	-62
	.byte	-63
	.byte	-22
	.byte	17
	.byte	49
	.byte	-48
	.byte	-119
	.byte	-62
	.byte	-63
	.byte	-30
	.byte	5
	.byte	49
	.byte	-48
	.byte	-61
	.byte	15
	.byte	31
	.byte	0
	.zero	171
	.comm	locked,4,4
	.comm	flg,4,4
	.comm	func_set,8,8
	.ident	"GCC: (Ubuntu 6.3.0-12ubuntu2) 6.3.0 20170406"
	.section	.note.GNU-stack,"",@progbits
