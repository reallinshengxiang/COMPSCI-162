	.file	"stdlib.c"
	.text
.Ltext0:
	.type	freelist_insert, @function
freelist_insert:
.LFB0:
	.file 1 "../../lib/user/stdlib.c"
	.loc 1 16 0
	.cfi_startproc
.LVL0:
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	.loc 1 17 0
	movl	8(%eax), %ecx
	movl	%ecx, 8(%edx)
	.loc 1 18 0
	movl	%eax, 12(%edx)
	.loc 1 19 0
	movl	8(%eax), %ecx
	testl	%ecx, %ecx
	je	.L2
	.loc 1 20 0
	movl	%edx, 12(%ecx)
.L2:
	.loc 1 22 0
	movl	%edx, 8(%eax)
	.loc 1 23 0
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE0:
	.size	freelist_insert, .-freelist_insert
	.type	freelist_remove, @function
freelist_remove:
.LFB1:
	.loc 1 25 0
	.cfi_startproc
.LVL1:
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	.loc 1 26 0
	movl	12(%eax), %edx
	testl	%edx, %edx
	je	.L5
	.loc 1 27 0
	movl	8(%eax), %ecx
	movl	%ecx, 8(%edx)
.L5:
	.loc 1 29 0
	movl	8(%eax), %edx
	testl	%edx, %edx
	je	.L6
	.loc 1 30 0
	movl	12(%eax), %ecx
	movl	%ecx, 12(%edx)
.L6:
	.loc 1 32 0
	cmpl	%eax, freelist
	je	.L9
.LVL2:
.L4:
	.loc 1 35 0
	popl	%ebp
	.cfi_remember_state
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
.LVL3:
.L9:
	.cfi_restore_state
	.loc 1 33 0
	movl	8(%eax), %eax
.LVL4:
	movl	%eax, freelist
	.loc 1 35 0
	jmp	.L4
	.cfi_endproc
.LFE1:
	.size	freelist_remove, .-freelist_remove
	.globl	malloc
	.type	malloc, @function
malloc:
.LFB2:
	.loc 1 37 0
	.cfi_startproc
.LVL5:
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$28, %esp
	.cfi_offset 7, -12
	.cfi_offset 6, -16
	.cfi_offset 3, -20
	movl	8(%ebp), %esi
.LVL6:
	.loc 1 41 0
	testl	%esi, %esi
	je	.L23
	.loc 1 45 0
	movl	freelist, %ebx
.LVL7:
	testl	%ebx, %ebx
	jne	.L15
.LVL8:
	.loc 1 77 0
	leal	16(%esi), %eax
	movl	%eax, (%esp)
	call	sbrk
.LVL9:
	movl	%eax, %edi
.LVL10:
	.loc 1 78 0
	cmpl	$-1, %eax
	je	.L33
	.loc 1 81 0
	movb	$0, (%edi)
	.loc 1 82 0
	movl	%esi, 4(%edi)
.LVL11:
.L21:
	.loc 1 86 0
	movl	$0, 12(%edi)
	.loc 1 87 0
	movl	$0, 8(%edi)
	.loc 1 88 0
	movl	%edi, freelist
	jmp	.L19
.LVL12:
.L35:
	.loc 1 52 0
	addl	%ebx, %edx
.LVL13:
	.loc 1 53 0
	movb	$1, (%edx)
	.loc 1 54 0
	movl	$-16, %eax
	subl	%esi, %eax
	addl	4(%ebx), %eax
	movl	%eax, 4(%edx)
	.loc 1 55 0
	movl	%esi, 4(%ebx)
	.loc 1 56 0
	movl	%ebx, %eax
	call	freelist_insert
.LVL14:
	jmp	.L14
.LVL15:
.L13:
	.loc 1 45 0 discriminator 2
	movl	8(%ebx), %eax
.LVL16:
	testl	%eax, %eax
	je	.L34
.LVL17:
	.loc 1 45 0 is_stmt 0
	movl	%eax, %ebx
.LVL18:
.L15:
	.loc 1 47 0 is_stmt 1
	movzbl	(%ebx), %edx
	testb	%dl, %dl
	je	.L13
	.loc 1 47 0 is_stmt 0 discriminator 1
	movl	4(%ebx), %eax
	cmpl	%esi, %eax
	jb	.L13
	.loc 1 50 0 is_stmt 1
	leal	16(%esi), %edx
	cmpl	%edx, %eax
	ja	.L35
.LVL19:
.L14:
	.loc 1 59 0
	movb	$0, (%ebx)
	.loc 1 61 0
	leal	16(%ebx), %eax
	jmp	.L10
.LVL20:
.L34:
	.loc 1 66 0
	testl	%ebx, %ebx
	je	.L16
	.loc 1 66 0 is_stmt 0 discriminator 1
	testb	%dl, %dl
	jne	.L36
.LVL21:
	.loc 1 77 0 is_stmt 1
	leal	16(%esi), %eax
	movl	%eax, (%esp)
	call	sbrk
.LVL22:
	movl	%eax, %edi
.LVL23:
	.loc 1 78 0
	cmpl	$-1, %eax
	je	.L37
	.loc 1 81 0
	movb	$0, (%edi)
	.loc 1 82 0
	movl	%esi, 4(%edi)
	.loc 1 84 0
	movl	%edi, %edx
	movl	%ebx, %eax
.LVL24:
	call	freelist_insert
.LVL25:
.L19:
	.loc 1 90 0
	leal	16(%edi), %eax
.LVL26:
.L10:
	.loc 1 92 0
	addl	$28, %esp
	popl	%ebx
	.cfi_remember_state
	.cfi_restore 3
	popl	%esi
	.cfi_restore 6
	popl	%edi
	.cfi_restore 7
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
.LVL27:
.L36:
	.cfi_restore_state
	.loc 1 68 0
	movl	%esi, %eax
.LVL28:
	subl	4(%ebx), %eax
	movl	%eax, (%esp)
.LVL29:
	call	sbrk
.LVL30:
	cmpl	$-1, %eax
	je	.L25
	.loc 1 71 0
	movb	$0, (%ebx)
	.loc 1 72 0
	movl	%esi, 4(%ebx)
	.loc 1 73 0
	leal	16(%ebx), %eax
	jmp	.L10
.LVL31:
.L23:
	.loc 1 42 0
	movl	$0, %eax
	jmp	.L10
.LVL32:
.L25:
	.loc 1 69 0
	movl	$0, %eax
	jmp	.L10
.LVL33:
.L33:
	.loc 1 79 0
	movl	$0, %eax
.LVL34:
	jmp	.L10
.LVL35:
.L37:
	movl	$0, %eax
.LVL36:
	jmp	.L10
.LVL37:
.L38:
	movl	$0, %eax
.LVL38:
	jmp	.L10
.LVL39:
.L16:
	.loc 1 77 0
	leal	16(%esi), %eax
	movl	%eax, (%esp)
	call	sbrk
.LVL40:
	movl	%eax, %edi
.LVL41:
	.loc 1 78 0
	cmpl	$-1, %eax
	je	.L38
	.loc 1 81 0
	movb	$0, (%edi)
	.loc 1 82 0
	movl	%esi, 4(%edi)
	jmp	.L21
	.cfi_endproc
.LFE2:
	.size	malloc, .-malloc
	.globl	free
	.type	free, @function
free:
.LFB3:
	.loc 1 94 0
	.cfi_startproc
.LVL42:
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%ebx
	.cfi_offset 3, -12
	movl	8(%ebp), %ebx
.LVL43:
	.loc 1 96 0
	testl	%ebx, %ebx
	je	.L39
	.loc 1 99 0
	movb	$1, -16(%ebx)
	.loc 1 100 0
	movl	-8(%ebx), %eax
	testl	%eax, %eax
	je	.L41
	.loc 1 100 0 is_stmt 0 discriminator 1
	cmpb	$0, (%eax)
	jne	.L43
.L41:
	.loc 1 105 0 is_stmt 1
	movl	-4(%ebx), %eax
	testl	%eax, %eax
	je	.L39
	.loc 1 105 0 is_stmt 0 discriminator 1
	cmpb	$0, (%eax)
	jne	.L44
.LVL44:
.L39:
	.loc 1 110 0 is_stmt 1
	popl	%ebx
	.cfi_remember_state
	.cfi_restore 3
.LVL45:
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	.p2align 4,,1
	ret
.LVL46:
.L43:
	.cfi_restore_state
	.loc 1 102 0
	movl	-12(%ebx), %ecx
	movl	4(%eax), %edx
	leal	16(%ecx,%edx), %edx
	movl	%edx, -12(%ebx)
	.loc 1 103 0
	call	freelist_remove
.LVL47:
	jmp	.L41
.LVL48:
.L44:
	.loc 1 107 0
	movl	-12(%ebx), %ecx
	leal	16(%ecx), %edx
	addl	%edx, 4(%eax)
	.loc 1 95 0
	leal	-16(%ebx), %eax
	.loc 1 108 0
	call	freelist_remove
.LVL49:
	jmp	.L39
	.cfi_endproc
.LFE3:
	.size	free, .-free
	.globl	calloc
	.type	calloc, @function
calloc:
.LFB4:
	.loc 1 112 0
	.cfi_startproc
.LVL50:
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$40, %esp
	movl	%ebx, -12(%ebp)
	movl	%esi, -8(%ebp)
	movl	%edi, -4(%ebp)
	.cfi_offset 3, -20
	.cfi_offset 6, -16
	.cfi_offset 7, -12
	.loc 1 113 0
	movl	12(%ebp), %ebx
	imull	8(%ebp), %ebx
	movl	%ebx, (%esp)
	call	malloc
.LVL51:
	movl	%eax, %edx
.LVL52:
	.loc 1 114 0
	testl	%eax, %eax
	je	.L45
	.loc 1 115 0
	movl	%ebx, %esi
	movl	%eax, %edi
	cmpl	$8, %ebx
	jnb	.L53
.LVL53:
.L47:
	testl	$4, %esi
	je	.L49
	movl	$0, (%edi)
	addl	$4, %edi
.L49:
	testl	$2, %esi
	je	.L50
	movw	$0, (%edi)
	addl	$2, %edi
.L50:
	testl	$1, %esi
	je	.L45
	movb	$0, (%edi)
.L45:
	.loc 1 118 0
	movl	%edx, %eax
	movl	-12(%ebp), %ebx
	movl	-8(%ebp), %esi
	movl	-4(%ebp), %edi
	movl	%ebp, %esp
	.cfi_remember_state
	.cfi_restore 7
	.cfi_restore 6
	.cfi_restore 3
	.cfi_def_cfa_register 4
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
.LVL54:
.L53:
	.cfi_restore_state
	.loc 1 115 0
	testb	$4, %al
	jne	.L54
.LVL55:
.L48:
	movl	%esi, %ecx
	shrl	$2, %ecx
	movl	$0, %eax
.LVL56:
	rep stosl
	andl	$3, %esi
	jmp	.L47
.LVL57:
.L54:
	movl	$0, (%eax)
.LVL58:
	leal	4(%eax), %edi
	leal	-4(%ebx), %esi
	jmp	.L48
	.cfi_endproc
.LFE4:
	.size	calloc, .-calloc
	.globl	realloc
	.type	realloc, @function
realloc:
.LFB5:
	.loc 1 120 0
	.cfi_startproc
.LVL59:
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$40, %esp
	movl	%ebx, -12(%ebp)
	movl	%esi, -8(%ebp)
	movl	%edi, -4(%ebp)
	.cfi_offset 3, -20
	.cfi_offset 6, -16
	.cfi_offset 7, -12
	movl	12(%ebp), %esi
	.loc 1 122 0
	cmpl	$0, 8(%ebp)
	je	.L62
	.loc 1 124 0
	testl	%esi, %esi
	je	.L63
	.loc 1 129 0
	movl	%esi, (%esp)
	call	malloc
.LVL60:
	movl	%eax, %ebx
.LVL61:
	.loc 1 130 0
	testl	%eax, %eax
	je	.L55
	.loc 1 134 0
	movl	%esi, %ecx
	movl	%eax, %edi
	movl	8(%ebp), %eax
.LVL62:
	cmpl	$8, %esi
	jnb	.L64
.LVL63:
.L59:
	movl	%eax, %esi
	rep movsb
	.loc 1 135 0
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	free
.LVL64:
.L55:
	.loc 1 138 0
	movl	%ebx, %eax
	movl	-12(%ebp), %ebx
	movl	-8(%ebp), %esi
	movl	-4(%ebp), %edi
	movl	%ebp, %esp
	.cfi_remember_state
	.cfi_restore 7
	.cfi_restore 6
	.cfi_restore 3
	.cfi_def_cfa_register 4
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
.LVL65:
.L62:
	.cfi_restore_state
	.loc 1 123 0
	movl	%esi, (%esp)
	call	malloc
.LVL66:
	movl	%eax, %ebx
	jmp	.L55
.L63:
	.loc 1 125 0
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	free
.LVL67:
	.loc 1 126 0
	movl	$0, %ebx
	jmp	.L55
.LVL68:
.L64:
	.loc 1 134 0
	testb	$4, %bl
	je	.L59
	movl	(%eax), %eax
	movl	%eax, (%ebx)
.LVL69:
	leal	4(%ebx), %edi
	movl	8(%ebp), %eax
	leal	4(%eax), %eax
	leal	-4(%esi), %ecx
	jmp	.L59
	.cfi_endproc
.LFE5:
	.size	realloc, .-realloc
	.local	freelist
	.comm	freelist,4,4
.Letext0:
	.file 2 "../../lib/stddef.h"
	.file 3 "../../lib/stdint.h"
	.file 4 "../../lib/user/syscall.h"
	.section	.debug_info,"",@progbits
.Ldebug_info0:
	.long	0x32e
	.value	0x4
	.long	.Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1
	.long	.LASF28
	.byte	0xc
	.long	.LASF29
	.long	.LASF30
	.long	.Ltext0
	.long	.Letext0-.Ltext0
	.long	.Ldebug_line0
	.uleb128 0x2
	.byte	0x4
	.byte	0x5
	.string	"int"
	.uleb128 0x3
	.long	.LASF4
	.byte	0x2
	.byte	0xa
	.long	0x37
	.uleb128 0x4
	.byte	0x4
	.byte	0x7
	.long	.LASF0
	.uleb128 0x4
	.byte	0x1
	.byte	0x6
	.long	.LASF1
	.uleb128 0x4
	.byte	0x2
	.byte	0x5
	.long	.LASF2
	.uleb128 0x4
	.byte	0x8
	.byte	0x5
	.long	.LASF3
	.uleb128 0x3
	.long	.LASF5
	.byte	0x3
	.byte	0x14
	.long	0x5e
	.uleb128 0x4
	.byte	0x1
	.byte	0x8
	.long	.LASF6
	.uleb128 0x4
	.byte	0x2
	.byte	0x7
	.long	.LASF7
	.uleb128 0x4
	.byte	0x8
	.byte	0x7
	.long	.LASF8
	.uleb128 0x5
	.long	.LASF31
	.byte	0x10
	.byte	0x1
	.byte	0x7
	.long	0xb0
	.uleb128 0x6
	.long	.LASF9
	.byte	0x1
	.byte	0x8
	.long	0xb0
	.byte	0
	.uleb128 0x6
	.long	.LASF10
	.byte	0x1
	.byte	0x9
	.long	0x2c
	.byte	0x4
	.uleb128 0x6
	.long	.LASF11
	.byte	0x1
	.byte	0xa
	.long	0xb7
	.byte	0x8
	.uleb128 0x6
	.long	.LASF12
	.byte	0x1
	.byte	0xb
	.long	0xb7
	.byte	0xc
	.byte	0
	.uleb128 0x4
	.byte	0x1
	.byte	0x2
	.long	.LASF13
	.uleb128 0x7
	.byte	0x4
	.long	0x73
	.uleb128 0x8
	.long	.LASF14
	.byte	0x1
	.byte	0xd
	.long	0xb7
	.uleb128 0x5
	.byte	0x3
	.long	freelist
	.uleb128 0x9
	.long	.LASF16
	.byte	0x1
	.byte	0x78
	.long	0x167
	.long	.LFB5
	.long	.LFE5-.LFB5
	.uleb128 0x1
	.byte	0x9c
	.long	0x167
	.uleb128 0xa
	.string	"ptr"
	.byte	0x1
	.byte	0x78
	.long	0x167
	.long	.LLST8
	.uleb128 0xb
	.long	.LASF10
	.byte	0x1
	.byte	0x78
	.long	0x2c
	.long	.LLST9
	.uleb128 0xc
	.long	.LASF15
	.byte	0x1
	.byte	0x79
	.long	0x167
	.long	.LLST10
	.uleb128 0xd
	.long	.LVL60
	.long	0x20e
	.long	0x129
	.uleb128 0xe
	.uleb128 0x2
	.byte	0x74
	.sleb128 0
	.uleb128 0x2
	.byte	0x76
	.sleb128 0
	.byte	0
	.uleb128 0xd
	.long	.LVL64
	.long	0x1c1
	.long	0x13f
	.uleb128 0xe
	.uleb128 0x2
	.byte	0x74
	.sleb128 0
	.uleb128 0x3
	.byte	0x91
	.sleb128 0
	.byte	0x6
	.byte	0
	.uleb128 0xd
	.long	.LVL66
	.long	0x20e
	.long	0x154
	.uleb128 0xe
	.uleb128 0x2
	.byte	0x74
	.sleb128 0
	.uleb128 0x2
	.byte	0x76
	.sleb128 0
	.byte	0
	.uleb128 0xf
	.long	.LVL67
	.long	0x1c1
	.uleb128 0xe
	.uleb128 0x2
	.byte	0x74
	.sleb128 0
	.uleb128 0x3
	.byte	0x91
	.sleb128 0
	.byte	0x6
	.byte	0
	.byte	0
	.uleb128 0x10
	.byte	0x4
	.uleb128 0x9
	.long	.LASF17
	.byte	0x1
	.byte	0x70
	.long	0x167
	.long	.LFB4
	.long	.LFE4-.LFB4
	.uleb128 0x1
	.byte	0x9c
	.long	0x1c1
	.uleb128 0xb
	.long	.LASF18
	.byte	0x1
	.byte	0x70
	.long	0x2c
	.long	.LLST5
	.uleb128 0xb
	.long	.LASF10
	.byte	0x1
	.byte	0x70
	.long	0x2c
	.long	.LLST6
	.uleb128 0x11
	.string	"ptr"
	.byte	0x1
	.byte	0x71
	.long	0x167
	.long	.LLST7
	.uleb128 0xf
	.long	.LVL51
	.long	0x20e
	.uleb128 0xe
	.uleb128 0x2
	.byte	0x74
	.sleb128 0
	.uleb128 0x2
	.byte	0x73
	.sleb128 0
	.byte	0
	.byte	0
	.uleb128 0x12
	.long	.LASF9
	.byte	0x1
	.byte	0x5e
	.long	.LFB3
	.long	.LFE3-.LFB3
	.uleb128 0x1
	.byte	0x9c
	.long	0x20e
	.uleb128 0xa
	.string	"ptr"
	.byte	0x1
	.byte	0x5e
	.long	0x167
	.long	.LLST3
	.uleb128 0xc
	.long	.LASF19
	.byte	0x1
	.byte	0x5f
	.long	0xb7
	.long	.LLST4
	.uleb128 0x13
	.long	.LVL47
	.long	0x2d1
	.uleb128 0xf
	.long	.LVL49
	.long	0x2d1
	.uleb128 0xe
	.uleb128 0x1
	.byte	0x50
	.uleb128 0x2
	.byte	0x73
	.sleb128 -16
	.byte	0
	.byte	0
	.uleb128 0x9
	.long	.LASF20
	.byte	0x1
	.byte	0x25
	.long	0x167
	.long	.LFB2
	.long	.LFE2-.LFB2
	.uleb128 0x1
	.byte	0x9c
	.long	0x2d1
	.uleb128 0x14
	.long	.LASF10
	.byte	0x1
	.byte	0x25
	.long	0x2c
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0xc
	.long	.LASF21
	.byte	0x1
	.byte	0x26
	.long	0xb7
	.long	.LLST1
	.uleb128 0xc
	.long	.LASF22
	.byte	0x1
	.byte	0x27
	.long	0xb7
	.long	.LLST2
	.uleb128 0xd
	.long	.LVL9
	.long	0x326
	.long	0x268
	.uleb128 0xe
	.uleb128 0x2
	.byte	0x74
	.sleb128 0
	.uleb128 0x2
	.byte	0x76
	.sleb128 16
	.byte	0
	.uleb128 0xd
	.long	.LVL14
	.long	0x2f6
	.long	0x287
	.uleb128 0xe
	.uleb128 0x1
	.byte	0x50
	.uleb128 0x2
	.byte	0x73
	.sleb128 0
	.uleb128 0xe
	.uleb128 0x1
	.byte	0x52
	.uleb128 0x7
	.byte	0x73
	.sleb128 0
	.byte	0x76
	.sleb128 0
	.byte	0x22
	.byte	0x23
	.uleb128 0x10
	.byte	0
	.uleb128 0xd
	.long	.LVL22
	.long	0x326
	.long	0x29c
	.uleb128 0xe
	.uleb128 0x2
	.byte	0x74
	.sleb128 0
	.uleb128 0x2
	.byte	0x76
	.sleb128 16
	.byte	0
	.uleb128 0xd
	.long	.LVL25
	.long	0x2f6
	.long	0x2b6
	.uleb128 0xe
	.uleb128 0x1
	.byte	0x50
	.uleb128 0x2
	.byte	0x73
	.sleb128 0
	.uleb128 0xe
	.uleb128 0x1
	.byte	0x52
	.uleb128 0x2
	.byte	0x77
	.sleb128 0
	.byte	0
	.uleb128 0x13
	.long	.LVL30
	.long	0x326
	.uleb128 0xf
	.long	.LVL40
	.long	0x326
	.uleb128 0xe
	.uleb128 0x2
	.byte	0x74
	.sleb128 0
	.uleb128 0x2
	.byte	0x76
	.sleb128 16
	.byte	0
	.byte	0
	.uleb128 0x15
	.long	.LASF24
	.byte	0x1
	.byte	0x19
	.long	.LFB1
	.long	.LFE1-.LFB1
	.uleb128 0x1
	.byte	0x9c
	.long	0x2f6
	.uleb128 0xb
	.long	.LASF23
	.byte	0x1
	.byte	0x19
	.long	0xb7
	.long	.LLST0
	.byte	0
	.uleb128 0x15
	.long	.LASF25
	.byte	0x1
	.byte	0xf
	.long	.LFB0
	.long	.LFE0-.LFB0
	.uleb128 0x1
	.byte	0x9c
	.long	0x326
	.uleb128 0x14
	.long	.LASF26
	.byte	0x1
	.byte	0xf
	.long	0xb7
	.uleb128 0x1
	.byte	0x50
	.uleb128 0x14
	.long	.LASF27
	.byte	0x1
	.byte	0x10
	.long	0xb7
	.uleb128 0x1
	.byte	0x52
	.byte	0
	.uleb128 0x16
	.long	.LASF32
	.long	.LASF32
	.byte	0x4
	.byte	0x33
	.byte	0
	.section	.debug_abbrev,"",@progbits
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1b
	.uleb128 0xe
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x10
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0
	.byte	0
	.uleb128 0x3
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x4
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0
	.byte	0
	.uleb128 0x5
	.uleb128 0x13
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x6
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x9
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2117
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xa
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0xb
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0xc
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0xd
	.uleb128 0x4109
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xe
	.uleb128 0x410a
	.byte	0
	.uleb128 0x2
	.uleb128 0x18
	.uleb128 0x2111
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0xf
	.uleb128 0x4109
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x31
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x10
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x11
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x12
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2117
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x13
	.uleb128 0x4109
	.byte	0
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x31
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x14
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x15
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2117
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x16
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3c
	.uleb128 0x19
	.uleb128 0x6e
	.uleb128 0xe
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_loc,"",@progbits
.Ldebug_loc0:
.LLST8:
	.long	.LVL59-.Ltext0
	.long	.LVL63-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	.LVL65-.Ltext0
	.long	.LVL69-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST9:
	.long	.LVL59-.Ltext0
	.long	.LVL63-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 4
	.long	.LVL65-.Ltext0
	.long	.LVL69-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 4
	.long	0
	.long	0
.LLST10:
	.long	.LVL61-.Ltext0
	.long	.LVL62-.Ltext0
	.value	0x1
	.byte	0x50
	.long	.LVL62-.Ltext0
	.long	.LVL64-.Ltext0
	.value	0x1
	.byte	0x53
	.long	.LVL68-.Ltext0
	.long	.LFE5-.Ltext0
	.value	0x1
	.byte	0x53
	.long	0
	.long	0
.LLST5:
	.long	.LVL50-.Ltext0
	.long	.LVL53-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	.LVL54-.Ltext0
	.long	.LVL55-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	.LVL57-.Ltext0
	.long	.LVL58-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST6:
	.long	.LVL50-.Ltext0
	.long	.LVL53-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 4
	.long	.LVL54-.Ltext0
	.long	.LVL55-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 4
	.long	.LVL57-.Ltext0
	.long	.LVL58-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 4
	.long	0
	.long	0
.LLST7:
	.long	.LVL52-.Ltext0
	.long	.LVL53-.Ltext0
	.value	0x1
	.byte	0x50
	.long	.LVL53-.Ltext0
	.long	.LVL54-.Ltext0
	.value	0x1
	.byte	0x52
	.long	.LVL54-.Ltext0
	.long	.LVL56-.Ltext0
	.value	0x1
	.byte	0x50
	.long	.LVL56-.Ltext0
	.long	.LVL57-.Ltext0
	.value	0x1
	.byte	0x52
	.long	.LVL57-.Ltext0
	.long	.LFE4-.Ltext0
	.value	0x1
	.byte	0x50
	.long	0
	.long	0
.LLST3:
	.long	.LVL42-.Ltext0
	.long	.LVL46-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	.LVL46-.Ltext0
	.long	.LVL48-.Ltext0
	.value	0x1
	.byte	0x53
	.long	.LVL48-.Ltext0
	.long	.LFE3-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 0
	.long	0
	.long	0
.LLST4:
	.long	.LVL43-.Ltext0
	.long	.LVL44-.Ltext0
	.value	0x6
	.byte	0x91
	.sleb128 0
	.byte	0x6
	.byte	0x40
	.byte	0x1c
	.byte	0x9f
	.long	.LVL44-.Ltext0
	.long	.LVL45-.Ltext0
	.value	0x3
	.byte	0x73
	.sleb128 -16
	.byte	0x9f
	.long	.LVL45-.Ltext0
	.long	.LVL46-.Ltext0
	.value	0x6
	.byte	0x91
	.sleb128 0
	.byte	0x6
	.byte	0x40
	.byte	0x1c
	.byte	0x9f
	.long	.LVL46-.Ltext0
	.long	.LFE3-.Ltext0
	.value	0x3
	.byte	0x73
	.sleb128 -16
	.byte	0x9f
	.long	0
	.long	0
.LLST1:
	.long	.LVL7-.Ltext0
	.long	.LVL8-.Ltext0
	.value	0x1
	.byte	0x53
	.long	.LVL8-.Ltext0
	.long	.LVL11-.Ltext0
	.value	0x2
	.byte	0x30
	.byte	0x9f
	.long	.LVL12-.Ltext0
	.long	.LVL16-.Ltext0
	.value	0x1
	.byte	0x53
	.long	.LVL16-.Ltext0
	.long	.LVL18-.Ltext0
	.value	0x1
	.byte	0x50
	.long	.LVL18-.Ltext0
	.long	.LVL20-.Ltext0
	.value	0x1
	.byte	0x53
	.long	.LVL20-.Ltext0
	.long	.LVL21-.Ltext0
	.value	0x1
	.byte	0x50
	.long	.LVL21-.Ltext0
	.long	.LVL25-.Ltext0
	.value	0x1
	.byte	0x53
	.long	.LVL27-.Ltext0
	.long	.LVL28-.Ltext0
	.value	0x1
	.byte	0x50
	.long	.LVL28-.Ltext0
	.long	.LVL29-.Ltext0
	.value	0x2
	.byte	0x73
	.sleb128 8
	.long	.LVL33-.Ltext0
	.long	.LVL35-.Ltext0
	.value	0x2
	.byte	0x30
	.byte	0x9f
	.long	.LVL35-.Ltext0
	.long	.LFE2-.Ltext0
	.value	0x1
	.byte	0x53
	.long	0
	.long	0
.LLST2:
	.long	.LVL6-.Ltext0
	.long	.LVL10-.Ltext0
	.value	0x2
	.byte	0x30
	.byte	0x9f
	.long	.LVL10-.Ltext0
	.long	.LVL12-.Ltext0
	.value	0x1
	.byte	0x50
	.long	.LVL12-.Ltext0
	.long	.LVL13-.Ltext0
	.value	0x1
	.byte	0x53
	.long	.LVL13-.Ltext0
	.long	.LVL14-1-.Ltext0
	.value	0x1
	.byte	0x52
	.long	.LVL14-1-.Ltext0
	.long	.LVL15-.Ltext0
	.value	0x8
	.byte	0x73
	.sleb128 0
	.byte	0x76
	.sleb128 0
	.byte	0x22
	.byte	0x23
	.uleb128 0x10
	.byte	0x9f
	.long	.LVL15-.Ltext0
	.long	.LVL17-.Ltext0
	.value	0x1
	.byte	0x53
	.long	.LVL17-.Ltext0
	.long	.LVL18-.Ltext0
	.value	0x1
	.byte	0x50
	.long	.LVL18-.Ltext0
	.long	.LVL19-.Ltext0
	.value	0x1
	.byte	0x53
	.long	.LVL20-.Ltext0
	.long	.LVL23-.Ltext0
	.value	0x1
	.byte	0x53
	.long	.LVL23-.Ltext0
	.long	.LVL24-.Ltext0
	.value	0x1
	.byte	0x50
	.long	.LVL24-.Ltext0
	.long	.LVL25-1-.Ltext0
	.value	0x1
	.byte	0x52
	.long	.LVL25-1-.Ltext0
	.long	.LVL26-.Ltext0
	.value	0x1
	.byte	0x57
	.long	.LVL27-.Ltext0
	.long	.LVL31-.Ltext0
	.value	0x1
	.byte	0x53
	.long	.LVL31-.Ltext0
	.long	.LVL32-.Ltext0
	.value	0x2
	.byte	0x30
	.byte	0x9f
	.long	.LVL32-.Ltext0
	.long	.LVL33-.Ltext0
	.value	0x1
	.byte	0x53
	.long	.LVL33-.Ltext0
	.long	.LVL34-.Ltext0
	.value	0x1
	.byte	0x50
	.long	.LVL34-.Ltext0
	.long	.LVL35-.Ltext0
	.value	0x1
	.byte	0x57
	.long	.LVL35-.Ltext0
	.long	.LVL36-.Ltext0
	.value	0x1
	.byte	0x50
	.long	.LVL36-.Ltext0
	.long	.LVL37-.Ltext0
	.value	0x1
	.byte	0x57
	.long	.LVL37-.Ltext0
	.long	.LVL38-.Ltext0
	.value	0x1
	.byte	0x50
	.long	.LVL38-.Ltext0
	.long	.LVL39-.Ltext0
	.value	0x1
	.byte	0x57
	.long	.LVL39-.Ltext0
	.long	.LVL41-.Ltext0
	.value	0x1
	.byte	0x53
	.long	.LVL41-.Ltext0
	.long	.LFE2-.Ltext0
	.value	0x1
	.byte	0x50
	.long	0
	.long	0
.LLST0:
	.long	.LVL1-.Ltext0
	.long	.LVL2-.Ltext0
	.value	0x1
	.byte	0x50
	.long	.LVL2-.Ltext0
	.long	.LVL3-.Ltext0
	.value	0x4
	.byte	0xf3
	.uleb128 0x1
	.byte	0x50
	.byte	0x9f
	.long	.LVL3-.Ltext0
	.long	.LVL4-.Ltext0
	.value	0x1
	.byte	0x50
	.long	.LVL4-.Ltext0
	.long	.LFE1-.Ltext0
	.value	0x4
	.byte	0xf3
	.uleb128 0x1
	.byte	0x50
	.byte	0x9f
	.long	0
	.long	0
	.section	.debug_aranges,"",@progbits
	.long	0x1c
	.value	0x2
	.long	.Ldebug_info0
	.byte	0x4
	.byte	0
	.value	0
	.value	0
	.long	.Ltext0
	.long	.Letext0-.Ltext0
	.long	0
	.long	0
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.section	.debug_str,"MS",@progbits,1
.LASF17:
	.string	"calloc"
.LASF4:
	.string	"size_t"
.LASF11:
	.string	"next"
.LASF30:
	.string	"/home/vagrant/code/course-material/hw-skeleton/hw-memory/spring-2020/hw6-sol/hw6-pintos-sol/src/memory/build"
.LASF23:
	.string	"toremove"
.LASF20:
	.string	"malloc"
.LASF27:
	.string	"newnode"
.LASF5:
	.string	"uint8_t"
.LASF6:
	.string	"unsigned char"
.LASF7:
	.string	"short unsigned int"
.LASF26:
	.string	"prevnode"
.LASF16:
	.string	"realloc"
.LASF29:
	.string	"../../lib/user/stdlib.c"
.LASF0:
	.string	"unsigned int"
.LASF8:
	.string	"long long unsigned int"
.LASF28:
	.string	"GNU C11 7.5.0 -m32 -msoft-float -march=i686 -g -O -fno-pic -fno-inline -fno-omit-frame-pointer -fno-stack-protector"
.LASF15:
	.string	"newptr"
.LASF21:
	.string	"currblk"
.LASF9:
	.string	"free"
.LASF3:
	.string	"long long int"
.LASF24:
	.string	"freelist_remove"
.LASF32:
	.string	"sbrk"
.LASF2:
	.string	"short int"
.LASF14:
	.string	"freelist"
.LASF19:
	.string	"tofree"
.LASF25:
	.string	"freelist_insert"
.LASF1:
	.string	"signed char"
.LASF22:
	.string	"newblk"
.LASF10:
	.string	"size"
.LASF13:
	.string	"_Bool"
.LASF12:
	.string	"prev"
.LASF31:
	.string	"freeblock"
.LASF18:
	.string	"nmemb"
	.ident	"GCC: (Ubuntu 7.5.0-3ubuntu1~18.04) 7.5.0"
	.section	.note.GNU-stack,"",@progbits
