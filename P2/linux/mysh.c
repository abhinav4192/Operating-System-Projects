#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <signal.h>

char error_message[30] = "An error has occurred\n";
#define MAXARG 15
#define MAXLENGTH 129
static int bkgrnd_count = 0;
int bkgrnd_max = 20;

void writeErrormsg(){
  if(!write(STDERR_FILENO, error_message, strlen(error_message)))
    printf("error in writing to error file\n");
}
void kill_background_proc(int *array){
  for(int i =0; i < bkgrnd_count; i++){
    int status;
    if(!waitpid(array[i], &status, WNOHANG)){//if not terminated already , send kill signal
      if(!kill(array[i], SIGKILL))
        continue;
      else
        writeErrormsg();
    }
    else
     continue;
  }
}
int main(int argc, char *argv[]){
    if(argc > 1){
      writeErrormsg();
      exit(1);
    }


    int i = 0;
    char *shellname = "mysh";


    char *input = (char *)malloc(sizeof(char)*MAXLENGTH);

    int pid;
    bool background_process = false;
    int bckgrnd_proc[20] = {0};
    int status;

    while(1)
    {
      ++i;
      char *outFileName = NULL;
      char *InFileName = NULL;
      char temp;
      int outfile = -1;
      int infile = -1;
      int outRed = 0;
      int inRed = 0;
      input[0] = '\0';
      input[MAXLENGTH-2] = '\0';
      int saved_stdout = -1;
      bool pipeline = false;
      bool background_process_current = false;


      int fd[2] = {-1,-1};
      // Reap lurking zombies before next prompt.
      if(background_process){
        for(int i = 0; i < bkgrnd_max ; i++){
          if(waitpid(bckgrnd_proc[i], &status, WNOHANG)){
            bckgrnd_proc[i] = 0;
          }
        }
      }
      bkgrnd_count = bkgrnd_count%bkgrnd_max;

      printf("%s (%d)> ",shellname, i);
      fflush(stdout);
      if(!fgets(input, MAXLENGTH, stdin))
        continue;


      if(strlen(input) == MAXLENGTH -1 ){
        if(strcmp(&input[MAXLENGTH -2], "\n")){
          while ((temp = getchar()) != '\n' && temp != EOF);
          writeErrormsg();
          goto error;
        }
      }

      char *str = input;
      char *delimiter = " \t\n"; //Ignore spaces, tabs and newlines.
      char *arguments[MAXARG] = {}, *saveptr;
      int k, argcount=0;

      for(k=0; k<MAXARG; k++,str = NULL)
      {
        arguments[k] = strtok_r(str, delimiter, &saveptr);
        if(arguments[k] == NULL){
          argcount = k;
          break;
        }
      }

      const char *program = arguments[0];
      const char *program2 = arguments[0]; // in case of pipe
      if(!program)
      {
        --i;// decrementing since we dont want to increase command history for blank command.
        continue;
      }

      //check for built commands.
      if(!strncmp(program, "exit",4))
      {
        if(argcount != 1){
          writeErrormsg();
          continue;
        }

        if(outfile){
          close(outfile);
        }

        if(background_process){
          kill_background_proc(bckgrnd_proc);
        }
        free(input);
        exit(0);
      }
      if(!strncmp(arguments[argcount-1],"&",strlen(arguments[argcount -1])))
      {
        if(argcount == 1){// Error check for no background program.
          writeErrormsg();
          goto error;
        }
        background_process = true;
        background_process_current = true;
        arguments[--argcount] = NULL;
      }
      //check for redirection operators and store input filename and Output file name.
      for(k=0; k < argcount ; k++)
      {

        if(!strncmp(arguments[k],"|",1))
        {
          program2 = arguments[k+1];
          if(!program2)
          {
            writeErrormsg();
            goto error;
          }
          arguments[k] = NULL;
          pipeline = true;
          if(pipe(fd) < 0)
          {
            printf("Pipe failed\n");
            writeErrormsg();
            goto error;
          }
          break;
        }

        if(!strncmp(arguments[k],"<",1))
        {

          inRed = 1;
          if(!arguments[k+1]){

            writeErrormsg();
            goto error; // No file name specified after redirection operator or more tahn one input file.
          }
          if(arguments[k+1])
          {
            InFileName = (char *)malloc(sizeof(char)*(strlen(arguments[k+1])+1));
            if(!InFileName)
            {
              writeErrormsg();
              goto error;
            }
            strncpy(InFileName,arguments[k+1],strlen(arguments[k+1]));
            InFileName[strlen(arguments[k+1])] ='\0';
            infile = open(InFileName, O_RDONLY);
            if(infile<0)
            {
                writeErrormsg();
            }
            arguments[k] = NULL;
            free(InFileName);
            continue;
          }

        }
        if(!strncmp(arguments[k],">",1))
        {
          outRed = 1;
          if(!arguments[k+1]){
            writeErrormsg();
            goto error; // No file name specified after redirection operator
          }

          if(arguments[k+1])
          {
            outFileName = (char *)malloc(sizeof(char)*(strlen(arguments[k+1])+1));
            if(!outFileName)
            {
              writeErrormsg();
              goto error;
            }
            strncpy(outFileName,arguments[k+1],strlen(arguments[k+1]));
            outFileName[strlen(arguments[k+1])] ='\0';
            outfile = open(outFileName, O_CREAT|O_WRONLY|O_TRUNC, S_IWUSR|S_IXUSR|S_IRUSR);
            if(outfile<0)
            {
                writeErrormsg();
                goto error;
            }
            free(outFileName);
            arguments[k] = NULL;
            continue;
          }
        }
      }
      if((inRed ^ outRed) && arguments[argcount -2]){ // Error check for input or output redirection file present in command.
        writeErrormsg();
        goto error;
      }
      // cd command handler
      if(!strncmp(program, "cd", 2))
      {
        const char *path;

        if(argcount > 2)
        {
          writeErrormsg();
          continue;
        }
        if(argcount ==1)
          path  = getenv("HOME");
        else
          path = arguments[argcount -1];

        int dirchange = chdir(path);
        if(dirchange == -1)
          writeErrormsg();
        continue;
      }

      //pwd command handler.
      else if(!strncmp(program, "pwd", 3))
      {
        if((outfile < 0 ) && !pipeline && argcount != 1)
        {
          writeErrormsg();
          continue;
        }

        //If redirection operator is there, write everything to outputfile instead of Stdout.
        // Had coded for redirection with pwd as well, though not needed in project specs.
        if(outfile>0)
        {
          saved_stdout  = dup(1);
          if(dup2(outfile, 1) < 0){
            writeErrormsg();
            goto error;
          }
        }
        if(pipeline){
          writeErrormsg();
          goto error;
        }

        char *buffer = (char *)malloc(100);
        printf("%s\n", getcwd(buffer, 100));

        free(buffer);
        if(outfile > 0)
        {
          dup2(saved_stdout,1);
          close(saved_stdout);
          close(outfile);
        }
        if(!pipeline)
          continue;
      }

      pid = fork();

      if(pid == 0)
      {
          if(outfile > 0)
          {
            if(dup2(outfile, 1) < 0){
              writeErrormsg();
            }
            close(outfile);
          }

          if(infile > 0)
          {
            if(dup2(infile, 0) < 0){
              writeErrormsg();
            }
            close(infile);
          }

        if(!pipeline){
          execvp(program, arguments);
        }
        else{
          close(0);
          if(dup(fd[0]) < 0){
            writeErrormsg();
          }
          close(fd[1]);
          close(fd[0]);
          execvp(program2, &arguments[k+1]);
        }
        writeErrormsg();
        exit(0);
      }
      else if(pid > 0)
      {

        int sec_child = 0;
        if(pipeline && strncmp(program,"pwd",3)){ //Pipeline with anything other than pwd.
          sec_child = fork();
          if(sec_child == 0)
          {
            saved_stdout  = dup(1);
            close(1);

            if(dup(fd[1]) < 0){
              printf("Dup failed\n");
              writeErrormsg();
            }
            close(fd[0]);
            close(fd[1]);
            execvp(program, arguments);
            printf("execvp inside sec child failed \n");
          }
        }
        if(background_process_current){
          while(bckgrnd_proc[bkgrnd_count]){// find an empty spot where background process has been reaped
            bkgrnd_count++;
          }
          bckgrnd_proc[bkgrnd_count++] = pid;
          waitpid(pid, &status, WNOHANG); // Reap it if already terminated.
        }
        else{
          if(pipeline){
            close(fd[1]);
            close(fd[0]);
          }
          waitpid(pid, &status, 0);
          if(sec_child)
            waitpid(sec_child, &status, 0);
        }
      }
error:
    continue;
    }

    return 0;
}
