CC=gcc -std=gnu11
CFLAGS=-O3 -g
TARGET=x86_64
DEPS=mir.h mir-varr.h mir-dlist.h mir-htab.h mir-hash.h
OBJS=mir.o mir-interp.o mir-gen.o
all: $(OBJS)

mir.o: mir.c $(DEPS)
	$(CC) -c $(CFLAGS) -DMIR_IO -DMIR_SCAN -o $@ $<

mir-interp.o: mir-interp.c $(DEPS) mir-interp.h mir-$(TARGET).c
	$(CC) -c -Os -g -o $@ $<

mir-gen.o: mir-gen.c $(DEPS) mir-bitmap.h mir-gen-$(TARGET).c
	$(CC) -c $(CFLAGS) -D$(TARGET) -o $@ $<

test: util-test mir-test io-test scan-test interp-test gen-test readme-example-test mir2c-test c2mir-test
	@echo ==============================Test is done
      
bench: interp-bench gen-bench io-bench mir2c-bench c2mir-bench
	@echo ==============================Bench is done

mir-test:
	$(CC) -g mir.c mir-tests/simplify.c && ./a.out

scan-test:
	$(CC) -g -DMIR_SCAN mir.c mir-tests/scan-test.c && ./a.out

io-test:
	$(CC) -g -DMIR_SCAN -DMIR_IO mir.c mir-tests/io.c && ./a.out

io-bench:
	$(CC) $(CFLAGS) -DNDEBUG -DMIR_SCAN -DMIR_IO mir.c mir-tests/io-bench.c && ./a.out

interp-loop:
	$(CC) -g -D$(TARGET) -DMIR_INTERP_DEBUG=1 mir.c mir-interp.c mir-tests/loop-interp.c && ./a.out
interp-loop-c:
	$(CC) -g -D$(TARGET) -DMIR_INTERP_DEBUG=1 -DMIR_C_INTERFACE=1 mir.c mir-interp.c mir-tests/loop-interp.c && ./a.out
interp-sieve:
	$(CC) -g -D$(TARGET) -DMIR_SCAN -DMIR_INTERP_DEBUG=1 mir.c mir-interp.c mir-tests/sieve-interp.c && ./a.out
interp-sieve-c:
	$(CC) -g -D$(TARGET) -DMIR_SCAN -DMIR_INTERP_DEBUG=1 -DMIR_C_INTERFACE=1 mir.c mir-interp.c mir-tests/sieve-interp.c && ./a.out
interp-hi:
	$(CC) -g -D$(TARGET) -DMIR_SCAN -DMIR_INTERP_DEBUG=1 mir.c mir-interp.c mir-tests/hi-interp.c && ./a.out
interp-args:
	$(CC) -g -D$(TARGET) -DMIR_SCAN mir.c mir-interp.c mir-tests/args-interp.c && ./a.out
interp-args-c:
	$(CC) -g -D$(TARGET) -DMIR_SCAN -DMIR_C_INTERFACE=1 mir.c mir-interp.c mir-tests/args-interp.c && ./a.out
interp-test8:
	$(CC) -g -D$(TARGET) -DMIR_SCAN mir.c mir-interp.c mir-gen.c mir-tests/run-test.c && ./a.out -i mir-tests/test8.mir
interp-test9:
	$(CC) -g -D$(TARGET) -DMIR_SCAN mir.c mir-interp.c mir-gen.c mir-tests/run-test.c && ./a.out -i mir-tests/test9.mir

interp-test10:
	$(CC) -g -D$(TARGET) -DMIR_SCAN mir.c mir-interp.c mir-gen.c mir-tests/run-test.c && ./a.out -i mir-tests/test10.mir

interp-test11:
	$(CC) -g -D$(TARGET) -DMIR_SCAN mir.c mir-interp.c mir-gen.c mir-tests/run-test.c && ./a.out -i mir-tests/test11.mir

interp-test: interp-loop interp-loop-c interp-sieve interp-sieve-c interp-hi interp-args interp-args-c interp-test8\
             interp-test9 interp-test10 interp-test11

interp-bench:
	$(CC) $(CFLAGS) -DNDEBUG -D$(TARGET) mir.c mir-interp.c mir-tests/loop-interp.c && ./a.out && size ./a.out
	$(CC) $(CFLAGS) -DNDEBUG -D$(TARGET) -DMIR_C_INTERFACE=1 mir.c mir-interp.c mir-tests/loop-interp.c && ./a.out && size ./a.out
	$(CC) $(CFLAGS) -DNDEBUG -D$(TARGET) -DMIR_SCAN mir.c mir-interp.c mir-tests/sieve-interp.c && ./a.out && size ./a.out

gen-test1:
	$(CC) -g -D$(TARGET) -DMIR_SCAN -DMIR_GEN_DEBUG=1 mir.c mir-gen.c mir-tests/test-gen.c && ./a.out mir-tests/test1.mir
gen-test2:
	$(CC) -g -D$(TARGET) -DMIR_SCAN -DMIR_GEN_DEBUG=1 mir.c mir-gen.c mir-tests/test-gen.c && ./a.out mir-tests/test2.mir
gen-test3:
	$(CC) -g -D$(TARGET) -DMIR_SCAN -DMIR_GEN_DEBUG=1 mir.c mir-gen.c mir-tests/test-gen.c && ./a.out mir-tests/test3.mir
gen-test4:
	$(CC) -g -D$(TARGET) -DMIR_SCAN -DMIR_GEN_DEBUG=1 mir.c mir-gen.c mir-tests/test-gen.c && ./a.out mir-tests/test4.mir
gen-test5:
	$(CC) -g -D$(TARGET) -DMIR_SCAN -DMIR_GEN_DEBUG=1 mir.c mir-gen.c mir-tests/test-gen.c && ./a.out mir-tests/test5.mir
gen-test6:
	$(CC) -g -D$(TARGET) -DMIR_SCAN -DMIR_GEN_DEBUG=1 mir.c mir-gen.c mir-tests/test-gen.c && ./a.out mir-tests/test6.mir
gen-test7:
	$(CC) -g -D$(TARGET) -DMIR_SCAN -DMIR_GEN_DEBUG=1 mir.c mir-gen.c mir-tests/test-gen.c && ./a.out mir-tests/test7.mir

gen-test8:
	$(CC) -g -D$(TARGET) -DMIR_SCAN -DMIR_GEN_DEBUG=1 mir.c mir-interp.c mir-gen.c mir-tests/run-test.c && ./a.out -g mir-tests/test8.mir

gen-test9:
	$(CC) -g -D$(TARGET) -DMIR_SCAN -DMIR_GEN_DEBUG=1 mir.c mir-interp.c mir-gen.c mir-tests/run-test.c && ./a.out -g mir-tests/test9.mir

gen-test10:
	$(CC) -g -D$(TARGET) -DMIR_SCAN mir.c mir-interp.c mir-gen.c mir-tests/run-test.c && ./a.out -g mir-tests/test10.mir

gen-test11:
	$(CC) -g -D$(TARGET) -DMIR_SCAN mir.c mir-interp.c mir-gen.c mir-tests/run-test.c -DMIR_GEN_DEBUG=1 && ./a.out -g mir-tests/test11.mir

gen-test: gen-test1 gen-test2 gen-test3 gen-test4 gen-test5 gen-test6 gen-test7 gen-test8 gen-test9 gen-test10 gen-test11
	$(CC) -g -D$(TARGET) -DTEST_GEN_LOOP -DMIR_GEN_DEBUG=1 mir.c mir-gen.c mir-tests/loop-sieve-gen.c && ./a.out
	$(CC) -g -D$(TARGET) -DMIR_SCAN -DTEST_GEN_SIEVE -DMIR_GEN_DEBUG=1 mir.c mir-gen.c mir-tests/loop-sieve-gen.c && ./a.out

gen-bench:
	$(CC) $(CFLAGS) -DNDEBUG -D$(TARGET) -DTEST_GEN_LOOP mir.c mir-gen.c mir-tests/loop-sieve-gen.c && ./a.out && size ./a.out
	$(CC) $(CFLAGS) -DNDEBUG -D$(TARGET) -DMIR_SCAN -DTEST_GEN_SIEVE mir.c mir-gen.c mir-tests/loop-sieve-gen.c && ./a.out && size ./a.out

readme-example-test:
	$(CC) -g -D$(TARGET) -DMIR_SCAN mir.c mir-interp.c mir-gen.c mir-tests/readme-example.c && ./a.out

c2mir-test:
	$(CC) -g -D$(TARGET) -DTEST_C2MIR -I. mir.c mir-interp.c mir-gen.c c2mir/c2mir.c && ./a.out -S -v -ei

c2mir-bench:
	$(CC) $(CFLAGS) -DNDEBUG -D$(TARGET) -DTEST_C2MIR -I. mir-interp.c mir-gen.c c2mir/c2mir.c mir.c && ./a.out -v -eg && size ./a.out

mir2c-test:
	$(CC) -g -DTEST_MIR2C -DMIR_SCAN -I. mir.c mir2c/mir2c.c && ./a.out

mir2c-bench:
	$(CC) $(CFLAGS) -DNDEBUG -DTEST_MIR2C -DMIR_SCAN -I. mir2c/mir2c.c mir.c && ./a.out -v && size ./a.out

util-test: varr-test dlist-test bitmap-test htab-test

varr-test:
	$(CC) -g -I. util-tests/mir-varr-test.c && ./a.out

dlist-test:
	$(CC) -g -I. util-tests/mir-dlist-test.c && ./a.out

bitmap-test:
	$(CC) -g -I. util-tests/mir-bitmap-test.c && ./a.out

htab-test:
	$(CC) -g -I. util-tests/mir-htab-test.c && ./a.out

clean:
	rm -f $(OBJS) ./a.out

realclean: clean

sloc:
	@echo -n 'C2MIR: ' && wc -l c2mir/c2mir.c | awk '{last=$$1} END {print last}'
	@echo -n 'ADT: ' && wc -l $(DEPS) mir-bitmap.h mir-mp.h | awk '{last=$$1} END {print last}'
	@echo -n 'MIR API: ' && wc -l mir.[ch] | awk '{last=$$1} END {print last}'
	@echo -n 'MIR Interpreter: ' && wc -l mir-interp.[ch] | awk '{last=$$1} END {print last}'
	@echo -n 'MIR Generator: ' && wc -l mir-gen.[ch] | awk '{last=$$1} END {print last}'
	@echo -n 'Machine dependent code: ' && wc -l mir-x86_64.c mir-gen-x86_64.c | awk '{last=$$1} END {print last}'
	@echo -n 'Overall: ' && wc -l c2mir/c2mir.c $(DEPS) mir-bitmap.h mir-mp.h mir.[ch] mir-interp.[ch] mir-gen.[ch] mir-x86_64.c mir-gen-x86_64.c | awk '{last=$$1} END {print last}'
