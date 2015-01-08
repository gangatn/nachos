#include "syscall.h"

#define NBTHREAD 3

struct formula_t
{
  char **verse;
  unsigned int n;
};

void singformula(void *arg)
{
  unsigned int i;
  struct formula_t *formula = (struct formula_t*)arg;

  for (i = 0 ; i < formula->n ; ++i) {
    PutString(formula->verse[i]);
    PutChar('\n');
  }
  
  UserThreadExit();
}

int main()
{
  unsigned int i;

  struct formula_t formula;

  const char* verses[] = {
    "Hockety pockety wockety wack", 
    "Abracabra dabra nack", 
    "Shrink in size very small", 
    "We've got to save enough room for all", 
    "Higitus Figitus migitus mum",
    "Pres-ti-dig-i-ton-i-um!"
  };

  formula.verse = (char **)verses;
  formula.n = 6;
   
  for (i = 0 ; i < NBTHREAD ; ++i) {
    PutString("Creating thread...\n");
    if (UserThreadCreate(singformula, (void*)&formula) == -1)
      PutString("Too many threads :(\n");
    else
      PutString("Thread created\n");
  }

  return 0;
}
