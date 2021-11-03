# Homework #3: Execution Integrity

* Author: Justin Raver
* Class: CS452 
* Semester: Fall 2021

## Overview

This project builds off of the given skeleton to create a basic shell. This shell implements
common functionality like IO redirection to and from commands and the ability to run built ins
"cd", "history", "pwd" and finally "exit". It can also run non-built such as ls, echo .. etc. 
Along with these functionalities I have added the ability to execute pipelines ex: "pwd | cat",
sequences ex: "cd .. ; pwd" and background commands ex: "pwd & cd .. ; pwd".

These abilities combine to create a simple, but robust interpretation of a shell.

## Implementation

* Redirection 
    Following the grammar I created functionality to parse a redirect and add it to its corresponding 
    command. I found the simplest way to change a commands file descriptors was by checking for 
    redirection when creating a new command and creating file descriptors accordingly. This allows me 
    to contain all of the file descriptor creation and changing in one place.
  
* Background Commands
    I implemented background command with the functionality that was contained in the skeleton by 
    changing the value of FG which I believe stands for forground. This allowed me to check whether
    the parent process should wait for the corresponding child process or proceed. In the case that 
    the parent process proceeds I add the child to the Jobs Deq and incrementaly check if the process
    has exited. If it has I cleanup the child process to prevent zombie processes. Finally I added some
    job management to reduce the number of Jobs in the Job Deq in a long lived session and to reduce
    memory leaks.

* Pipelines
    I chose to implement pipelines starting in the interpreter. With the skeleton we could tell in
    i_pipeline that a pipeline would have 1 or more commands. From there I created file descriptors 
    for the pipes and passed the to the calls to i_command which would then create a new command using
    those descriptors. Although I am not sure this is optimal I found it to be the best way for me 
    to set the pipeline file descriptors.

    When executing the pipelines I setup the parent process to iterate through the pipelines commands
    creating a child process for each and then waiting for all of the children to finish. I found the 
    most optimal place to do the waiting to be in pipeline.c where the calls to exec  command occur.
* Jobs
    The jobs functionality was largely implemented, however I utilized the jobs to also manage the 
    PIDS by modifying the pipeline struct. This allowed me to easily iterate through all of the PID's
    in a pipeline and wait for them. The PID's could have also been added to command, but the was 
    simpler for me to implement. 

    To reduce memory leaks I also had to implement some job management which would free the pipelines
    that had no active PIDS. This means for longer sessions in the shell the Jobs Deq wont become 
    extremely large. 

## Compiling and Using

From directory with all c files, Mastermakefile and GNUmakefile:

build and run: make run

run test suite: make test

valgrind report with suppressions: make ; ./vg

## Testing

When testing I attempted to be as thurough as possible. Some of the tests may pass or fail due to the
synchronous nature of the program and must be inspected manually to check results. The history tests
must be inspected manually to see the result are correct.

I attempted to test all aspects of the implementation and combinations of the functionality where
possible. I Tested sequences, pipelines, IO redirection, the built in commands, sequences that contain 
pipelines, sequences that contain redirection, pipelines that contian redirection and so on.
## Valgrind Report after running (pwd ; cd .. ; cd - ; pwd | cat | cat | cat ; echo "hi" ; exit)
* I use the ./vg script, but the suppressions dont catch these
  
==2770353== 
==2770353== HEAP SUMMARY:
==2770353==     in use at exit: 139,342 bytes in 196 blocks
==2770353==   total heap usage: 1,129 allocs, 933 frees, 396,339 bytes allocated
==2770353== 
==2770353== 47 bytes in 1 blocks are still reachable in loss record 12 of 46
==2770353==    at 0x4C37419: realloc (vg_replace_malloc.c:834)
==2770353==    by 0x52CD286: _nc_doalloc (in /usr/lib64/libtinfo.so.6.1)
==2770353==    by 0x52D641B: _nc_read_termtype (in /usr/lib64/libtinfo.so.6.1)
==2770353==    by 0x52D68EC: ??? (in /usr/lib64/libtinfo.so.6.1)
==2770353==    by 0x52D6CD6: ??? (in /usr/lib64/libtinfo.so.6.1)
==2770353==    by 0x52D6E04: _nc_read_entry2 (in /usr/lib64/libtinfo.so.6.1)
==2770353==    by 0x52CF9D5: _nc_setup_tinfo (in /usr/lib64/libtinfo.so.6.1)
==2770353==    by 0x52CFD52: _nc_setupterm (in /usr/lib64/libtinfo.so.6.1)
==2770353==    by 0x52D0256: tgetent_sp (in /usr/lib64/libtinfo.so.6.1)
==2770353==    by 0x4E7421B: _rl_init_terminal_io (in /usr/lib64/libreadline.so.7.0)
==2770353==    by 0x4E5B4F8: rl_initialize (in /usr/lib64/libreadline.so.7.0)
==2770353==    by 0x4E5B73A: readline (in /usr/lib64/libreadline.so.7.0)
==2770353== 
==2770353== 160 bytes in 1 blocks are still reachable in loss record 17 of 46
==2770353==    at 0x4C37419: realloc (vg_replace_malloc.c:834)
==2770353==    by 0x52CD286: _nc_doalloc (in /usr/lib64/libtinfo.so.6.1)
==2770353==    by 0x52D6457: _nc_read_termtype (in /usr/lib64/libtinfo.so.6.1)
==2770353==    by 0x52D68EC: ??? (in /usr/lib64/libtinfo.so.6.1)
==2770353==    by 0x52D6CD6: ??? (in /usr/lib64/libtinfo.so.6.1)
==2770353==    by 0x52D6E04: _nc_read_entry2 (in /usr/lib64/libtinfo.so.6.1)
==2770353==    by 0x52CF9D5: _nc_setup_tinfo (in /usr/lib64/libtinfo.so.6.1)
==2770353==    by 0x52CFD52: _nc_setupterm (in /usr/lib64/libtinfo.so.6.1)
==2770353==    by 0x52D0256: tgetent_sp (in /usr/lib64/libtinfo.so.6.1)
==2770353==    by 0x4E7421B: _rl_init_terminal_io (in /usr/lib64/libreadline.so.7.0)
==2770353==    by 0x4E5B4F8: rl_initialize (in /usr/lib64/libreadline.so.7.0)
==2770353==    by 0x4E5B73A: readline (in /usr/lib64/libreadline.so.7.0)
==2770353== 
==2770353== 3,896 bytes in 1 blocks are still reachable in loss record 33 of 46
==2770353==    at 0x4C37419: realloc (vg_replace_malloc.c:834)
==2770353==    by 0x52CD286: _nc_doalloc (in /usr/lib64/libtinfo.so.6.1)
==2770353==    by 0x52D6475: _nc_read_termtype (in /usr/lib64/libtinfo.so.6.1)
==2770353==    by 0x52D68EC: ??? (in /usr/lib64/libtinfo.so.6.1)
==2770353==    by 0x52D6CD6: ??? (in /usr/lib64/libtinfo.so.6.1)
==2770353==    by 0x52D6E04: _nc_read_entry2 (in /usr/lib64/libtinfo.so.6.1)
==2770353==    by 0x52CF9D5: _nc_setup_tinfo (in /usr/lib64/libtinfo.so.6.1)
==2770353==    by 0x52CFD52: _nc_setupterm (in /usr/lib64/libtinfo.so.6.1)
==2770353==    by 0x52D0256: tgetent_sp (in /usr/lib64/libtinfo.so.6.1)
==2770353==    by 0x4E7421B: _rl_init_terminal_io (in /usr/lib64/libreadline.so.7.0)
==2770353==    by 0x4E5B4F8: rl_initialize (in /usr/lib64/libreadline.so.7.0)
==2770353==    by 0x4E5B73A: readline (in /usr/lib64/libreadline.so.7.0)
==2770353== 
==2770353== LEAK SUMMARY:
==2770353==    definitely lost: 0 bytes in 0 blocks
==2770353==    indirectly lost: 0 bytes in 0 blocks
==2770353==      possibly lost: 0 bytes in 0 blocks
==2770353==    still reachable: 4,103 bytes in 3 blocks
==2770353==         suppressed: 135,239 bytes in 193 blocks
==2770353== 
==2770353== For lists of detected and suppressed errors, rerun with: -s
==2770353== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)