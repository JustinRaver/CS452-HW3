#include "Jobs.h"
#include "deq.h"
#include "error.h"
#include <sys/wait.h>

extern Jobs newJobs() {
  return deq_new();
}

extern void addJobs(Jobs jobs, Pipeline pipeline) {
  deq_tail_put(jobs,pipeline);
}

extern int sizeJobs(Jobs jobs) {
  return deq_len(jobs);
}

extern void freeJobs(Jobs jobs) { // never called
  deq_del(jobs,freePipeline);
}

extern void manageJobs(Jobs jobs){
  if(sizeJobs(jobs) == 0) return;
  /*
   * Go through all of the jobs in reverse order so that we can safely 
   * remove any jobs with 0 active processes
   */
  for(int i=sizeJobs(jobs)-1 ; i>=0 ; i--){
    // each pipeline job in jobs
    Pipeline pipeline = deq_head_ith(jobs,i);
    PipelineRep r = pipeline;

    int sizePIDS = sizePipePIDS(pipeline);
    if(sizePIDS == 0){
      freePipeline(pipeline);
    }
    /*
     * Go through all PIDS in reverse order so that we can safely delete any
     * that have exited
     */
    for(int j=sizePIDS-1;j>=0;j--){
      // get the ith PID
      pid_t pid = (long)deq_head_ith(r->pids,j);
      if(waitpid(pid, NULL, WNOHANG) > 0){
        printf("[%d] done\n",pid);
        deq_head_rem(r->pids,deq_head_ith(r->pids,j));
      } 
    }
  }
}
