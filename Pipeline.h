#ifndef PIPELINE_H
#define PIPELINE_H

typedef void *Pipeline;

#include "deq.h"
#include "Command.h"
#include "Jobs.h"

typedef struct {
  Deq processes;
  Deq pids;
  int fg;			// not "&"
} *PipelineRep;

extern Pipeline newPipeline(int fg);
extern void addPipeline(Pipeline pipeline, Command command);
extern void addPipePID(Pipeline pipeline, long process);
extern int sizePipePIDS(Pipeline pipeline);
extern Data getPipePID(Pipeline pipeline);
extern int sizePipeline(Pipeline pipeline);
extern void execPipeline(Pipeline pipeline, Jobs jobs, int *eof);
extern void freePipeline(Pipeline pipeline);

#endif
