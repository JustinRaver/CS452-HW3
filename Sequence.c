/*
 * Author: Justin Raver
 * Class: CS452
 */
#include <sys/wait.h>
#include "Sequence.h"
#include "deq.h"
#include "error.h"

extern Sequence newSequence() {
  return deq_new();
}

extern void addSequence(Sequence sequence, Pipeline pipeline) {
  deq_tail_put(sequence,pipeline);
}

extern void freeSequence(Sequence sequence) {
  deq_del(sequence,freePipeline);
}

// Added job management after sequence exits to reduce job space consumption
extern void execSequence(Sequence sequence, Jobs jobs, int *eof) {
  while (deq_len(sequence) && !*eof)
    execPipeline(deq_head_get(sequence),jobs,eof);
  // manage all the PIDS and jobs in the job handler
  manageJobs(jobs);

  freeSequence(sequence);
}
