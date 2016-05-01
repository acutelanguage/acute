all: lib

lib:
	cd src && make lib

clean:
	cd src && make clean
	cd tests && make clean

test: lib
	cd tests && make test

