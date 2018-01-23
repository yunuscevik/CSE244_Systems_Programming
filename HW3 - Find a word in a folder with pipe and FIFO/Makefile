all: withPipeandFIFO

withPipeandFIFO: main.o 
	gcc main.o  -o withPipeandFIFO

main.o: main.c
	 gcc -c -pedantic-errors -ansi -Wall main.c


clean:
	rm *.o withPipeandFIFO *.log
