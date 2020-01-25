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
    Image out;
    int treated;
    int writed;
}ImageToTreat;

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

int count_img_in_dir(char* dir_path){
    DIR * dirp;
    struct dirent * entry;
    dirp = opendir(dir_path);

    int count =0;
    while ((entry = readdir(dirp)) != NULL) {
        if (entry->d_type == DT_REG) {
            count++;
        }
    }
    return count;
}

int find_image_to_treat(int imageCount, ImageToTreat* listImage){
    for(int i = 0; i < imageCount; i++){
        printf("\n%d//%d--%d\n", listImage[i].treated, listImage[i].writed, i);
        if(listImage[i].treated != 1){
            return i;
        }
    }
    return -1;
}

int find_image_to_write(int imageCount, ImageToTreat* listImage){
    for(int i = 0; i < imageCount; i++){
        printf("\n%d//%d--%d\n", listImage[i].treated, listImage[i].writed, i);
        if(listImage[i].writed != 1 && listImage[i].treated == 1){
            return i;
        }
    }
    return -1;
}

void write_image_to_bitmap(ImageToTreat* listimg, int count) {
    //int index = find_image_to_write(count, listimg);
    int index = find_image_to_write(count, listimg);
    printf("\n%d/-/%s\n", listimg[index].out.bmp_header.width, listimg[index].path_out);
    if(index == -1) {
        return;
    }
    save_bitmap(listimg[index].out, listimg[index].path_out);
    listimg[index].writed = 1;
}

void treat_image(ImageToTreat* img, int count, enum EffectType effect){
    //printf("%s/-/%s/-/%d\n", img->path, img->path_out, img->img.bmp_header.width);
    //int index = find_image_to_treat(count, &img);
    //printf("index : %d\n", index);
    int index = find_image_to_treat(count, img);
    if(index == -1) {
        return;
    }
    apply_effect(&img[index].img, &img[index].out, effect);
    img[index].treated = 1;
}

void display(ImageToTreat* listimg, int count){
    for(int i = 0; i < count; i++){
        printf("\n%d/;;/%d", listimg[i].treated, listimg[i].writed);
    }
}

int main(int argc, char** argv) {
    ImageToTreat* listimg;
    char* dir = "./images";
    int count = count_img_in_dir(dir);
    list_image(dir, &listimg, count, "./example");

    for(int i = 0; i < count; i++){
        treat_image(listimg, count, SHARPEN);
        //display(listimg, count);
        write_image_to_bitmap(listimg, count);
    }
}
