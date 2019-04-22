
all:
	gcc -c DiskUtil.c
	gcc -o wrdsk wrdsk.c DiskUtil.o
	gcc -o rddsk rddsk.c DiskUtil.o

clean:
	rm *.o wrdsk rddsk *~
