/*
 * Author: Justin Raver
 * Class: CS452
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "Pipeline.h"
#include "error.h"

/*
 * Added a Deq of process ID's to the pipeline so that they could be grouped
 * and then quickly iterated through when waiting or removing
 */
typedef struct {
  Deq processes;
  Deq pids;
  int fg;			// not "&"
} *PipelineRep;

extern Pipeline newPipeline(int fg) {
  PipelineRep r=(PipelineRep)malloc(sizeof(*r));
  if (!r)
    ERROR("malloc() failed");
  r->processes=deq_new();
  // Create a deq for the PIDS
  r->pids=deq_new();
  r->fg=fg;
  return r;
}

/*
 * Gets the deq of PIDs for a specific pipeline and returns it
 */
extern Deq getPIDS(Pipeline pipeline){
  PipelineRep r=pipeline;
  return r->pids;
}

/*
 * Adds a PID to a pipelines PIDS deq
 */
extern void addPipePID(Pipeline pipeline, long pid){
  PipelineRep r = pipeline;
  // add a process id to the process
  deq_head_put(r->pids, (Data)pid);
}

/*
 * Removes the PID at the head of the Pipelines deq
 * and returns it
 */
extern Data getPipePID(Pipeline pipeline){
  PipelineRep r = pipeline;
  return deq_head_get(r->pids);
}

/*
 * Returns the length of the PIDS deq
 */
extern int sizePipePIDS(Pipeline pipeline){
  PipelineRep r = pipeline;
  return deq_len(r->pids);
}

extern void addPipeline(Pipeline pipeline, Command command) {
  PipelineRep r=(PipelineRep)pipeline;
  deq_tail_put(r->processes,command);
}

extern int sizePipeline(Pipeline pipeline) {
  PipelineRep r=(PipelineRep)pipeline;
  return deq_len(r->processes);
}

static void execute(Pipeline pipeline, Jobs jobs, int *jobbed, int *eof) {
  PipelineRep r=(PipelineRep)pipeline;

  for (int i=0; i<sizePipeline(r) && !*eof; i++){
    execCommand(deq_head_ith(r->processes,i),pipeline,jobs,jobbed,eof,r->fg);
  }

  if((sizePipeline(r) > 1) && !(r->fg)){
    while(sizePipePIDS(pipeline) > 0){
      // Removes a PID from the DEQ and waits for it
      pid_t pid = (long) getPipePID(pipeline);
      waitpid(pid, NULL, 0);
    }
  }
}

extern void execPipeline(Pipeline pipeline, Jobs jobs, int *eof) {
  int jobbed=0;
  execute(pipeline,jobs,&jobbed,eof);

  if (!jobbed || (sizePipePIDS(pipeline) == 0)){ // frees the pipeline if all processes have exited
    // remove and free job from jobs FIXME: Is this necessary?
    deq_head_rem(jobs,pipeline);
    freePipeline(pipeline);	// for fg builtins, and such
  }
}

extern void freePipeline(Pipeline pipeline) {
  PipelineRep r=(PipelineRep)pipeline;
  deq_del(r->pids,0);
  deq_del(r->processes,freeCommand);
  free(r);
}
