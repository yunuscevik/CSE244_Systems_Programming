all: listdir

listdir: main.o 
	gcc main.o  -o listdir

main.o: main.c
	 gcc -c -pedantic-errors -Wall main.c


clean:
	rm *.o listdir *.log
