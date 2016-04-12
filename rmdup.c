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

int sortFile(char filename[])
{
  int fd[2];
  char buf[200];
  int nr, nw;
  pid_t pid;

  if (pipe(fd) != 0) {
    exit(1);
  }

  if ((pid = fork()) < 0) {
    perror("fork");
    exit(2);
  }

  if (pid > 0) {
    close(fd[READ]);
    int f = open(filename, O_RDONLY, S_IRUSR | S_IWUSR | S_IXUSR);
    while ((nr = read(f, buf, 200)) > 0) {
      if ((nw = write(fd[WRITE], buf, nr)) <= 0 || nw != nr) {
        exit(3);
      }
    }
    close(f);
    close(fd[WRITE]);
  }
  else {
    close(fd[WRITE]);
    dup2(fd[READ], STDIN_FILENO);
    close(fd[READ]);
    int f = open("files.txt", O_RDWR | O_CREAT , S_IRUSR | S_IWUSR | S_IXUSR);
    dup2(f, STDOUT_FILENO);
    execlp("sort", "sort", (char*) NULL);
    close(f);
  }

  return 0;
}

int main(int argc, char* argv[]) {
  int f;
  pid_t pid;

  if (argc != 2)
  {
      printf("Usage: %s <directory>\n", argv[0]);
      exit(1);
  }

  if ((f = open("files.txt", O_CREAT | O_TRUNC, S_IRWXU)) == -1)
  {
      perror("files.txt");
      exit(2);
  }

  if ((f = open("filestemp.txt", O_CREAT | O_TRUNC, S_IRWXU)) == -1)
  {
      perror("filestemp.txt");
      exit(3);
  }

  if ((pid = fork()) < 0) {
      perror("fork");
      exit(4);
  }

  if (pid == 0) {
      execl("./lsmdir","./lsmdir", argv[1], NULL);
  }

  else {
      waitpid(pid, NULL, 0);
      sortFile("filestemp.txt");
      remove("filestemp.txt");
  }

  return 0;
}
