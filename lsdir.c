#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    DIR * dirp;

    struct dirent *d;
    struct stat s;
    pid_t pid;

    if (argc != 2)
    {
        printf("Usage: %s <directory>\n", argv[0]);
        exit(1);
    }

    dirp = opendir(argv[1]);

    if (dirp == NULL)
    {
        perror(argv[1]);
        exit(2);
    }

    while((d = readdir(dirp)) != NULL)
    {
        char n[200];
        sprintf(n, "%s/%s", argv[1], d->d_name);

        if (lstat(n, &s) == -1)
        {
            perror(n);
            exit(3);
        }

        if (!(d->d_name[0] == '.' || strcmp(d->d_name, "files.txt") == 0 || strcmp(d->d_name, "filestemp.txt") == 0))
        {
          if (S_ISREG(s.st_mode))
          {
              int f = open("filestemp.txt", O_WRONLY | O_APPEND);
              sprintf(n, "%s|%d|%d|%d|%s\n", d->d_name, (int)s.st_size, s.st_mode, (int)s.st_mtime, argv[1]);
              dup2(f, STDOUT_FILENO);
              printf("%s", n);
              close(f);
          }
          else if (S_ISDIR(s.st_mode))
          {
              if ((pid = fork()) < 0) {
                perror("fork");
                exit(4);
              }

              else if (pid == 0) {
                  execl("./lsdir","./lsdir", n , NULL);
              }

              else {
                waitpid(pid, NULL, 0);
              }
          }
        }
    }

    closedir(dirp);
    return 0;
}
