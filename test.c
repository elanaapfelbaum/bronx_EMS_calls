#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define FILE_NAME "EMSrawData.gz"
#define PLACE "BRONX"
#define ZIP_CODE "10463"


int main(){
  pid_t pid;                                 /* process id */
  int pipefd[2];                             /* pipe file descriptor - 1 = write, 0 = read */
  pipe(pipefd);                              /* creates a pipe with these 2 fds */
  pid = fork();                              /* branch to a child pipe */
  
  if (pid < 0){
    perror("fork");                          /* if there was an error with the fork */
    exit(EXIT_FAILURE);
  }

  if (pid > 0){                               /* reads from the child pipe */
    close(pipefd[1]);                         /* closes the read end of the pipe */
    dup2(pipefd[0], 0);                       /* replace stdin with input file */
    execlp("wc", "wc", "-l", NULL);           /* execute the grep operator = first child pipe */
    close(pipefd[0]);

    /* fork again to the next pipe */
    pid_t pid2 = fork();
    if (pid2 < 0){                            /* error */
      perror("fork");
      exit(EXIT_FAILURE);
    }
    if (pid2 == 0){
      close(pipefd[1]);                       /* close the unused read descriptor */
      dup2(pipefd[0], 0);                     /* replace stdin with input file */
      execlp("grep", "-i", PLACE, NULL);      /* execute the wc operator = child of the child */
      close(pipefd[0]);
    }
  }
  
  else{                                       /* parent */
    close(pipefd[0]);                         /* close unused write end of the pipe */
    dup2(pipefd[1], 1);                       /* replace stdout with output file  */
    execlp("zcat", "zcat", FILE_NAME, NULL);  /* parent = zcat */
    exit(EXIT_FAILURE);
  }
}
