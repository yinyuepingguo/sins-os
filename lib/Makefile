.c.o:
	$(CC) $(CFLAGS) -o $*.o -c $<
.s.o:
	$(AS) $(ASMFLAGS) -o $*.o -c $<

lib.o: string.o vsprintf.o ctype.o div64.o syscalls.o
	$(LD) -r $(LDFLAGS) -o $@  $^
clean:
	-rm *.o
