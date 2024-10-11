#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>
void dirwalk(const char *dir, char parametr);
void processing_options(char *path , int number_of_argument_in_cmd , char *argv[]);

