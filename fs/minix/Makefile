.c.o:
	$(CC) $(CFLAGS) -o $*.o -c $<
.s.o:
	$(CC) $(ASMFLAGS) -o $*.o -c $<


minix.o: main.o block.o super.o inode.o read.o
	$(LD) -r $(LDFLAGS) -o $@  $^

clean:
	-rm *.o
