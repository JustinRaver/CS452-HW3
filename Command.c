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

typedef struct {
  int input;
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

BIDEFN(exit) {
  builtin_args(r,0);
  wait(NULL);
  *eof=1;
}

BIDEFN(pwd) {
  builtin_args(r,0);
  if (!cwd)
    cwd=getcwd(0,0);
  
  dprintf(r->output, "%s\n",cwd);
  if(r->output != 1) close(r->output);
}

BIDEFN(cd) {
  builtin_args(r,1);
  if (strcmp(r->argv[1],"-")==0) {
    if(owd){
      char *twd=cwd;
      cwd=owd;
      owd=twd;
    }
  } else {
    if (owd) free(owd);
    if(!cwd) cwd = getcwd(0,0);
    owd=cwd;
    cwd=strdup(r->argv[1]);
  }
  // printf("%s\n",cwd);
  if (cwd && chdir(cwd))
    ERROR("chdir() failed"); // warn
  cwd = getcwd(0,0);
}

BIDEFN(history) {
  builtin_args(r,0);

  register HIST_ENTRY **hist_list;

  hist_list = history_list();
  
  if(hist_list){
    for(int i=0; hist_list[i]; i++){
      dprintf(r->output, "%d %s\n", i+history_base, hist_list[i]->line);
    }
  }

  if(r->output != 1) close(r->output);
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

extern Command newCommand(T_words words, T_redir redir, int input, int output) {
  CommandRep r=(CommandRep)malloc(sizeof(*r));
  if (!r)
    ERROR("malloc() failed");

  r->argv=getargs(words);
  r->file=r->argv[0];
  r->input=input;
  r->output=output;

  if (redir != NULL){
    if (strchr(redir->op1, '<') != NULL){
      r->input = open(redir->word1->s, O_RDONLY);

      if(redir->op2 != NULL){
        r->output= open(redir->word2->s, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
      }
    }else{
      r->output= open(redir->word1->s, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    }

    if(r->output < 0 || r->input < 0){
        perror("Failed to open file");
        exit(-222);
    }
  }
  return r;
}

static void child(CommandRep r, int fg) {
  int eof=0;
  Jobs jobs=newJobs();
  if (builtin(r,&eof,jobs))
    return;
 
  if(r->output != 1){
    dup2(r->output, STDOUT_FILENO);
    close(r->output);
  } 
  if(r->input != 0){
    dup2(r->input, STDIN_FILENO);
    close(r->input);
  }
  printf("Changed descriptors\n");

  printf("Made it before execvp\n");
  execvp(r->argv[0],r->argv);
  
  ERROR("execvp() failed");
  exit(0);
}

extern void execCommand(Command command, Pipeline pipeline, Jobs jobs,
			int *jobbed, int *eof, int fg) {
  CommandRep r=command;

  // printf("FG: %d\n", fg);
  if (fg && builtin(r,eof,jobs))
    return;
  if (!*jobbed) {
    *jobbed=1;
    addJobs(jobs,pipeline);
  }

  int pid=fork();
  if (pid==-1)
    ERROR("fork() failed");
  if (pid==0){
    printf("Went into a child funct\n");
    child(r,fg);
    printf("Returned from child\n");
  }
  // else{
  //   wait(NULL);
  //   printf("Child pid: %d\n", pid);
  // }
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
