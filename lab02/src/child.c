#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

int main(int argc, char *argv[], char *env[]) {
    pid_t pid, ppid;
    pid = getpid();
    ppid = getppid();
    printf("I child program. My name = %s, my pid = %d, my ppid = %d\n", argv[0], (int)pid, (int)ppid);
    printf("CHILD ENVIRONMENT:\n");
    char * filename = argv[1];
    char buffer[256];
    FILE *fp = fopen(filename, "r");
    if(fp)
    {
        while((fgets(buffer, 256, fp))!=NULL)
        {
            buffer[strcspn(buffer, "\n")] = '\0';
            printf("%s : %s\n",buffer,getenv(buffer));

        }
        fclose(fp);
    }
    exit(0);
}
