
%if __BITS__  == 64

section .text


;; protoypes not meants to be used now

global blend_int8_int32_avx2_asm
blend_int8_int32_avx2_asm:

	mov [rsp + 32], rbx ;windows make rbx useable
	mov [rsp + 24], r12

	mov rax, [rsp + 40] ; element_count in aex
	shr rax, 3
	mov [rsp + 16], rax ; stack count /8 for avx
	mov eax, [rsp + 40]
	and eax, 7
	mov [rsp + 8], eax ; stack count %8 for after avx

	sub rsp, 32 ; array of 8 int32

	xor eax, eax ; total_weights

l1:
	mov r10d, [r8]
	test r10d, r10d
	;je l3 ; test if weights is null

	add eax, r10d ; update total weights
	VBROADCASTSS ymm1, [r8] ; fill register with weight

	xor r11d, r11d ; tmp for 8 to 32
	mov r10, [rdx] ; frame data ptr
	mov r12, [rsp + 80] ; tmp_frame
	mov rbx, [rsp + 48] ; count /8

l2:

	;convert 8 int32
	;mov r11b, [r10]
	;mov [rsp], r11d
	;mov r11b, [r10 +1]
	;mov [rsp + 4], r11d
	;mov r11b, [r10 +2]
	;mov [rsp + 8], r11d
	;mov r11b, [r10 +3]
	;mov [rsp + 12], r11d
	;mov r11b, [r10 +4]
	;mov [rsp + 16], r11d
	;mov r11b, [r10 +5]
	;mov [rsp + 20], r11d
	;mov r11b, [r10 +6]
	;mov [rsp + 24], r11d
	;mov r11b, [r10 +7]
	;mov [rsp + 28], r11d
	add r10, 8

	vmovupd ymm0, [rsp] ; in_frame data
	vmovupd ymm3, [r12] ; tmp_frame data
	VPMULLD ymm2, ymm0, ymm1
	vpaddd ymm4, ymm2, ymm3
	vmovupd [r12], ymm4

	add r12, 32
	sub rbx, 1
	test rbx, rbx
	jne l2

l3:
	add rdx, 8 ; in_frames
	add r8, 4 ; weights
	sub r9, 1 ; nb
	test r9, r9
	jne l1

	;
	mov r12, [rsp + 80] ; tmp_frame
	mov rbx, [rsp + 72] ; count
	;push eax
	;VBROADCASTSS ymm1, [rsp]
	;pop eax;

l4:

	;mov r10d, [r12]
	;add r12, 4
	;div r10d
	;mov [rcx], r10b
	;sub rbx, 1
	;test rbx, rbx
	;jne l4





	add rsp, 32
	mov rbx, [rsp + 32]
	mov r12, [rsp + 24]
	
	;sub rsp, 32
	;mov rax, [rdx]
	;mov rbx, 0
	;mov bl, [rax]
	;mov [rsp], ebx
	;mov [rsp +4], ebx
	;vmovdqu YMM1, [rsp]
	;add rsp, 32
	ret


	global blend_float32_float32_avx_asm
blend_float32_float32_avx_asm:

	;windows
	mov [rsp + 32], rbx
	mov [rsp + 24], r12
	mov [rsp + 16], r13
	mov [rsp + 8], r14

	;loop calculate total weight
	mov r10, r9 ; nb
	mov r11, r8 ; weights
	xorps xmm1, xmm1
b1:
	addss xmm1, [r8]
	add r11, 8
	sub r10, 1
	test r10, r10
	jne b1
	
	mov eax,__?float32?__(1.0)
	movd xmm2, eax ; 1.0

	vdivss xmm1, xmm2, xmm1

	vbroadcastss ymm0, xmm1
	vbroadcastss ymm10, xmm1
	vbroadcastss ymm11, xmm1
	vbroadcastss ymm12, xmm1

	;loop calculate tmp_frame
	;fma mult add 
	;avx div
	mov r12, [rsp + 48] ; tmp_frame

	mov r14, [rsp + 40] ; element_count
	shr r14, 3 ; /8
	xor r13, r13
b3:
	

	xor r10, r10
	vpxor ymm3, ymm3 ; null
	vpxor ymm5, ymm5
	vpxor ymm7, ymm7
	vpxor ymm9, ymm9
b2:
	vbroadcastss ymm1, [r8 + r10*4] ; broadcast weight
	vbroadcastss ymm13, [r8 + r10*4]
	vbroadcastss ymm14, [r8 + r10*4]
	vbroadcastss ymm15, [r8 + r10*4]
	mov r11, [rdx + 8*r10] ; take a input frame
	mov rax, r13
	shl rax, 5 ;
	vmovups ymm2, [r11 + rax] ; input_frame data
	vmulps ymm2, ymm2, ymm1 ; tmp_frame_data += weight * input_frame_data
	vaddps ymm3, ymm3, ymm2
	vmovups ymm4, [r11 + rax + 32]
	vmulps ymm4, ymm4, ymm13
	vaddps ymm5, ymm5, ymm4
	vmovups ymm6, [r11 + rax + 64]
	vmulps ymm6, ymm6, ymm14
	vaddps ymm7, ymm7, ymm6
	vmovups ymm8, [r11 + rax + 96]
	vmulps ymm8, ymm8, ymm15
	vaddps ymm9, ymm9, ymm8
	add r10, 1
	cmp r10, r9
	jne b2
	

	vmulps ymm3, ymm3, ymm0 ; tmp_frame_data /= total_weights
	vmovups [r12], ymm3 ; tmp_frame_data -> tmp_frame
	vmulps ymm5, ymm5, ymm10
	vmovups [r12 + 32], ymm5
	vmulps ymm7, ymm7, ymm11
	vmovups [r12 + 64], ymm7
	vmulps ymm9, ymm9, ymm12
	vmovups [r12 + 96], ymm9
	add r12, 128 ; move tmp_frame*
	add r13, 4
	cmp r13, r14
	jb b3

	mov rbx, [rsp + 32]
	mov r12, [rsp + 24]
	mov r13, [rsp + 16]
	mov r14, [rsp + 8]

	ret


	global blend_float32_float32_fma3avx_asm
blend_float32_float32_fma3avx_asm:

	;windows
	mov [rsp + 32], rbx
	mov [rsp + 24], r12
	mov [rsp + 16], r13
	mov [rsp + 8], r14

	;loop calculate total weight
	mov r10, r9 ; nb
	mov r11, r8 ; weights
	xorps xmm1, xmm1
c1:
	addss xmm1, [r8]
	add r11, 8
	sub r10, 1
	test r10, r10
	jne c1
	
	mov eax,__?float32?__(1.0)
	movd xmm2, eax ; 1.0

	vdivss xmm1, xmm2, xmm1

	vbroadcastss ymm0, xmm1
	vbroadcastss ymm10, xmm1
	vbroadcastss ymm11, xmm1
	vbroadcastss ymm12, xmm1

	;loop calculate tmp_frame
	;fma mult add 
	;avx div
	mov r12, [rsp + 48] ; tmp_frame

	mov r14, [rsp + 40] ; element_count
	shr r14, 3 ; /8
	xor r13, r13
c3:
	

	xor r10, r10
	vpxor ymm3, ymm3 ; null
	vpxor ymm5, ymm5
	vpxor ymm7, ymm7
	vpxor ymm9, ymm9
c2:
	vbroadcastss ymm1, [r8 + r10*4] ; broadcast weight
	vbroadcastss ymm13, [r8 + r10*4]
	vbroadcastss ymm14, [r8 + r10*4]
	vbroadcastss ymm15, [r8 + r10*4]
	mov r11, [rdx + 8*r10] ; take a input frame
	mov rax, r13
	shl rax, 5 ;
	vmovups ymm2, [r11 + rax] ; input_frame data
	vfmadd231ps ymm3, ymm1, ymm2 ; tmp_frame_data += weight * input_frame_data
	vmovups ymm4, [r11 + rax + 32]
	vfmadd231ps ymm5, ymm13, ymm4
	vmovups ymm6, [r11 + rax + 64]
	vfmadd231ps ymm7, ymm14, ymm6
	vmovups ymm8, [r11 + rax + 96]
	vfmadd231ps ymm9, ymm15, ymm8
	add r10, 1
	cmp r10, r9
	jne c2
	

	vmulps ymm3, ymm3, ymm0 ; tmp_frame_data /= total_weights
	vmovups [r12], ymm3 ; tmp_frame_data -> tmp_frame
	vmulps ymm5, ymm5, ymm10
	vmovups [r12 + 32], ymm5
	vmulps ymm7, ymm7, ymm11
	vmovups [r12 + 64], ymm7
	vmulps ymm9, ymm9, ymm12
	vmovups [r12 + 96], ymm9
	add r12, 128 ; move tmp_frame*
	add r13, 4
	cmp r13, r14
	jb c3

	mov rbx, [rsp + 32]
	mov r12, [rsp + 24]
	mov r13, [rsp + 16]
	mov r14, [rsp + 8]

	ret

%endif