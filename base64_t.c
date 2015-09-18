#include <stdio.h>
#include "base64.h"

int
main(int argc, char **argv) {
    char enc[4096], dec[4096];
    int i;

    i = base64_enc(argv[1], 0, enc, sizeof(enc));
    printf("base64_enc: i=%d buf=%s\n", i, enc);
    i = base64_dec(enc, i, dec, sizeof(dec));
    printf("base64_dec: i=%d buf=%s\n", i, dec);
    return 0;
}
