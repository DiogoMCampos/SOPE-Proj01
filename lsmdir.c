#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
# include <unistd.h>

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
    
    printf("%s\n", argv[1]);
    
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
            printf(" %-20s - %s\n", d->d_name, "regular");
        }
        else if (S_ISDIR(s.st_mode))
        {
            if (!((strcmp(d->d_name, ".") == 0) || (strcmp(d->d_name, "..") == 0)))
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
    
    printf("\n");
    
    closedir(dirp);
    return 0;
}