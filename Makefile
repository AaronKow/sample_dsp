# Makefile

project:
	rm -rf build
	mkdir -p build
	cd build && cmake .. && make

clean:
	rm -rf build
	rm -rf *.uf2