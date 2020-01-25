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
    int applying_treatment;
}ImageToTreat;

typedef struct ArgForThread {
    ImageToTreat* images;
    int image_count;
    enum EffectType effect;
}ArgForThread;

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
            list[count].applying_treatment = 0;
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
        printf("\n%d//%d--%d", listImage[i].treated, listImage[i].writed, i);
        if(listImage[i].treated != 1 && listImage[i].applying_treatment != 1){
            listImage[i].applying_treatment = 1;
            return i;
        }
    }
    return -1;
}

int find_image_to_write(int imageCount, ImageToTreat* listImage){
    for(int i = 0; i < imageCount; i++){
        printf("\n%d//%d--%d", listImage[i].treated, listImage[i].writed, i);
        if(listImage[i].writed != 1 && listImage[i].treated == 1){
            return i;
        }
    }
    return -1;
}

void write_image_to_bitmap(ImageToTreat* listimg) {
    save_bitmap(listimg->out, listimg->path_out);
    listimg->writed = 1;
}

void treat_image(ImageToTreat* img, enum EffectType effect){
    apply_effect(&img->img, &img->out, effect);
    img->treated = 1;
}

void display(ImageToTreat* listimg, int count){
    for(int i = 0; i < count; i++){
        printf("\n%d/;;/%d", listimg[i].treated, listimg[i].writed);
    }
}

void* treat_image_thread(void* args){
    ArgForThread* thread_args = (ArgForThread*) args;
    int count = thread_args->image_count;
    ImageToTreat* img = thread_args->images;
    enum EffectType effect = thread_args->effect;
    int index;
    while((index = find_image_to_treat(count, img)) != -1) {
        printf("index : %d", index);
        treat_image(&img[index], effect);
    }
}

void* write_image_thread(void* args){
    ArgForThread* thread_args = (ArgForThread*) args;
    int count = thread_args->image_count;
    ImageToTreat* img = thread_args->images;
    enum EffectType effect = thread_args->effect;
    int index;
    while((index = find_image_to_write(count, img)) != -1){
        write_image_to_bitmap(&img[index]);
    }
}

int main(int argc, char** argv) {
    ImageToTreat* listimg;
    char* dir = "./images";
    int count = count_img_in_dir(dir);
    pthread_t threadList[count];
    pthread_t threadWrite;
    list_image(dir, &listimg, count, "./example");
    ArgForThread* args = malloc(sizeof(int) + (sizeof(ImageToTreat) * count));
    args->images = listimg;
    args->image_count = count;
    args->effect = BOXBLUR;
    pthread_create(&threadWrite, NULL, treat_image_thread, (void*)args);
    pthread_join(threadWrite, NULL);
    pthread_create(&threadList[0], NULL, write_image_thread, (void*)args);
    pthread_join(threadList[0], NULL);
}
