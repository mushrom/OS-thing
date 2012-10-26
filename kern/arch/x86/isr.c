#ifndef _kernel_isr_c
#define _kernel_isr_c

#include <arch/x86/isr.h>

isr_t interrupt_handlers[256];

void isr_handler( registers_t regs ){
	if ( interrupt_handlers[regs.int_no] != 0 ){
		isr_t handler = interrupt_handlers[regs.int_no];
		handler( regs );
	} else {
		printf( "Unhandled interrupt: 0x%x : 0x%x\n", regs.int_no, regs.err_code );
	}
}

void irq_handler( registers_t regs ){
	//kputs( "So yup, we got here...\n" );
	if ( regs.int_no >= 40 ){
		outb( 0xa0, 0x20 );
	}
	outb( 0x20, 0x20 );
	
	if ( interrupt_handlers[regs.int_no] != 0 ){
		isr_t handler = interrupt_handlers[regs.int_no];
		handler( regs );
	}
}

void end_bad_task( void ){
#ifdef NO_DEBUG
	extern task_t *current_task;
	if ( !current_task )
		return;
	int buf = getpid();
	exit_thread();
#else
	PANIC( "Something bad happend. :(\n" );
#endif
}

void register_interrupt_handler( uint8_t n, isr_t handler ){
	interrupt_handlers[n] = handler;
}

void unregister_interrupt_handler( uint8_t n ){
	interrupt_handlers[n] = 0;
}

void dump_registers( registers_t regs ){
	printf( "edi: 0x%x\tesi: 0x%x\tebp: 0x%x\tesp: 0x%x\tebx: 0x%x\n"
		"edx: 0x%x\tecx: 0x%x\teax: 0x%x\teip: 0x%x\t\n"
		" cs: 0x%x\t ss: 0x%x\t ds: 0x%x\tuser esp: 0x%x\n"
		"int: 0x%x\terr: 0x%x\teflags: 0x%s\n",
		regs.edi, regs.esi, regs.ebp, regs.esp, regs.ebx, regs.edx,
		regs.ecx, regs.eax, regs.eip, regs.cs,  regs.ss,  regs.ds,  regs.useresp,
		regs.int_no, regs.err_code, regs.eflags );
}

void gen_protect_fault( registers_t regs ){
	printf( "General protection fault\n" );
	dump_registers( regs );
	end_bad_task( );

	//PANIC( "General protection fault" );
}

void zero_division_fault( registers_t regs ){
	printf( "Zero division fault\n" );
	dump_registers( regs );
	end_bad_task( );
}

#endif
