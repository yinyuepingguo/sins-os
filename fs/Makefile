.c.o:
	$(CC) $(CFLAGS) -o $*.o -c $<
.s.o:
	$(CC) $(ASMFLAGS) -o $*.o -c $<


fs.o: char_dev.o block_dev.o file_dev.o open.o namei.o inode.o	\
	 super.o dentry.o exec.o minix/minix.o
	$(LD) -r $(LDFLAGS) -o $@  $^

minix/minix.o:
	-cd minix; make


clean:
	-rm *.o
	-cd minix; make clean
