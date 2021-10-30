
default: conv conv2

conv: mir mir2lua
	$(CC) ./build/mir.o mir2lua/mir2lua.c -I. -o ./build/mir2lua -g3

conv2: mir mir2lua
	$(CC) ./build/mir.o mir2lua/mir2lua2.c -I. -o ./build/mir2c

mir: .dummy
	@mkdir -p ./build
	$(MAKE) -f GNUmakefile BUILD_DIR=./build

clean: .dummy
	rm -r ./build

.dummy:
