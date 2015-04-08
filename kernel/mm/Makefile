.c.o:
	$(CC) $(CFLAGS) -o $*.o -c $<
.s.o:
	$(AS) $(ASMFLAGS) -o $*.o -c $<

mm.o: init.o page_alloc.o highmem.o kmalloc.o page_table.o
	$(LD) -r $(LDFLAGS) -o $@  $^
clean:
	-rm *.o
