#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <string.h>
#define FILE_NAME "EMSrawData.gz"

// using string comparisons to check whether the input is any of the five boroughs
int isBorough(char *place){
  if (strcmp(place, "brooklyn") != 0 && strcmp(place, "bronx") != 0 && strcmp(place, "manhattan") != 0 &&
      strcmp(place, "queens") != 0 && strcmp(place, "staten island") != 0){
    return 0;
  }
  return 1;
}
  
int main(){
  // simulating this pipeline: zcat EMSrawData.gz | grep -i bronx | wc -l
  // main function serves as the parent and each commad is a new child (i.e. fork)
  pid_t child1, child2, child3;   
  int pipes[4];       // pipe with 4 file descriptors - read and write for both pipes! 
  int status;
  char PLACE[20];
  
  // allowing user input - user can search the amount of calls in any borough, not just the bronx!
  // if the file is empty, it will return zero
  // if the file doesn't exit it will give you an error
  printf("Search EMS data for the amount of calls in your favorite borough!!\n");
  printf("Pick a borough:  ");
  scanf("%s", PLACE);

  
  // ensure that only boroughs are valid input
  // turn the string to all lowercase so that it can be compared
  // ignores leading and trailing whitespace 
  for (int i=0; PLACE[i]; i++){
    PLACE[i] = tolower(PLACE[i]);
  }

  // if the input isn't equal to any of the 5 boroughs then it is an invalid input :(
  /*  if (!isBorough(PLACE))
    printf("invalid borough!\n");
  */
  
  while (isBorough(PLACE) == 0){
    printf("Invalid borough try again!\n");
    printf("Pick a borough: ");
    scanf("%s", PLACE);
    
    for (int i=0; PLACE[i]; i++){
      PLACE[i] = tolower(PLACE[i]);
    }
  }
    
  printf("Counting the amount of calls from %s... hang tight!\n", PLACE);                               
  sleep(1); // just to give a sec before records the answer
 
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

  // CHILD 1 = zcat
  child1 = fork();                 
  if (child1 < 0){       // check error
    perror("fork1");
    exit(EXIT_FAILURE);
  }
  
  else if (child1 == 0){       // child = 0, parent > 0       
    if (dup2(pipes[1], 1) < 0){
      perror("dup: zcat");
      exit(EXIT_FAILURE);
    } 
    // close all the pipes!
    // don't worry the parent can open again for the next child                               
    for (int i=0; i < 4; i++){                              
        if (close(pipes[i]) < 0){                           
	  fprintf(stderr, "close: zcat fd %d - ", i);      // will tell you which fd had trouble closing          
	  perror("");
	  exit(EXIT_FAILURE);
	}                                                       
    }
    if (execlp("/bin/zcat", "zcat", FILE_NAME, NULL) < 0){
      perror("exec: zcat");
      exit(EXIT_FAILURE);
    }
  }
  
  // CHILD 2 = grep
  child2 = fork(); 
  if (child2 < 0){                                                                      
    perror("fork2");                                                                                                   
    exit(EXIT_FAILURE);
  }

  // grep needs to deal with the write side of pipe 2 and the read side of pipe 1
  // this means 2 dups are necessary
  else if (child2 == 0){  
    if (dup2(pipes[0], 0) < 0){
      perror("dup: grep 1");
      exit(EXIT_FAILURE);
    }
    if (dup2(pipes[3], 1) < 0){
      perror("dup: grep 2");
      exit(EXIT_FAILURE);
    }
    // close all the pipe fds                               
    for (int i=0; i < 4; i++){                              
        if (close(pipes[i]) < 0){                           
	  fprintf(stderr, "close: grep fd %d - ", i);                
	  perror("");                                           
	  exit(EXIT_FAILURE);
	}                                                       
    }
    if (execlp("/bin/grep", "grep", "-i", PLACE, NULL) < 0) {
      perror("exec: grep");
      exit(EXIT_FAILURE);
    }
  }

  // CHILD 3 = wc
  child3 = fork();                
  if (child3 < 0){
    perror("fork3");
    exit(EXIT_FAILURE);
  }
 
  else if (child3 == 0) {
    if (dup2(pipes[2], 0) < 0){
      perror("dup: wc");
      exit(EXIT_FAILURE);
    }
    // close all the pipe fds
    for (int i=0; i < 4; i++){
	if (close(pipes[i]) < 0){
	  fprintf(stderr, "close: wc fd %d - ", i);
	  perror("");
	  exit(EXIT_FAILURE);
	}
    }
    if (execlp("/usr/bin/wc", "wc", "-l", NULL) < 0) {
      perror("exec: wc");
    }
  }

  // close the parent pipe fds
  for (int i=0; i < 4; i++){
    if (close(pipes[i]) < 0){
      fprintf(stderr, "close: parent fd %d - ", i);
      perror("");
      exit(EXIT_FAILURE);
    }
  }

  // wait for every process to finish - make sure there aren't any zombie children!
  for (int i=0; i < 3; i++){
    if (wait(&status) < 0){
      perror("wait");
      exit(EXIT_FAILURE);
    }
  }
}

