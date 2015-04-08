.c.o:
	$(CC) $(CFLAGS) -o $*.o -c $<
.S.o:
	$(AS) $(ASMFLAGS) -o $*.o -c $<


arch.o: gdt.o entry.o idt.o idt_generic.o i8259.o timer.o sched.o	\
	signal.o
	$(LD) -r $(LDFLAGS) -o $@  $^

clean:
	-rm *.o
