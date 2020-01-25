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
    int total_treated;
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
    pthread_mutex_lock(&mutex);
    for(int i = 0; i < imageCount; i++){
        printf("\n%d//%d--%d", listImage[i].treated, listImage[i].writed, i);
        if(listImage[i].treated != 1 && listImage[i].applying_treatment != 1){
            listImage[i].applying_treatment = 1;
            pthread_mutex_unlock(&mutex);
            return i;
        }
    }

    pthread_mutex_unlock(&mutex);
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
        pthread_mutex_lock(&mutex);
        thread_args->total_treated += 1;
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&cond);
    }
}

void* write_image_thread(void* args){
    ArgForThread* thread_args = (ArgForThread*) args;
    int count = thread_args->image_count;
    ImageToTreat* img = thread_args->images;
    int index;
    while(count >= thread_args->total_treated){
        index = find_image_to_write(count, img);
        if(index == -1){
            pthread_cond_wait(&cond,&mutex);
        }
        write_image_to_bitmap(&img[index]);
    }
}
enum EffectType string_to_effect(char* effect){
    printf("%s", effect);
    if(strcmp(effect,"blur") == 0){
        return BOXBLUR;
    } else if (strcmp(effect,"sharpen") == 0){
        return SHARPEN;
    } else if (strcmp(effect,"edge") == 0) {
        return EDGE;
    } else {
        printf("Choose a valid effect : blur sharpen edge");
        return -1;
    }
}

int main(int argc, char** argv) {
    ImageToTreat* listimg;
    if(argv[1] == NULL || argv[2] == NULL || argv[3] == NULL || argv[4] == NULL){
        printf("arguments format : ./in/ ./out/ 3 blur");
        return -1;
    }
    char* dir = argv[1];//"./images";
    char* out = argv[2];
    int threads_number = atoi(argv[3]);
    int count = count_img_in_dir(dir);
    printf("%d", strcmp(argv[4],"blur"));
    enum EffectType effect = string_to_effect(argv[4]);

    if(threads_number > count){
        printf("Please choose a directory with more images or choose less threads");
        return 0;
    }
    if(effect == -1){
        return effect;
    }

    list_image(dir, &listimg, count, out);
    ArgForThread* args = malloc(sizeof(int) + (sizeof(ImageToTreat) * count));
    args->images = listimg;
    args->image_count = count;
    args->effect = BOXBLUR;
    args->total_treated = 0;

    pthread_t threadList[threads_number];
    pthread_t threadWrite;
    pthread_create(&threadWrite, NULL, treat_image_thread, (void*)args);
    for(int i = 0; i < threads_number; i++){
        pthread_create(&threadList[i], NULL, write_image_thread, (void*)args);
    }
    pthread_join(threadWrite, NULL);
}
