/*
 * Author: Justin Raver
 * Class: CS452
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "Interpreter.h"
#include "Sequence.h"
#include "Pipeline.h"
#include "Command.h"
#include "error.h"

static Command i_command(T_command t, int input, int output);
static void i_pipeline(T_pipeline t, Pipeline pipeline, int input);
static void i_sequence(T_sequence t, Sequence sequence);

// Added an input and output that represent a commands file descriptors
static Command i_command(T_command t, int input, int output) {
  if (!t)
    return 0;
  Command command=0;
  if (t->words)
    command=newCommand(t->words, t->redir,input,output);
  return command;
}

/*
 * if the pipeline contains more than 1 command create a pipe() and set the 
 * input and output file descriptors of the corresponding commands
 */
static void i_pipeline(T_pipeline t, Pipeline pipeline, int input) {
  if (!t)
    return;
  // if the pipeline has more than one command
  if(t->pipeline != NULL){ 
    // file descriptors
    int fd[2];
    // setup the file descriptors and check for an error
    if(pipe(fd) < 0) ERROR("Failed to open pipe");

    /* 
     * set the file descriptors for the command to the input of the previous call
     * and output to fd[1]
     */
    addPipeline(pipeline,i_command(t->command, input, fd[1]));
    // next command in pipe set input to fd[0]
    i_pipeline(t->pipeline,pipeline, fd[0]);
  }else{
    // Either only command in pipe or last. Set input to passed and output always to STDOUT
    addPipeline(pipeline,i_command(t->command, input, STDOUT_FILENO));
  }
}

static void i_sequence(T_sequence t, Sequence sequence) {
  if (!t)
    return;
  
  // Creates a new pipeline and sets fg = 0 for & or pipeline, 1 otherwise
  Pipeline pipeline=newPipeline(!(t->pipeline->pipeline != NULL) && !(t->op != NULL && strchr(t->op, '&') != NULL));
  // Call i_pipeline with input as the default STDIN
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