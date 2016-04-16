#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define READ 0
#define WRITE 1

typedef struct{
    char name[256];
    char path[256];
    char link[512];
    char size[6];
    char mode[11];
    char date[20];
} fileSettings;

void storeFileInfo(char buff[], fileSettings *fs);
void removeDuplicate(fileSettings fs1, fileSettings fs2, FILE *destFile);
int sameContent(fileSettings fs1, fileSettings fs2);
int compareFiles(fileSettings fs1, fileSettings fs2);
void verifyFiles(char destFilePath[]);
int sortFile(char filename[]);
