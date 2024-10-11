#include "lib.h"
void dirwalk(const char *dir, char parametr)
{
struct dirent *entry;
DIR *dp;
char path[1024];
struct stat st;
int lstat(const char *path, struct stat *buf);
if ((dp = opendir(dir)) == NULL) {
    perror("opendir");
    return;
}
while ((entry = readdir(dp)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
    snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);
    if (lstat(path, &st) == -1) {
        perror("lstat");
        continue;
    }
    if (S_ISDIR(st.st_mode)) {
        if (parametr == 'd')
            printf("%s\n", path);
        dirwalk(path, parametr);
    } else if (S_ISREG(st.st_mode)) {
        if (parametr == 'f')
            printf("%s\n", path);
    } else if (S_ISLNK(st.st_mode)) {
        if (parametr == 'l')
            printf("%s\n", path);
    }
}
closedir(dp);
}

void processing_options(char *path , int number_of_argument_in_cmd , char *argv[] )
{
    if(argv[number_of_argument_in_cmd][0]=='-')
    {
    int i=1;
    while(argv[number_of_argument_in_cmd][i]!='\0')
      {
        if(argv[number_of_argument_in_cmd][i]=='l')dirwalk (path,'l');
        if(argv[number_of_argument_in_cmd][i]=='f')dirwalk (path,'f');
        if(argv[number_of_argument_in_cmd][i]=='d')dirwalk (path,'d');
        i++;
      }
    }
}
