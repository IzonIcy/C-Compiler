/*
 * Standalone driver for the fuzz harness, for environments without the
 * libFuzzer runtime (e.g. macOS Command Line Tools). Reads each file from
 * argv and pushes it through LLVMFuzzerTestOneInput under ASan/UBSan.
 *
 * Build with: make fuzz
 * Run:       ./build/bin/fuzz-standalone <file...>
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

int main(int argc, char **argv) {
    int i;

    if (argc < 2) {
        fprintf(stderr, "usage: %s <input-file>...\n", argv[0]);
        return 2;
    }

    for (i = 1; i < argc; i++) {
        FILE *stream;
        long length;
        uint8_t *data;

        stream = fopen(argv[i], "rb");
        if (stream == NULL) {
            perror(argv[i]);
            return 2;
        }
        if (fseek(stream, 0, SEEK_END) != 0) {
            perror(argv[i]);
            fclose(stream);
            return 2;
        }
        length = ftell(stream);
        if (length < 0) {
            perror(argv[i]);
            fclose(stream);
            return 2;
        }
        if (fseek(stream, 0, SEEK_SET) != 0) {
            perror(argv[i]);
            fclose(stream);
            return 2;
        }

        data = malloc((size_t)length > 0 ? (size_t)length : 1);
        if (data == NULL) {
            fprintf(stderr, "out of memory\n");
            fclose(stream);
            return 2;
        }
        if (length > 0 && fread(data, 1, (size_t)length, stream) != (size_t)length) {
            fprintf(stderr, "short read on '%s'\n", argv[i]);
            free(data);
            fclose(stream);
            return 2;
        }
        fclose(stream);

        LLVMFuzzerTestOneInput(data, (size_t)length);
        free(data);
    }

    return 0;
}