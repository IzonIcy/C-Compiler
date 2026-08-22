CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -g
CPPFLAGS ?= -Iinclude
ANALYZER ?= clang
FUZZ_CC ?= clang

TARGET := build/bin/C-Compiler
SOURCES := $(wildcard src/ccompiler/*.c)
OBJECTS := $(patsubst src/%.c,build/obj/%.o,$(SOURCES))
FUZZ_TARGET := build/bin/fuzz-standalone
FUZZ_LIBFUZZER_TARGET := build/bin/fuzz-pipeline
FUZZ_PARSE_TARGET := build/bin/fuzz-parse
FUZZ_PARSE_LIBFUZZER_TARGET := build/bin/fuzz-parse-libfuzzer
# main.c provides the driver's main(); the fuzz builds bring their own.
FUZZ_SOURCES := $(filter-out src/ccompiler/main.c,$(SOURCES))

.PHONY: all clean test analyze verify sanitizers fuzz fuzz-libfuzzer fuzz-parse fuzz-parse-libfuzzer

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

build/obj/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Portable fuzz harness (ASan/UBSan, works with any compiler).
$(FUZZ_TARGET): $(FUZZ_SOURCES) fuzz/pipeline_fuzz.c fuzz/standalone_main.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) -std=c11 -g -fsanitize=address,undefined -fno-omit-frame-pointer \
		$(FUZZ_SOURCES) fuzz/pipeline_fuzz.c fuzz/standalone_main.c -o $@

# Real libFuzzer build; requires clang with the fuzzer runtime (Linux CI has it).
$(FUZZ_LIBFUZZER_TARGET): $(FUZZ_SOURCES) fuzz/pipeline_fuzz.c
	@mkdir -p $(dir $@)
	$(FUZZ_CC) $(CPPFLAGS) -std=c11 -g -fsanitize=fuzzer,address,undefined -fno-omit-frame-pointer \
		$(FUZZ_SOURCES) fuzz/pipeline_fuzz.c -o $@

# Parser-only harnesses: same rules as the pipeline targets, but the input is
# always parsed (diagnostics included) to exercise error recovery.
$(FUZZ_PARSE_TARGET): $(FUZZ_SOURCES) fuzz/parser_fuzz.c fuzz/standalone_main.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) -std=c11 -g -fsanitize=address,undefined -fno-omit-frame-pointer \
		$(FUZZ_SOURCES) fuzz/parser_fuzz.c fuzz/standalone_main.c -o $@

$(FUZZ_PARSE_LIBFUZZER_TARGET): $(FUZZ_SOURCES) fuzz/parser_fuzz.c
	@mkdir -p $(dir $@)
	$(FUZZ_CC) $(CPPFLAGS) -std=c11 -g -fsanitize=fuzzer,address,undefined -fno-omit-frame-pointer \
		$(FUZZ_SOURCES) fuzz/parser_fuzz.c -o $@

test: $(TARGET)
	sh tests/data_driven_smoke.sh
	sh tests/lexer_smoke.sh
	sh tests/parser_smoke.sh
	sh tests/preprocessor_smoke.sh
	sh tests/sema_smoke.sh
	sh tests/codegen_smoke.sh
	sh tests/golden_smoke.sh
	sh tests/clang_diff.sh

analyze:
	$(ANALYZER) --analyze $(CPPFLAGS) $(CFLAGS) src/ccompiler/*.c

fuzz: $(FUZZ_TARGET)

fuzz-libfuzzer: $(FUZZ_LIBFUZZER_TARGET)

fuzz-parse: $(FUZZ_PARSE_TARGET)

fuzz-parse-libfuzzer: $(FUZZ_PARSE_LIBFUZZER_TARGET)

verify: test analyze

sanitizers:
	$(MAKE) clean
	$(MAKE) test CFLAGS="$(CFLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer"

clean:
	rm -rf build
