
default: conv

conv: mir mir2lua
	$(CC) ./build/mir.o mir2lua/mir2lua.c -I. -o ./build/mir2lua -g3

mir: .dummy
	@mkdir -p ./build
	$(MAKE) -f GNUmakefile BUILD_DIR=./build

clean: .dummy
	rm -r ./build

.dummy:
