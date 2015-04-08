.c.o:
	$(CC) $(CFLAGS) -o $*.o -c $<
.s.o:
	$(AS) $(ASMFLAGS) -o $*.o -c $<


kernel.o: console.o panic.o printk.o boot_params.o initcall.o mm/mm.o	\
	irq.o timer.o sched.o exit.o notifier.o semaphore.o syscall.o	\
	signal.o time.o
	$(LD) -r $(LDFLAGS) -o $@  $^

mm/mm.o:
	cd mm;make

clean:
	-rm *.o
	-cd mm;make clean
