.c.o:
	$(CC) $(CFLAGS) -o $*.o -c $<
.s.o:
	$(CC) $(ASMFLAGS) -o $*.o -c $<


drivers.o: console.o tty.o hd.o keyboard.o proc.o
	$(LD) -r $(LDFLAGS) -o $@  $^

clean:
	-rm *.o
