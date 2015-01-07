#ifdef CHANGED

#include "copyright.h"
#include "system.h"
#include "synchconsole.h"

static void ReadAvail(int arg) { ((SynchConsole*)arg)->readAvail->V(); }
static void WriteDone(int arg) { ((SynchConsole*)arg)->writeDone->V(); }

SynchConsole::SynchConsole(char *readFile, char *writeFile)
{
  readAvail = new Semaphore("read avail", 0);
  writeDone = new Semaphore("write done", 0);

  console = new Console(readFile, writeFile, ReadAvail, WriteDone, (int)this);
}

SynchConsole::~SynchConsole()
{
  delete console;
  delete readAvail;
  delete writeDone;
}

void SynchConsole::SynchPutChar(const char ch)
{
  console->PutChar(ch);
  writeDone->P();
}

char SynchConsole::SynchGetChar()
{
  readAvail->P();
  return console->GetChar();
}

void SynchConsole::SynchPutString(const char *s)
{
  unsigned int i = 0;
  while (s[i] != '\0') {
    SynchPutChar(s[i]);
    ++i;
  } 
}

void SynchConsole::SynchGetString(char *s, int n)
{
  int i = 0;
  char ch;

  while (i < n-1) {
    ch = SynchGetChar();
    if (ch == EOF)
      break;
    s[i] = ch;
    ++i;
    if (ch == '\n')
      break;
  }
  s[i] = '\0';
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
