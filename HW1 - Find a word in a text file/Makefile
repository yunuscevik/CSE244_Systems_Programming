all: list

list: main.o 
	gcc main.o  -o list

main.o: main.c
	 gcc -c -pedantic-errors -Wall  main.c


clean:
	rm *.o list
