#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    DIR * dirp;
    
    struct dirent *d;
    struct stat s;
    
    if (argc != 2)
    {
        printf("Argumentos nao validos\n");
        return 1;
    }
    
    dirp = opendir(argv[1]);
    
    if (dirp == NULL)
    {
        printf("Diretorio nao existente\n");
        return 2;
    }
    
    while((d = readdir( dirp)) != NULL)
    {

        char n[200];
        sprintf(n,"%s/%s",argv[1],d->d_name);
        
        if (lstat(n, &s) == -1)
        {
            printf("Erro ao definir a stat\n");
            printf("%s\n", argv[1]);
            return 3;
        }
        
        if (S_ISREG(s.st_mode)) 
        {
            int f = open("files.txt",  O_WRONLY | O_CREAT | O_EXCL , S_IRUSR | S_IWUSR | S_IXUSR);
            sprintf(n, "%-25s %-20s %-8d %-10d %-10d\n", d->d_name, argv[1], (int)s.st_size, s.st_mode, (int)s.st_mtime);
            dup2(f,STDOUT_FILENO);
            printf("%s", n);
            close(f);
        }
        else if (S_ISDIR(s.st_mode))
        {
            if (!((strcmp(d->d_name, ".") == 0) || (strcmp(d->d_name, "..")  == 0) || (strcmp(d->d_name, ".git")  == 0)))
            {
                if (fork() == 0)
                {
                    
                    execl("./lsmdir","./lsmdir", n , NULL);
                }
            }
        }
        else
        {
            printf(" %-20s - %s\n", d->d_name, "outro");
        }
    }
    
    int f = open("files.txt", O_RDWR | O_CREAT , S_IRUSR | S_IWUSR | S_IXUSR);
    dup2(f,STDIN_FILENO);
    close(f);
    f = open("files.txt", O_RDWR | O_CREAT , S_IRUSR | S_IWUSR | S_IXUSR);
    dup2(f,STDOUT_FILENO);
    execlp("sort", "sort", "files.txt", NULL);
    close(f);
    
    closedir(dirp);
    return 0;
}