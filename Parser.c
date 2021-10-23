#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Parser.h"
#include "Tree.h"
#include "Scanner.h"
#include "error.h"

static Scanner scan;

#undef ERROR
#define ERROR(s) ERRORLOC(__FILE__,__LINE__,"error","%s (pos: %d)",s,posScanner(scan))

static char *next()       { return nextScanner(scan); }
static char *curr()       { return currScanner(scan); }
static int   cmp(char *s) { return cmpScanner(scan,s); }
static int   eat(char *s) { return eatScanner(scan,s); }

static T_word p_word();
static T_words p_words();
static T_command p_command();
static T_pipeline p_pipeline();
static T_sequence p_sequence();
static T_redir p_redir();


static T_word p_word() {
  char *s=curr();

  if (!s)
    return 0;
  T_word word=new_word();
  word->s=strdup(s);
  next();
  return word;
}

static T_words p_words() {
  T_word word=p_word();
  if (!word)
    return 0;
  T_words words=new_words();
  words->word=word;
  if (cmp("|") || cmp("&") || cmp(";") || cmp("<") || cmp(">"))
    return words;
  words->words=p_words();
  return words;
}

static T_command p_command() {
  T_words words=0;
  words=p_words();
  if (!words)
    return 0;
  T_command command=new_command();
  T_redir redir=p_redir();

  command->redir=redir;
  command->words=words;
  return command;
}

static T_pipeline p_pipeline() {
  T_command command=p_command();
  if (!command)
    return 0;
  T_pipeline pipeline=new_pipeline();
  pipeline->command=command;
  if (eat("|"))
    pipeline->pipeline=p_pipeline();
  return pipeline;
}

static T_sequence p_sequence() {
  T_pipeline pipeline=p_pipeline();
  if (!pipeline)
    return 0;
  T_sequence sequence=new_sequence();
  sequence->pipeline=pipeline;
  if (eat("&")) {
    sequence->op="&";
    sequence->sequence=p_sequence();
  }
  if (eat(";")) {
    sequence->op=";cd ";
    sequence->sequence=p_sequence();
  }
  return sequence;
}

static T_redir p_redir(){
  printf("Doing redir \n");
  T_redir redir=new_redir();

  int ate = 0;

  if(eat("<")){
    redir->op1="<";
    redir->word1=p_word();
    ate = 1;
  }
  if(eat(">")){
    if(ate){
      redir->op2 = ">";
      redir->word2=p_word();
      ate = 0;
    }else{
      printf("Made it here!\n");
      redir->op1= ">";
      redir->word1=p_word();
    }
  }
  printf("Made it past conditionals\n");
  if(strchr(redir->op1, '<') != NULL || strchr(redir->op1, '>') != NULL){
    if(ate){
      redir->op2 = NULL;
      redir->word2=NULL;
    } 
    printf("Returning something\n");
    return redir;
  }

  return NULL;
}

extern Tree parseTree(char *s) {
  scan=newScanner(s);
  Tree tree=p_sequence();
  if (curr())
    ERROR("extra characters at end of input");
  freeScanner(scan);
  return tree;
}

static void f_word(T_word t);
static void f_words(T_words t);
static void f_command(T_command t);
static void f_pipeline(T_pipeline t);
static void f_sequence(T_sequence t);
static void f_redir(T_redir t);

static void f_word(T_word t) {
  if (!t)
    return;
  if (t->s)
    free(t->s);
  free(t);
}

static void f_words(T_words t) {
  if (!t)
    return;
  f_word(t->word);
  f_words(t->words);
  free(t);
}

static void f_command(T_command t) {
  if (!t)
    return;
  f_words(t->words);
  f_redir(t->redir);
  free(t);
}

static void f_pipeline(T_pipeline t) {
  if (!t)
    return;
  f_command(t->command);
  f_pipeline(t->pipeline);
  free(t);
}

static void f_sequence(T_sequence t) {
  if (!t)
    return;
  f_pipeline(t->pipeline);
  f_sequence(t->sequence);
  free(t);
}

static void f_redir(T_redir t){
  if (!t)
    return;
  f_word(t->word1);
  f_word(t->word2);
  free(t);
}

extern void freeTree(Tree t) {
  f_sequence(t);
}
