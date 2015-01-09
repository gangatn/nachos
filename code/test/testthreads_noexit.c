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
  
}

int main()
{
  unsigned int i;
  struct formula_t formula;
  int tids[NBTHREAD];

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
    if ((tids[i] = UserThreadCreate(singformula, (void*)&formula)) == -1)
      PutString("Too many threads :(\n");
    else
      PutString("Thread created\n");
  }

  for (i = 0 ; i < NBTHREAD ; ++i) {
    if (tids[i] != -1) {
      PutString("Joining thread ");
      PutInt(tids[i]);
      PutString("...\n");
      UserThreadJoin(tids[i]);
      PutString("Thread ");
      PutInt(tids[i]);
      PutString(" joined.\n");
    }
  }

  return 0;
}
