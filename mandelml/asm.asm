	global iterate4 

	section .text

iterate4:
	push rbx
	push rbp
	mov rbp, rsp

	mov r8, rdi			;Save our pointer to cr's
	mov r9, rsi			;Save our pointer to ci's

	vmovupd ymm0, [r8]
	vmovupd ymm1, [r9]		;Load up C's
	mov rbx, const_0
	vmovapd ymm2, [rbx]		;zr = 0
	vmovapd ymm3, ymm2		;zi = 0
	vmovapd ymm4, ymm2		;Set up zr^2
	vmovapd ymm5, ymm3		;set up zi^2
	mov rbx, const_2
	vmovapd ymm6, [rbx]		;Load up 2's
	
	mov r10, rdx			;Get our max_iterations

	mov rcx, 4
	mov rax, -1
	mov rdi, itertbl0
	rep stosq			;-1 placeholders in iterations table

	mov rcx, 0			;set up iterations count
cycle:
	vmulpd ymm8, ymm3, ymm6		; zi * 2
	vmulpd ymm9, ymm8, ymm2		; * zr
	vaddpd ymm3, ymm9, ymm1		; + ci, assign to zi

	vsubpd ymm8, ymm4, ymm5		; zr2 - zi2
	vaddpd ymm2, ymm8, ymm0		; + cr, assign to zr

	vmulpd ymm4, ymm2, ymm2		; zr^2 = zr * zr
	vmulpd ymm5, ymm3, ymm3		; zi^2 = zi * zi

	vaddpd ymm8, ymm4, ymm5		; ymm8 = zr^2 + zi^2
	mov rbx, sav0
	vmovapd [rbx], ymm8		; Save result

	mov rbx, const_4
	movsd xmm12, [rbx]		;load up our 4

	mov r11, 0			;our recycle flag, if we found unset table entries

	mov rbx, itertbl0
	mov rax, [rbx]
	cmp rax, -1
	jne tag1
	inc r11
	mov rbx, sav0
	movsd xmm13, [rbx]
	cmpsd xmm13, xmm12, 1		;less than 4?
	cvtsd2si rax, xmm13
	and rax, rax
	jne tag1
	mov rbx, itertbl0
	mov [rbx], rcx			;save current iteration
tag1:
	mov rbx, itertbl1
	mov rax, [rbx]
	cmp rax, -1
	jne tag2
	inc r11
	mov rbx, sav1
	movsd xmm13, [rbx]
	cmpsd xmm13, xmm12, 1		;less than 4?
	cvtsd2si rax, xmm13
	and rax, rax
	jne tag2
	mov rbx, itertbl1
	mov [rbx], rcx			;save current iteration
tag2:
	mov rbx, itertbl2
	mov rax, [rbx]
	cmp rax, -1
	jne tag3
	inc r11
	mov rbx, sav2
	movsd xmm13, [rbx]
	cmpsd xmm13, xmm12, 1		;less than 4?
	cvtsd2si rax, xmm13
	and rax, rax
	jne tag3
	mov rbx, itertbl2
	mov [rbx], rcx			;save current iteration
tag3:
	mov rbx, itertbl3
	mov rax, [rbx]
	cmp rax, -1
	jne tagdone
	inc r11
	mov rbx, sav3
	movsd xmm13, [rbx]
	cmpsd xmm13, xmm12, 1		;less than 4?
	cvtsd2si rax, xmm13
	and rax, rax
	jne tagdone
	mov rbx, itertbl3
	mov [rbx], rcx			;save current iteration
tagdone:
	and r11, r11
	jne recycle
	jmp done

recycle:
	inc rcx
	cmp rcx, r10			;Have we reached max_iterations yet?
	jb cycle

	mov rbx, itertbl0
	mov rax, [rbx]
	cmp rax, -1
	jne fill1
	mov [rbx], r10
fill1:
	mov rbx, itertbl1
	mov rax, [rbx]
	cmp rax, -1
	jne fill2
	mov [rbx], r10
fill2:
	mov rbx, itertbl2
	mov rax, [rbx]
	cmp rax, -1
	jne fill3
	mov [rbx], r10
fill3:
	mov rbx, itertbl3
	mov rax, [rbx]
	cmp rax, -1
	jne done
	mov [rbx], r10			; Fill any ones that didn't bail with max_iterations

done:
	mov rax, itertbl0		;return pointer to array
	pop rbp
	pop rbx
	ret

	section .data

	align 32

sav0:
	dq 0
sav1:
	dq 0
sav2:
	dq 0
sav3:
	dq 0

const_0:
	dq 0.0
	dq 0.0
	dq 0.0
	dq 0.0

const_2:
	dq 2.0
	dq 2.0
	dq 2.0
	dq 2.0

const_4:
	dq 4.0

itertbl0:
	dq 0
itertbl1:
	dq 0
itertbl2:
	dq 0
itertbl3:
	dq 0



