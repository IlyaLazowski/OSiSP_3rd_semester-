#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>

void environment_output(char *envp[]) {
    int count = 0;
    printf("PARENT ENVIRONMENT OUTPUT:\n");
    while (envp[count] != NULL) {
        printf("%s\n", envp[count]);
        count++;
    }
}

int compare_env(const void *a, const void *b) {
    const char *str_a = *(const char **)a;
    const char *str_b = *(const char **)b;
    return strcmp(str_a, str_b);
}

int main(int argc, char *argv[], char *envp[]) {

    int count_child_programm = 0;
    extern char **environ;
    char *child_path = NULL;
    int num_env_vars = 0;

    while (envp[num_env_vars] != NULL) {
        num_env_vars++;
    }
    qsort(envp, num_env_vars, sizeof(char *), compare_env);
    environment_output(envp);

    while (1) {
        char key;
        printf("Введите символ для управления:");
        key = getchar();
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {}
        if (key == 'q' || count_child_programm == 100) {
            printf("Выход из родительского окружения.");
            break;
        } else {
            if (key == '*' || key == '&' || key == '+') {

                pid_t pid_for_child_process = fork();
                if (pid_for_child_process == -1)
                    printf("Ошибка при создании дочернего процесса, код ошибки - %d.", errno);
                else if (pid_for_child_process == 0) {
                    char child_name[20];
                    sprintf(child_name, "child_%02d", count_child_programm);
                   // count_child_programm++;
                    char *child_argv[] = {child_name,argv[1], NULL};
                    if (key == '+') {
                        child_path = getenv("CHILD_PATH");
                    }
                    if (key == '*') {
                        while (*envp) {
                            if (strncmp(*envp, "CHILD_PATH=", 11) == 0) {
                                child_path = *envp + 11;
                                break;
                            }
                            envp++;
                        }
                    }
                    if (key == '&') {
                        while (*environ) {
                            if (strncmp(*environ, "CHILD_PATH=", 11) == 0) {
                                child_path = *environ + 11;
                                break;
                            }
                            environ++;
                        }
                    }
                    if (child_path == NULL) {
                        printf("Не удалось получить путь к дочерней программе из окружения.");
                        exit(EXIT_FAILURE);
                    }
                    execve(child_path, child_argv, envp);
                } else{
                      count_child_programm++;
                      wait(NULL);
                }

            } else
                printf("Данный символ не может быть обработан.");
        }
    }

    return 0;
}
