#ifndef TREE_H
#define TREE_H

typedef struct T_redir    *T_redir;
typedef struct T_sequence *T_sequence;
typedef struct T_pipeline *T_pipeline;
typedef struct T_command  *T_command;
typedef struct T_words    *T_words;
typedef struct T_word     *T_word;

// Define a new redir type which has space for 2 filenames and an op
struct T_redir {
  char *op1;			/* < or > */
  T_word word1;
  T_word word2;
};
struct T_sequence {
  T_pipeline pipeline;
  char *op;			/* ; or & */
  T_sequence sequence;
};

struct T_pipeline {
  T_command command;
  T_pipeline pipeline;
};

// Added a redir to match the grammar
struct T_command {
  T_words words;
  T_redir redir;
};

struct T_words {
  T_word word;
  T_words words;
};

struct T_word {
  char *s;
};

extern T_redir    new_redir();
extern T_sequence new_sequence();
extern T_pipeline new_pipeline();
extern T_command  new_command();
extern T_words    new_words();
extern T_word     new_word();

#endif
