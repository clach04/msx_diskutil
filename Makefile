
ifeq ($(OS),Windows_NT)
    RM = del /Q /F
else
    RM = rm -f
endif

all:
	gcc -c DiskUtil.c
	gcc -o wrdsk wrdsk.c DiskUtil.o
	gcc -o rddsk rddsk.c DiskUtil.o

clean:
	$(RM) *.o wrdsk rddsk *~
