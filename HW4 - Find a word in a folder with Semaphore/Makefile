all: grepTh

grepTh: main.o 
	gcc main.o  -o grepTh -lpthread -lrt -lm

main.o: main.c
	 gcc -c  main.c

clean:
	rm *.o grepTh *.log
