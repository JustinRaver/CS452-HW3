/*
 * Author: Justin Raver
 * Class: CS452
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <readline/history.h>
#include <sys/types.h>
#include <sys/stat.h>
#include<sys/wait.h>
#include <fcntl.h>
#include "Command.h"
#include "error.h"

// Added input and output to store file descriptors for each command
typedef struct {
  // input fd
  int input;
  // output fd
  int output;
  char *file;
  char **argv;
} *CommandRep;

#define BIARGS CommandRep r, int *eof, Jobs jobs
#define BINAME(name) bi_##name
#define BIDEFN(name) static void BINAME(name) (BIARGS)
#define BIENTRY(name) {#name,BINAME(name)}

static char *owd=0;
static char *cwd=0;

static void builtin_args(CommandRep r, int n) {
  char **argv=r->argv;
  for (n++; *argv++; n--);
  if (n)
    ERROR("wrong number of arguments to builtin command"); // warn
}

/*
 * Added a wait for all child processes to exit
 */
BIDEFN(exit) {
  builtin_args(r,0);
  // wait for all child processes
  while ((wait(NULL)) > 0);
  *eof=1;
}

BIDEFN(pwd) {
  builtin_args(r,0);
  if (!cwd) cwd = getcwd(0,0);
  // print the current working directory to the commands fd
  dprintf(r->output, "%s\n",cwd);
}

/*
 * Added functionality to call cd - which does nothing if the cd command has not 
 * been called previously.
 * 
 * Added a full path so that cd - has a direct path to go to when called
 */
BIDEFN(cd) {
  builtin_args(r,1);
  if (strcmp(r->argv[1],"-")==0) {
    // if old working directory is set
    if(owd){
      // set temp working directory to current
      char *twd=cwd;
      // set current to old
      cwd=owd;
      // set old to temp
      owd=twd;
    }
  } else {
    // if old exists free
    if (owd) free(owd);
    // if cwd not set set it
    if(!cwd) cwd = getcwd(0,0);
    // set old working directory
    owd=cwd;

    // set current working directory
    cwd=strdup(r->argv[1]);
  }
  // throw error if failed
  if (cwd && chdir(cwd))
    ERROR("chdir() failed"); // warn
  // reset cwd to current working directory
  cwd = getcwd(0,0);
}

/* 
 * Used the history library and corresponding function history_list() to create
 * a list of the commands in the .history file and iterate through the commands
 * to print for the user
 */
BIDEFN(history) {
  builtin_args(r,0);

  register HIST_ENTRY **hist_list;
  // get list of history commands
  hist_list = history_list();
  
  if(hist_list){
    // iterate through commands with i as the command number
    for(int i=0; hist_list[i]; i++){
      // print to the commands output file descriptor
      dprintf(r->output, "%d %s\n", i+history_base, hist_list[i]->line);
    }
  }
}


static int builtin(BIARGS) {
  typedef struct {
    char *s;
    void (*f)(BIARGS);
  } Builtin;
  static const Builtin builtins[]={
    BIENTRY(exit),
    BIENTRY(pwd),
    BIENTRY(cd),
    BIENTRY(history),
    {0,0}
  };
  int i;
  for (i=0; builtins[i].s; i++)
    if (!strcmp(r->file,builtins[i].s)) {
      builtins[i].f(r,eof,jobs);
      // close the file descriptors if they were opened
      if(r->input != STDIN_FILENO) close(r->input);
      if(r->output != STDOUT_FILENO) close(r->output);
      return 1;
    }
  return 0;
}

static char **getargs(T_words words) {
  int n=0;
  T_words p=words;
  while (p) {
    p=p->words;
    n++;
  }
  char **argv=(char **)malloc(sizeof(char *)*(n+1));
  if (!argv)
    ERROR("malloc() failed");
  p=words;
  int i=0;
  while (p) {
    argv[i++]=strdup(p->word->s);
    p=p->words;
  }
  argv[i]=0;
  return argv;
}

/*
 * 
 * Added file descriptor handling which either sets default to those passed in
 * or if a redirect occurs will create new file descriptors and overwrite the 
 * default
 * 
 * Handles errors from opening the file descriptors
 */
extern Command newCommand(T_words words, T_redir redir, int input, int output) {
  CommandRep r=(CommandRep)malloc(sizeof(*r));
  if (!r)
    ERROR("malloc() failed");

  r->argv=getargs(words);
  r->file=r->argv[0];
  // set the input file descriptor to the given (handles default and pipe fd)
  r->input=input;
  // set the out put to given (handles default and pipe fd)
  r->output=output;

  // check if a redir occured in the command
  if (redir->op1 != NULL){
    // check redir type
    if (strchr(redir->op1, '<') != NULL){
      // create input file descriptor
      r->input = open(redir->word1->s, O_RDONLY);
      // check if a second redir occured to output
      if(redir->word2 != NULL){
        // create output file descriptor
        r->output= open(redir->word2->s, O_RDWR | O_CREAT | O_TRUNC, 0777);
      }
    }else{
      // create output file descriptor
      r->output= open(redir->word1->s, O_RDWR | O_CREAT | O_TRUNC, 0777);
    }

    // if opening the fds failed return an error
    if(r->output < 0 || r->input < 0){
        ERROR("Failed to open file");
        // free the failed command 
        freeCommand(r);
        exit(-222);
    }
  }
  return r;
}

/*
 * Changes file descriptors in the child for external commands and 
 * closes the corresponding descriptors in the child process
 * 
 * Proceeds to call execvp and throw error if failure has occured
 */
static void child(CommandRep r, int fg) {
  int eof=0;
  Jobs jobs=newJobs();
  if (builtin(r,&eof,jobs))
    return;
  
  // if command is external proceed to check file descriptors and change if not default
  if(r->output != STDOUT_FILENO){
    // Change descriptors
    if(dup2(r->output, STDOUT_FILENO) < 0) ERROR("Dup2 failed to execute");
    // close descriptors
    if(close(r->output) < 0) ERROR("Failed to close file descriptor");
  } 
  if(r->input != STDIN_FILENO){
    // Change descriptors
    if(dup2(r->input, STDIN_FILENO) < 0) ERROR("Dup2 failed to execute");
    // close descriptors
    if(close(r->input)< 0) ERROR("Failed to close file descriptor"); 
  }

  
  //Attempt to exec throw error on failure
  if(execvp(r->argv[0],r->argv) < 0) ERROR("Exec failed");

  ERROR("execvp() failed");
  exit(0);
}

/*
 * Added job functionality to track PID's for pipeline commands and
 * wait functionality to wait for single commands executed in the foreground
 */
extern void execCommand(Command command, Pipeline pipeline, Jobs jobs,
			int *jobbed, int *eof, int fg) {
  CommandRep r=command;

  if ((fg == 1) && builtin(r,eof,jobs))
    return;
  if (!*jobbed) { // 0 or 1
    *jobbed=1;
    addJobs(jobs,pipeline);
  }

  int pid=fork();
  if (pid==-1)
    ERROR("fork() failed");
  if (pid==0){
    child(r,fg);
    exit(0);
  }else{
    // close the file descriptors 
    closeFD(r);

    // if the command isnt a foreground command then wait for it to execute
    if(fg == 1){
      // wait for the process that was just added
      waitpid(pid, NULL, 0);
    }else{
      // get the pid of the child process and add it to the jobs for the pipeline
      addPipePID(pipeline,pid);
    }
  }
}

// Check command and closes open file descriptors if necessary
extern void closeFD(Command command){
  CommandRep r=command;

  if(r->input != STDIN_FILENO){
    if(close(r->input) < 0) ERROR("Failed to close pipe");
  }
  if(r->output != STDOUT_FILENO){
    if(close(r->output) < 0) ERROR("Failed to close pipe");
  }
}

extern void freeCommand(Command command) {
  CommandRep r=command;
  char **argv=r->argv;
  while (*argv)
    free(*argv++);
  free(r->argv);
  free(r);
}

extern void freestateCommand() {
  if (cwd) free(cwd);
  if (owd) free(owd);
}
