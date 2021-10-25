#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "Interpreter.h"
#include "Sequence.h"
#include "Pipeline.h"
#include "Command.h"

static Command i_command(T_command t, int input, int output);
static void i_pipeline(T_pipeline t, Pipeline pipeline, int input);
static void i_sequence(T_sequence t, Sequence sequence);

static Command i_command(T_command t, int input, int output) {
  if (!t)
    return 0;
  Command command=0;
  if (t->words)
    command=newCommand(t->words, t->redir,input,output);
  return command;
}

static void i_pipeline(T_pipeline t, Pipeline pipeline, int input) {
  if (!t)
    return;
  
  if(t->pipeline != NULL){ 
    int fd[2];
    if(pipe(fd) < 0) perror("Failed to open pipe");

    // first command in pipe and no pipe before
    addPipeline(pipeline,i_command(t->command, input, fd[1]));
    // next command in pipe
    i_pipeline(t->pipeline,pipeline, fd[0]);
  }else{
    addPipeline(pipeline,i_command(t->command, input, STDOUT_FILENO));
  }
}

static void i_sequence(T_sequence t, Sequence sequence) {
  if (!t)
    return;
  
  // -1 for pipeline, 0 for &, 1 otherwise
  Pipeline pipeline=newPipeline((t->pipeline->pipeline != NULL) ? -1 : !(t->op != NULL && strchr(t->op, '&') != NULL));
  i_pipeline(t->pipeline,pipeline, STDIN_FILENO);
  addSequence(sequence,pipeline);
  i_sequence(t->sequence,sequence);
}

extern void interpretTree(Tree t, int *eof, Jobs jobs) {
  if (!t)
    return;
  Sequence sequence=newSequence();
  i_sequence(t,sequence);
  execSequence(sequence,jobs,eof);
}
