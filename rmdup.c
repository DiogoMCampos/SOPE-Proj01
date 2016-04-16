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

void storeFileInfo(char buff[], fileSettings *fs)
{
    char sep[2] = "|";
    char *s;

    s = strtok(buff, sep);
    strcpy(fs->name, s);
    s = strtok(NULL, sep);
    strcpy(fs->size, s);
    s = strtok(NULL, sep);
    strcpy(fs->mode, s);
    s = strtok(NULL, sep);
    strcpy(fs->date, s);
    s = strtok(NULL, "\n");
    strcpy(fs->path, s);
    sprintf(fs->link, "%s/%s", fs->path, fs->name);
}

void removeDuplicate(fileSettings fs1, fileSettings fs2, FILE *destFile)
{
    unlink(fs2.link);
    link(fs1.link, fs2.link);
    fprintf(destFile, "%s is now an hard link to %s.\n", fs2.link, fs1.link);
}

int sameContent(fileSettings fs1, fileSettings fs2)
{
    FILE *f1, *f2;
    char c1, c2;

    if ((f1 = fopen(fs1.link, "r")) == NULL) {
      perror(fs1.name);
      exit(1);
    }

    if ((f2 = fopen(fs2.link, "r")) == NULL) {
      perror(fs1.name);
      exit(2);
    }

    do {
      c1 = getc(f1);
      c2 = getc(f2);
    } while (c1 == c2 && c1 != EOF);

    fclose(f1);
    fclose(f2);

    if (c1 != c2)
        return -1;

    return 0;
}

// returns 0 if files are equal, 1 if files have the same name, size and mode
// but different contents and -1 if the files are completely different
int compareFiles(fileSettings fs1, fileSettings fs2)
{
    if ((strcmp(fs1.name, fs2.name) == 0) &&
        (strcmp(fs1.size, fs2.size) == 0) &&
        (strcmp(fs1.mode, fs2.mode) == 0))
        {
            if (sameContent(fs1, fs2) == 0)
                return 0;
            else
                return 1;
        }
    else
        return -1;

}

void verifyFiles(char destFilePath[]) {
    FILE *f, *destFile;
    fileSettings fs1;
    fileSettings fs2;
    char buff[256];

    if ((f = fopen("files.txt", "r")) == NULL)
    {
        perror("files.txt");
        exit(1);
    }

    if ((destFile = fopen(destFilePath, "w")) == NULL)
    {
        perror(destFilePath);
        exit(2);
    }

    if (fgets(buff, 256, f) == NULL)
    {
       printf("The file is empty.\n");
       exit(3);
    }

    storeFileInfo(buff, &fs1);

    while (fgets(buff, 256, f) != NULL) {
        storeFileInfo(buff, &fs2);

        int cmpResult = compareFiles(fs1, fs2);

        if (cmpResult == 0) {
            removeDuplicate(fs1, fs2, destFile);
        }
        else if (cmpResult == -1) {
            strcpy(fs1.name, fs2.name);
            strcpy(fs1.path, fs2.path);
            strcpy(fs1.size, fs2.size);
            strcpy(fs1.mode, fs2.mode);
            strcpy(fs1.date, fs2.date);
            strcpy(fs1.link, fs2.link);
        }

    }

    fclose(f);
}

int sortFile(char filename[])
{
    int fd[2];
    char buf[200];
    int nr, nw;
    pid_t pid;

    int std_in = dup(STDIN_FILENO);
    int std_out = dup(STDOUT_FILENO);

    if (pipe(fd) != 0) {
        exit(1);
    }

    if ((pid = fork()) < 0) {
        perror("Fork failed");
        exit(2);
    }

    if (pid == 0) {
        close(fd[READ]);

        int f = open(filename, O_RDONLY);
        if (f == -1) {
            perror(filename);
            exit(3);
        }

        while ((nr = read(f, buf, 200)) > 0) {
            if ((nw = write(fd[WRITE], buf, nr)) <= 0 || nw != nr)
                exit(4);
        }

        close(f);
        close(fd[WRITE]);
        exit(0);
    }
    else {
        waitpid(pid, NULL, 0);
        close(fd[WRITE]);
        dup2(fd[READ], STDIN_FILENO);
        close(fd[READ]);

        int f = open("files.txt", O_RDWR);
        if (f == -1) {
            perror(filename);
            exit(3);
        }

        dup2(f, STDOUT_FILENO);

        pid_t pid2;

        if ((pid2 = fork()) < 0) {
            perror("Fork failed");
            exit(4);
        }

        if (pid2 == 0)
            execlp("sort", "sort", (char*) NULL);
        else {
            waitpid(pid2, NULL, 0);
            close(f);
        }
    }

    dup2(std_in, STDIN_FILENO);
    dup2(std_out, STDOUT_FILENO);
    return 0;
}

int main(int argc, char* argv[]) {
    int f1, f2, f3;
    pid_t pid;

    if (argc != 2)
    {
        printf("Usage: %s <directory>\n", argv[0]);
        exit(1);
    }

    if ((f1 = open("files.txt", O_CREAT | O_TRUNC, S_IRWXU)) == -1)
    {
        perror("files.txt");
        exit(2);
    }

    if ((f2 = open("filestemp.txt", O_CREAT | O_TRUNC, S_IRWXU)) == -1)
    {
        perror("filestemp.txt");
        exit(3);
    }

    char destFilePath[512];
    sprintf(destFilePath, "%s/hlinks.txt", argv[1]);

    if ((f3 = open(destFilePath, O_CREAT | O_TRUNC, S_IRWXU)) == -1)
    {
      perror(destFilePath);
      exit(4);
    }

    close(f1);
    close(f2);
    close(f3);

    if ((pid = fork()) < 0) {
        perror("Fork failed");
        exit(5);
    }
    else if (pid == 0) {
        execlp("./lsmdir","./lsmdir", argv[1], NULL);
    }
    else {
        waitpid(pid, NULL, 0);
        sortFile("filestemp.txt");
        remove("filestemp.txt");
        verifyFiles(destFilePath);
    }
    return 0;
}
