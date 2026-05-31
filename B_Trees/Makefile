all:
	make build
	make run
	make clean

build:
	clang -I./include/ \
		./src/*.c \
		main.c \
		-o a.out

run:
	./a.out

clean:
	rm ./a.out