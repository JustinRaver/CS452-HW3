#ifndef COMMAND_H
#define COMMAND_H

typedef void *Command;

#include "Tree.h"
#include "Jobs.h"
#include "Sequence.h"

extern Command newCommand(T_words words, T_redir redir, int input, int output);

extern void execCommand(Command command, Pipeline pipeline, Jobs jobs,
			int *jobbed, int *eof, int fg);

extern void closeFD(Command command);
extern void freeCommand(Command command);
extern void freestateCommand();

#endif
