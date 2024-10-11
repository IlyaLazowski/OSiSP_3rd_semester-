#include "lib.h"

int main (int argc, char *argv[])
{
    if(strcmp(argv[1],"dirwalk") == 0)
    {
      if(argc==3)processing_options( "./", 2,argv);
      if(argc==4)processing_options( argv[2], 3,argv);
      if(argv[2][0]!='-' && argc == 3){
        dirwalk (argv[2],'l');
        dirwalk (argv[2],'d');
        dirwalk (argv[2],'f');
      }
    }else printf("command not find.");
    if(argc>4)printf("too many command line arguments");
  return 0;
}

