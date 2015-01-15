#ifdef CHANGED

#include "copyright.h"
#include "system.h"
#include "synchconsole.h"

static void ReadAvail(int arg)
{
  ((SynchConsole*)arg)->readAvail->V();
}
static void WriteDone(int arg)
{
  ((SynchConsole*)arg)->writeDone->V();
}

SynchConsole::SynchConsole(char *readFile, char *writeFile)
{
  readAvail = new Semaphore("read avail", 0);
  writeDone = new Semaphore("write done", 0);

  console = new Console(readFile, writeFile, ReadAvail, WriteDone, (int)this);

  writesem = new Semaphore("writecharsem", 1);
  readsem = new Semaphore("readcharsem", 1);

  wrStrSem = new Semaphore("wrStrSem", 1);
  reStrSem = new Semaphore("reStrSem", 1);
}

SynchConsole::~SynchConsole()
{
  delete console;
  delete readAvail;
  delete writeDone;
  delete writesem;
  delete readsem;
}

void SynchConsole::SynchPutChar(const char ch)
{
  writesem->P();
  console->PutChar(ch);
  writeDone->P();
  writesem->V();
}

int SynchConsole::SynchGetChar()
{
  int c;
  readsem->P();
  readAvail->P();
  c = console->GetChar();
  readsem->V();
  return c;
}

void SynchConsole::SynchPutString(const char *s)
{
  unsigned int i = 0;
  wrStrSem->P();
  while (s[i] != '\0')
  {
    SynchPutChar(s[i]);
    ++i;
  }
  wrStrSem->V();
}

void SynchConsole::SynchGetString(char *s, int n)
{
  int i = 0;
  int ch;
  reStrSem->P();

  while (i < n - 1)
  {
    ch = SynchGetChar();
    if (ch == EOF)
      break;
    s[i] = (char)ch;
    ++i;
    if (ch == '\n')
      break;
  }
  s[i] = '\0';
  reStrSem->V();
}

void SynchConsole::SynchPutInt(int n)
{
  char dec[12]; // [sign,] 10 digits, \0
  snprintf(dec, 12, "%d", n);
  SynchPutString(dec);
}

void SynchConsole::SynchGetInt(int *n)
{
  char dec[12];
  SynchGetString(dec, 12);
  sscanf(dec, "%d", n);
}

#endif
