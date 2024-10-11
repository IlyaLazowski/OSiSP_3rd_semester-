#define _POSIX_C_SOURCE 100
#include<signal.h>
#include<stdio.h>
#include<stdbool.h>
#include<sys/types.h>
#include<sys/time.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

#include<sys/wait.h>
#define MAX_SIZE_CHILD_PROCESS 100

pid_t array_of_child_program_pid[MAX_SIZE_CHILD_PROCESS];
int count_of_child_process = 0;
int ret_value;
int process_completion_status;



int main() {

  char line_to_process_the_operation[16];
  while(fgets(line_to_process_the_operation,sizeof(line_to_process_the_operation),stdin)){
    int num;
    if(line_to_process_the_operation[0]=='q'){
      while(count_of_child_process>0){
        count_of_child_process--;
        if(kill(array_of_child_program_pid[count_of_child_process],SIGKILL))perror("kill");
        pid_t local_pid = waitpid(array_of_child_program_pid[count_of_child_process],&process_completion_status,0);
        if(local_pid==-1)perror("waitpid");
      }
      printf("---ALL PROCESS DELETED---\n");
      break;
    }
    else if(line_to_process_the_operation[0]=='+'){
      if(count_of_child_process<MAX_SIZE_CHILD_PROCESS){
        printf("---CREATED CHILD PROCESS WITH NUMBER %d---\n",count_of_child_process);
        pid_t local_pid = fork();
        if(local_pid > 0) array_of_child_program_pid[count_of_child_process] = local_pid;
        else if(!local_pid){
          char *const argv[] = {NULL};
          execve("build/debug/child", argv, NULL);
        }
        count_of_child_process++;
      }else printf("Достигнуто максимальное количество дочерних процессов.\n");
    }
    else if(line_to_process_the_operation[0]=='-'){
      if(count_of_child_process==0) printf("На данный момент нет дочерних процессов.\n");
      else if(count_of_child_process>0){

        printf("---KILLED PROCESS WITH NUMBER %d---\n",count_of_child_process-1);
        count_of_child_process--;
        if(kill(array_of_child_program_pid[count_of_child_process],SIGKILL))perror("kill");
        pid_t local_pid = waitpid(array_of_child_program_pid[count_of_child_process],&process_completion_status,0);
        if(local_pid==-1)perror("waitpid");
        if(WIFEXITED(process_completion_status)) printf("process with pid - %d exited with status %d\n", array_of_child_program_pid[count_of_child_process], WEXITSTATUS(process_completion_status));
      }
    }
    else if(line_to_process_the_operation[0]=='l'){
      printf("Родительский процесс PID = %d\n", getpid());
      if(count_of_child_process==0) printf("На данный момент нет дочерних процессов.\n");
      else for(int i = 0;i<count_of_child_process;i++) printf("Дочерний процесс %d PID = %d\n",i,array_of_child_program_pid[i]);

    }
    else if(line_to_process_the_operation[0]=='k'){
      if(count_of_child_process==0) printf("На данный момент нет дочерних процессов.\n");
      else{
        while(count_of_child_process>0){
          count_of_child_process--;
          if(kill(array_of_child_program_pid[count_of_child_process],SIGKILL))perror("kill");
          pid_t local_pid = waitpid(array_of_child_program_pid[count_of_child_process],&process_completion_status,0);
          if(local_pid==-1)perror("waitpid");
        }
        printf("---ALL PROCESS DELETED---\n");
      }
    }
    else if(line_to_process_the_operation[0]=='s'){
      if(sscanf(line_to_process_the_operation,"s%d",&num)==1){
        if(num<count_of_child_process)kill(array_of_child_program_pid[num],SIGUSR1);
        if(num>=count_of_child_process) printf("Нет процесса под таким номером.\n");
      }
      else {
        for(int i=0;i<count_of_child_process;i++)kill(array_of_child_program_pid[i],SIGUSR1);
      }
    }
    else if(line_to_process_the_operation[0]=='g'){
                alarm(0);
      if(sscanf(line_to_process_the_operation,"g%d",&num)==1){
        if(num<count_of_child_process)kill(array_of_child_program_pid[num],SIGUSR2);
        if(num>=count_of_child_process) printf("Нет процесса под таким номером.\n");
      } else for(int i=0;i<count_of_child_process;i++)kill(array_of_child_program_pid[i],SIGUSR2);
    }
    else if (line_to_process_the_operation[0] == 'p' && sscanf(line_to_process_the_operation, "p%d", &num) == 1) {
    if (num < count_of_child_process) {
        // Запрещаем всем C_k вывод статистики
        for (int i = 0; i < count_of_child_process; i++){
          if(i==num)continue;
          else  kill(array_of_child_program_pid[i], SIGUSR1);
        }







            fd_set rfds;
    struct timeval tv;
    int retval;

    /* Ожидание ввода на stdin (fd 0) */
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);

    /* Ожидание до пяти секунд. */
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    retval = select(1, &rfds, NULL, NULL, &tv);
    if (retval == -1)
        perror("select()");
    else if (retval) {
        char c;
        scanf("%c", &c);
        if (c == 'g') {
          for(int i=0;i<count_of_child_process;i++)kill(array_of_child_program_pid[i],SIGUSR2);
          continue;
        }
    } else
    {
      for(int i=0;i<count_of_child_process;i++)kill(array_of_child_program_pid[i],SIGUSR2);
      continue;
    }

fflush(stdin);
rewind(stdin);

    } else {
        printf("Нет процесса под таким номером.\n");
    }
}
  //  else printf("Операция введена некорректно\n");
  }
  return 0;
}

