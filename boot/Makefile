boot.o:	boot.S
	$(CC) $(ASMFLAGS) -o $@ -c $^

clean:	
	-rm -rf *.o
	-rm link.lds
	-rm $(shell basename $(TARGET))


