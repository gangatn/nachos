#ifndef USERTHREAD_H
#define USERTHREAD_H

extern int do_UserThreadCreate(int f, int arg);
extern void do_UserThreadExit();
extern void do_UserThreadJoin(int tid);

#endif
