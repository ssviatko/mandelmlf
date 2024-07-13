	global iterate8 

	section .text

iterate8:
	push rbx
	push rbp
	mov rbp, rsp
	sub rsp, 96

	mov r8, rdi			;Save our pointer to cr's
	mov r9, rsi			;Save our pointer to ci's

	vmovups ymm0, [r8]
	vmovups ymm1, [r9]		;Load up C's
	mov rbx, const_0
	vmovaps ymm2, [rbx]		;zr = 0
	vmovaps ymm3, ymm2		;zi = 0
	vmovaps ymm4, ymm2		;Set up zr^2
	vmovaps ymm5, ymm3		;set up zi^2
	mov rbx, const_2
	vmovaps ymm6, [rbx]		;Load up 2's
	
	mov r10, rdx			;Get our max_iterations
	mov r12, rcx			;Get our pointer to itertbl to fill

	mov rcx, 8
	mov rax, -1
	mov rdi, rbp
	sub rdi, 96 
	rep stosq			;-1 placeholders in iterations table

	mov rcx, 0			;set up iterations count
cycle:
	vmulps ymm8, ymm3, ymm6		; zi * 2
	vmulps ymm9, ymm8, ymm2		; * zr
	vaddps ymm3, ymm9, ymm1		; + ci, assign to zi

	vsubps ymm8, ymm4, ymm5		; zr2 - zi2
	vaddps ymm2, ymm8, ymm0		; + cr, assign to zr

	vmulps ymm4, ymm2, ymm2		; zr^2 = zr * zr
	vmulps ymm5, ymm3, ymm3		; zi^2 = zi * zi

	vaddps ymm8, ymm4, ymm5		; ymm8 = zr^2 + zi^2
	vmovups [rbp-32], ymm8		; Save result

	mov rbx, const_4
	movss xmm12, [rbx]		;load up our 4

	mov r11, 0			;our recycle flag, if we found unset table entries

	mov rax, [rbp-96]
	cmp rax, -1
	jne tag1
	inc r11
	movss xmm13, [rbp-32]
	cmpss xmm13, xmm12, 1		;less than 4?
	cvtss2si rax, xmm13
	and rax, rax
	jne tag1
	mov [rbp-96], rcx		;save current iteration
tag1:
	mov rax, [rbp-88]
	cmp rax, -1
	jne tag2
	inc r11
	movss xmm13, [rbp-28]
	cmpss xmm13, xmm12, 1		;less than 4?
	cvtss2si rax, xmm13
	and rax, rax
	jne tag2
	mov [rbp-88], rcx		;save current iteration
tag2:
	mov rax, [rbp-80]
	cmp rax, -1
	jne tag3
	inc r11
	movss xmm13, [rbp-24]
	cmpss xmm13, xmm12, 1		;less than 4?
	cvtss2si rax, xmm13
	and rax, rax
	jne tag3
	mov [rbp-80], rcx		;save current iteration
tag3:
	mov rax, [rbp-72]
	cmp rax, -1
	jne tag4
	inc r11
	movss xmm13, [rbp-20]
	cmpss xmm13, xmm12, 1		;less than 4?
	cvtss2si rax, xmm13
	and rax, rax
	jne tag4
	mov [rbp-72], rcx		;save current iteration
tag4:
	mov rax, [rbp-64]
	cmp rax, -1
	jne tag5
	inc r11
	movss xmm13, [rbp-16]
	cmpss xmm13, xmm12, 1		;less than 4?
	cvtss2si rax, xmm13
	and rax, rax
	jne tag5
	mov [rbp-64], rcx		;save current iteration
tag5:
	mov rax, [rbp-56]
	cmp rax, -1
	jne tag6
	inc r11
	movss xmm13, [rbp-12]
	cmpss xmm13, xmm12, 1		;less than 4?
	cvtss2si rax, xmm13
	and rax, rax
	jne tag6
	mov [rbp-56], rcx		;save current iteration
tag6:
	mov rax, [rbp-48]
	cmp rax, -1
	jne tag7
	inc r11
	movss xmm13, [rbp-8]
	cmpss xmm13, xmm12, 1		;less than 4?
	cvtss2si rax, xmm13
	and rax, rax
	jne tag7
	mov [rbp-48], rcx		;save current iteration
tag7:
	mov rax, [rbp-40]
	cmp rax, -1
	jne tagdone
	inc r11
	movss xmm13, [rbp-4]
	cmpss xmm13, xmm12, 1		;less than 4?
	cvtss2si rax, xmm13
	and rax, rax
	jne tagdone
	mov [rbp-40], rcx		;save current iteration
tagdone:
	and r11, r11
	jne recycle
	jmp done

recycle:
	inc rcx
	cmp rcx, r10			;Have we reached max_iterations yet?
	jb cycle

	mov rax, [rbp-96]
	cmp rax, -1
	jne fill1
	mov [rbp-96], r10
fill1:
	mov rax, [rbp-88]
	cmp rax, -1
	jne fill2
	mov [rbp-88], r10
fill2:
	mov rax, [rbp-80]
	cmp rax, -1
	jne fill3
	mov [rbp-80], r10
fill3:
	mov rax, [rbp-72]
	cmp rax, -1
	jne fill4
	mov [rbp-72], r10			; Fill any ones that didn't bail with max_iterations
fill4:
	mov rax, [rbp-64]
	cmp rax, -1
	jne fill5
	mov [rbp-64], r10
fill5:
	mov rax, [rbp-56]
	cmp rax, -1
	jne fill6
	mov [rbp-56], r10
fill6:
	mov rax, [rbp-48]
	cmp rax, -1
	jne fill7
	mov [rbp-48], r10
fill7:
	mov rax, [rbp-40]
	cmp rax, -1
	jne done
	mov [rbp-40], r10

done:
	mov rdi, r12			;get our pointer to table to fill
	mov rsi, rbp
	sub rsi, 96
	mov rcx, 8
	rep movsq			;Move our iteration table into destination

	add rsp, 96
	pop rbp
	pop rbx
	ret

	section .data

	align 32

const_0:
	dd 0.0
	dd 0.0
	dd 0.0
	dd 0.0
	dd 0.0
	dd 0.0
	dd 0.0
	dd 0.0

const_2:
	dd 2.0
	dd 2.0
	dd 2.0
	dd 2.0
	dd 2.0
	dd 2.0
	dd 2.0
	dd 2.0

const_4:
	dd 4.0

