#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "lib/bitmap.h"
#include <stdint.h>
#include <dirent.h>
#include <pthread.h>
#include "lib/edge-detect.c"
// ARG DIRECTORYNAME THREAD NUMBER

int main(int argc, char** argv) {
    Image img = open_bitmap("./images/bmp_tank1.bmp");
    Image new_i;
    apply_effect(&img, &new_i, SHARPEN);
    save_bitmap(new_i, "test_out.bmp");
}
