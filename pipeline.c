#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#define FILE_NAME "EMSrawData.gz"
#define PLACE "bronx"

int main(){
  pid_t child1, child2, child3;       // 3 children = 3 commands 
  int pipes[4];                       // pipe with 4 file descriptors - read and write for both pipes! 
  int status;

  // 4 fds:
  // pipes[0] = read end of zcat --> grep pipe (read by grep) 
  // pipes[1] = write end of zcat --> grep pipe (written by zcat)
  // pipes[2] = read end of grep --> wc pipe (read by wc)
  // pipes[3] = write end of grep --> wc pipe (written by grep)

  
  // create pipes from the file descriptors and check for any errors/failures!
  if (pipe(pipes)  < 0) {
    perror("pipe1");
    exit(EXIT_FAILURE);
  }
  if (pipe(pipes+2)  < 0) {
    perror("pipe2");
    exit(EXIT_FAILURE);
  }
  
  child1 = fork();                      // child 1 = zcat
  if (child1 < 0){                      // if error, return error
    perror("fork1");
    exit(EXIT_FAILURE);
  }
  
  if(child1 == 0){                      // child = 0, parent > 0       
    if (dup2(pipes[1], 1) < 0)
      perror("dup: zcat");

    // close all the pipes!
    // don't worry the parent can open again for the next child                               
    for (int i=0; i < 4; i++){                              
        if (close(pipes[i]) < 0){                           
      fprintf(stderr, "close: zcat fd %d", i);                
      perror("");                                           
	}                                                       
    }

    if (execlp("/bin/zcat", "zcat", FILE_NAME, NULL) < 0){
      perror("exec: zcat");
    }
  }
  
  child2 = fork();                      // child 2 = grep
  if (child2 < 0){                                                                      
    perror("fork2");                                                                                                   
    exit(EXIT_FAILURE);
  }
 
  if (child2 == 0){  
    if (dup2(pipes[0], 0) < 0)
      perror("dup: grep 1");
    if (dup2(pipes[3], 1) < 0)
      perror("dup: grep 2");
    
    // close all the pipe fds                               
    for (int i=0; i < 4; i++){                              
        if (close(pipes[i]) < 0){                           
      fprintf(stderr, "close: grep fd %d", i);                
      perror("");                                           
	}                                                       
    }
    
    if(execlp("/bin/grep", "grep", "-i", PLACE, NULL) < 0) {
      perror("exec: grep");
    }
  }
  
  child3 = fork();                      // child 3 = wc
  if (child3 < 0){
    perror("fork3");
    exit(EXIT_FAILURE);
  }
 
  if (child3 == 0) {
    if (dup2(pipes[2], 0) < 0)
      perror("dup: wc");

    // close all the pipe fds
    for (int i=0; i < 4; i++){
	if (close(pipes[i]) < 0){
      fprintf(stderr, "close: wc fd %d", i);
      perror("");
	}
    }
    
  
    if (execlp("/usr/bin/wc", "wc", "-l", NULL) < 0) {
      perror("exec: wc");
    }
  }

  // close the parent pipe fds
  for (int i=0; i < 4; i++){
    if (close(pipes[i]) < 0){
      fprintf(stderr, "close: parent fd %d", i);
      perror("");
    }
  }

  // wait for every process to finish - make sure there aren't any zombie children!
  for (int i=0; i < 3; i++)
    wait(&status);
}

