#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define FILE_NAME "EMSrawData.gz"
#define ZIP_CODE  "10463"
#define PLACE     "BRONX"

int main(){
  /*printf("This program will tell you the number of EMS calls in a NYC borough of interest\n");                                             
  printf("Which borough would you like to search? Type here:  ");                                                                         
  scanf("%c", &PLACE);
  printf("Checking... Hang tight! :) \n");
  printf("Number of calls: "); */

  pid_t procID;                                 /* process id */ 
  int pipefd[2];
  pipe(pipefd);                                 /* creates a pipe with the 2 file descriptors - 0 = read, 1 = write */
  procID = fork();                              /* branch to a child pipe */
  
  if (procID < 0){
    perror("fork");                             /* if there was an error with the fork */
    exit(EXIT_FAILURE);
  }

  if (procID > 0){                               /* reads from the child pipe */
    close(pipefd[1]);                         /* closes the read end of the pipe */
    dup2(pipefd[0], 0);                       /* replace stdin with input file */
    execlp("zcat", "zcat", FILE_NAME, NULL);           /* execute the grep operator = first child pipe */
    close(pipefd[0]);  
     
    /* fork again to the next pipe */
    pid_t procID2 = fork();
    if (procID2 < 0){                            /* error */
     perror("fork");
     exit(EXIT_FAILURE);
    }
    
    if (procID2 == 0){
      close(pipefd[0]);                       /* close the unused read descriptor */
      dup2(pipefd[1], 1);                     /* replace stdin with input file */
      execlp("wc", "wc", "-l", NULL);      /* execute the wc operator = child of the child */
      close(pipefd[1]); 
    }
  }
  
  else{                                       /* parent */
    close(pipefd[0]);                         /* close unused write end of the pipe */
    dup2(pipefd[1], 1);                       /* replace stdout with output file  */
    execlp("grep", "-i", PLACE, NULL);  /* parent = zcat */
    exit(EXIT_FAILURE);
  }
}
