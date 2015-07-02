#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "rdict.h"

#define RMACHINE "localhost"

int initw();
int nextin(char* cmd, char* word);
int insertw(const char* word);
int deletew(const char* word);
int lookupw(const char* word);

CLIENT *handle;

int main(int argc, char* argv[])
{
  char word[MAXWORD + 1];
  int wrdlen;
  char cmd;

  handle = clnt_create(RMACHINE, RDICTPROG, RDICTVERS, "tcp");
  if (0 == handle) {
    printf("Could not contact remote program\n");
    exit(1);
  }

  while (1) {
    wrdlen = nextin(&cmd, word);
    if (wrdlen < 0) 
      exit(1);

    switch (cmd) {
    case 'I':
      initw();
      printf("Dictionary initialized to empty\n");
      break;

    case 'i':
      insertw(word);
      printf("%s inserted\n", word);
      break;

    case 'd':
      if (deletew(word))
	printf("%s deleted\n", word);
      else
	printf("%s not deleted\n", word);
      break;

    case 'l':
      if (lookupw(word))
	printf("%s was found\n", word);
      else 
	printf("%s was not found\n", word);
      break;
    case 'q':
      printf("program quit\n");
      exit(0);

    default:
      printf("command %c invalid\n", cmd);
      break;
    }
  } /*end of which*/

  clnt_destroy(handle);
  return 0;
}

int nextin(char* cmd, char* word)
{
  int i, ch;

  ch = getc(stdin);
  while (isspace(ch)) 
    ch = getc(stdin);

  if (EOF == ch)
    return -1;
  *cmd = (char)ch;

  ch = getc(stdin);
  while (isspace(ch))
    ch = getc(stdin);

  if (EOF == ch)
    return -1;

  if ('\n' == ch)
    return 0;

  i = 0;
  while (!isspace(ch)) {
    if (++i > MAXWORD) {
      printf("error: word too long\n");
      exit(1);
    }
    *word++ = ch;
    ch = getc(stdin);
  }
  *word++ = 0;
  return i;
}
