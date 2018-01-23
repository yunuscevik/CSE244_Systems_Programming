all: grepSh

grepSh: main.o 
	gcc main.o  -o grepSh -lpthread -lrt -lm

main.o: main.c
	 gcc -c  main.c

clean:
	rm *.o grepSh log.txt
