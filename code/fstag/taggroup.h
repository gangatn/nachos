#ifndef TAGGROUP_H
#define TAGGROUP_H

#include <list>
#include "fileheader.h"

struct file {
  char *name;
  FileHeader *header;
};

class TagGroup {
private:
  std::list<struct file*> files;
  std::list<Tag> taglist;

public:
  TagGroup(char *tags[]);
  int allocate();
  int addFile(char *name, FileHeader *fh);
};

#endif
