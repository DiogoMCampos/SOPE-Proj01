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

int compareFiles(fileSettings fs1, fileSettings fs2)
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

  c1 = getc(f1);
  c2 = getc(f2);

  if (c1 == EOF && c2 == EOF)
    return 0;
  else {
    while ((c1 = getc(f1)) != EOF) {
      if ((c2 = getc(f2)) != c1) {
        return 1;
      }
    }
  }

  fclose(f1);
  fclose(f2);

  return 0;
}

void verifyFiles() {
    FILE *f;
    fileSettings fs1;
    fileSettings fs2;
    char buff[256];
    char *s;
    char sep[2] = "|";

    if ((f = fopen("files.txt", "r")) == NULL)
    {
        perror("files.txt");
        exit(1);
    }

    if (fgets(buff, 256, f) == NULL)
    {
       printf("The file is empty.\n");
       exit(2);
    }

    s = strtok(buff, sep);
    strcpy(fs1.name, s);
    printf("%s\n", fs1.name);

    s = strtok(NULL, sep);
    strcpy(fs1.size, s);
    printf("%s\n", fs1.size);

    s = strtok(NULL, sep);
    strcpy(fs1.mode, s);
    printf("%s\n", fs1.mode);

    s = strtok(NULL, sep);
    strcpy(fs1.date, s);
    printf("%s\n", fs1.date);

    s = strtok(NULL, "\n");
    strcpy(fs1.path, s);
    printf("%s\n\n", fs1.path);

    sprintf(fs1.link, "%s/%s", fs1.path, fs1.name);

    while(fgets(buff, 256, f) != NULL)
    {
        s = strtok(buff, sep);
        strcpy(fs2.name, s);
        printf("%s\n", fs2.name);

        s = strtok(NULL, sep);
        strcpy(fs2.size, s);
        printf("%s\n", fs2.size);

        s = strtok(NULL, sep);
        strcpy(fs2.mode, s);
        printf("%s\n", fs2.mode);

        s = strtok(NULL, sep);
        strcpy(fs2.date, s);
        printf("%s\n", fs2.date);

        s = strtok(NULL, "\n");
        strcpy(fs2.path, s);
        printf("%s\n\n", fs2.path);

        sprintf(fs2.link, "%s/%s", fs2.path, fs2.name);

        // No caso de haver 3 ficheiros com o mesmo nome e, pela ordem do files.txt, o 1º
        // ser diferente do segundo mas igual ao terceiro, não deve funcionar corretamente.
        if ((strcmp(fs1.name, fs2.name) == 0) &&
            (strcmp(fs1.size, fs2.size) == 0) &&
            (strcmp(fs1.mode, fs2.mode) == 0) &&
            (compareFiles(fs1, fs2) == 0))
        {
            unlink(fs2.link);
            link(fs2.link, fs1.link);
            printf("%s e %s sao iguais.\n\n", fs1.link, fs2.link);
        }
        else {
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
  pid_t pid2;

  int std_in = dup(STDIN_FILENO);
  int std_out = dup(STDOUT_FILENO);

  if (pipe(fd) != 0) {
    exit(1);
  }

  if ((pid = fork()) < 0) {
    perror("fork");
    exit(2);
  }

  if (pid == 0) {

    close(fd[READ]);
    int f = open(filename, O_RDONLY, S_IRUSR | S_IWUSR | S_IXUSR);
    while ((nr = read(f, buf, 200)) > 0) {
      if ((nw = write(fd[WRITE], buf, nr)) <= 0 || nw != nr) {
        exit(3);
      }
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
    dup2(f, STDOUT_FILENO);
    if((pid2 = fork()) == 0)
        execlp("sort", "sort", (char*) NULL);
    waitpid(pid2, NULL, 0);
    close(f);
  }

  dup2(std_in, STDIN_FILENO);
  dup2(std_out, STDOUT_FILENO);
  return 0;
}

int main(int argc, char* argv[]) {
  int f1;
  int f2;
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

  close(f1);
  close(f2);

  if ((pid = fork()) < 0) {
      perror("fork");
      exit(4);
  }
  else if (pid == 0) {
      execlp("./lsmdir","./lsmdir", argv[1], NULL);
  }
  else {
      waitpid(pid, NULL, 0);
      sortFile("filestemp.txt");
      remove("filestemp.txt");
      verifyFiles();
  }
  return 0;
}
