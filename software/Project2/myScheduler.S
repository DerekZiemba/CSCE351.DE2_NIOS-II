	.section .exceptions.exit.user, "xa"

	// Check timerflag
	call	check_timer_flag
	beq		r2, r0, DONE
	call	reset_timer_flag

	/* Disable Interrupts */
	//wrctl 	status, zero

	mov		r4, sp  //Pass stack pointer to mythread_scheduler
	//mov		r5, fp  //Pass frame pointer to mythread_scheduler
	call 	mythread_scheduler
	stw 	fp, -4(sp);
	mov 	sp, r2;
	ldw		fp, -4(sp);
	//beq		r2, r0, DONE
	//mov		sp, r2 //Get new threads stack pointer from result of mythread_scheduler
	//mov		fp, r3 //Get new threads frame pointer from result of mythread_scheduler


	/* Enable interrupts */
	//movi et, 1
	//wrctl status, et

	DONE:


