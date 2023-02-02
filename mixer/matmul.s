	.file	"matmul.c"
	.option nopic
	.text
	.align	1
	.globl	main
	.type	main, @function
main:
	addi	sp,sp,-480
	sd	s0,472(sp)
	addi	s0,sp,480
	mv	a5,a0
	sd	a1,-480(s0)
	sw	a5,-468(s0)
	sw	zero,-20(s0)
	j	.L2
.L7:
	sw	zero,-24(s0)
	j	.L3
.L6:
	lw	a3,-24(s0)
	lw	a4,-20(s0)
	mv	a5,a4
	slli	a5,a5,1
	add	a5,a5,a4
	slli	a5,a5,1
	add	a5,a5,a3
	slli	a5,a5,2
	addi	a5,a5,-16
	add	a5,a5,s0
	sw	zero,-448(a5)
	sw	zero,-28(s0)
	j	.L4
.L5:
	lw	a3,-24(s0)
	lw	a4,-20(s0)
	mv	a5,a4
	slli	a5,a5,1
	add	a5,a5,a4
	slli	a5,a5,1
	add	a5,a5,a3
	slli	a5,a5,2
	addi	a5,a5,-16
	add	a5,a5,s0
	lw	a3,-448(a5)
	lw	a2,-28(s0)
	lw	a4,-20(s0)
	mv	a5,a4
	slli	a5,a5,1
	add	a5,a5,a4
	slli	a5,a5,1
	add	a5,a5,a2
	slli	a5,a5,2
	addi	a5,a5,-16
	add	a5,a5,s0
	lw	a2,-160(a5)
	lw	a1,-24(s0)
	lw	a4,-28(s0)
	mv	a5,a4
	slli	a5,a5,1
	add	a5,a5,a4
	slli	a5,a5,1
	add	a5,a5,a1
	slli	a5,a5,2
	addi	a5,a5,-16
	add	a5,a5,s0
	lw	a5,-304(a5)
	mulw	a5,a2,a5
	sext.w	a5,a5
	addw	a5,a3,a5
	sext.w	a3,a5
	lw	a2,-24(s0)
	lw	a4,-20(s0)
	mv	a5,a4
	slli	a5,a5,1
	add	a5,a5,a4
	slli	a5,a5,1
	add	a5,a5,a2
	slli	a5,a5,2
	addi	a5,a5,-16
	add	a5,a5,s0
	sw	a3,-448(a5)
	lw	a5,-28(s0)
	addiw	a5,a5,1
	sw	a5,-28(s0)
.L4:
	lw	a5,-28(s0)
	sext.w	a4,a5
	li	a5,5
	ble	a4,a5,.L5
	lw	a5,-24(s0)
	addiw	a5,a5,1
	sw	a5,-24(s0)
.L3:
	lw	a5,-24(s0)
	sext.w	a4,a5
	li	a5,5
	ble	a4,a5,.L6
	lw	a5,-20(s0)
	addiw	a5,a5,1
	sw	a5,-20(s0)
.L2:
	lw	a5,-20(s0)
	sext.w	a4,a5
	li	a5,5
	ble	a4,a5,.L7
	li	a5,0
	mv	a0,a5
	ld	s0,472(sp)
	addi	sp,sp,480
	jr	ra
	.size	main, .-main
	.ident	"GCC: (g) 11.1.0"
	.section	.note.GNU-stack,"",@progbits
