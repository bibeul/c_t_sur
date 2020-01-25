#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "lib/bitmap.h"
#include <stdint.h>
#include <dirent.h>
#include <pthread.h>
#include <string.h>
#include "lib/edge-detect.c"
// ARG DIRECTORYNAME THREAD NUMBER

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex =  PTHREAD_MUTEX_INITIALIZER ;

typedef struct ImageToTreat {
    char* path;
    char* path_out;
    Image img;
    int treated;
    int writed;
}ImageToTreat;

void write_image_to_bitmap(Image* newImage, char* path) {
    save_bitmap(*newImage, path);
}

char* create_path(char* path, char* imageName) {
    char* full_path = malloc(strlen(path) + strlen(imageName) + 1);
    strcpy(full_path, path);
    strcat(full_path, "/");
    strcat(full_path, imageName);
    return full_path;
}

void list_image(char* dir_path, ImageToTreat** listImage, int imageCount, char* dir_out){
    ImageToTreat* list = malloc(sizeof(ImageToTreat) * imageCount);
    DIR * dirp;
    struct dirent * entry;
    dirp = opendir(dir_path);

    int count =0;
    while ((entry = readdir(dirp)) != NULL) {
        if (entry->d_type == DT_REG) {
            list[count].path = create_path(dir_path, entry->d_name);
            list[count].img = open_bitmap(list[count].path);
            list[count].treated = 0;
            list[count].writed = 0;
            list[count].path_out = create_path(dir_out, entry->d_name);
            count++;
        }
    }
    closedir(dirp);
    *listImage=list;
}

int main(int argc, char** argv) {
    ImageToTreat* listimg;
    list_image("./images", &listimg, 3, "./exemple");

    for(int i=0; i < 3; i++){
        printf("%s/-/%s/-/%d\n", listimg[i].path, listimg[i].path_out, listimg[i].img.bmp_header.width);
    }
    
    Image img = open_bitmap("./images/bmp_tank1.bmp");
    Image new_i;
    apply_effect(&img, &new_i, SHARPEN);
    save_bitmap(new_i, "test_out.bmp");
}
