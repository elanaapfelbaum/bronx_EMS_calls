#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#define FILE_NAME "EMSrawData.gz"
#define PLACE "BRONX"

int main(){
  pid_t child1, child2, child3;
  int pipe1fd[2], pipe2fd[2];
  int status;
  if (pipe(pipe1fd)  < 0) {
    perror("pipe1");
    exit(EXIT_FAILURE);
  }

  if (pipe(pipe2fd)  < 0) {
    perror("pipe2");
    exit(EXIT_FAILURE);
  }
  
  child1 = fork();                      /* child 1 = zcat */
  if (child1 < 0){                      /* if error, return error */
    perror("fork1");
    exit(EXIT_FAILURE);
  }
  
  if(child1 ==  0){       
    close(pipe1fd[0]);
    dup2(pipe1fd[1], 1);
    if (execlp("/bin/zcat", "zcat", FILE_NAME, NULL) < 0){
      perror("exec: zcat");
    }
  }
  /* wait for child process to finish - avoid zombies */
  /*  waitpid(child1, &status, 0); */

  child2 = fork();                      /* child 2 = grep */                                                                                                      
  if (child2 < 0){                                                                      
    perror("fork2");                                                                                                   
    exit(EXIT_FAILURE);
  }
 
  if (child2 == 0){                                             
    close(pipe1fd[1]);            
    dup2(pipe1fd[0], 0);

    close(pipe2fd[0]);
    dup2(pipe2fd[1], 1);
    if(execlp("/bin/grep", "grep", "-i", PLACE, NULL) < 0) {
      perror("exec: grep");
    }
  }
  
  child3 = fork();                      /* child 3 = wc */
  if (child3 < 0){
    perror("fork3");
    exit(EXIT_FAILURE);
  }
 
  if (child3 == 0) {
    close(pipe2fd[1]);
    dup2(pipe2fd[0], 0);
    printf("about to run wc\n");
    if (execlp("/usr/bin/wc", "wc", "-l", NULL) < 0) {
      perror("exec: wc");
    }
  }
}

