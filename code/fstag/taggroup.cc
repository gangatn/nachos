#include "taggroup.h"
#include "string.h"

using namespace std;

TagGroup::TagGroup(char *tags[])
{
  char **t;
  Tag *tag;

  /* for each tag, verify if it exists already
   * if not create it
   * in both cases, add it to the taglist
   */
  t = tags;
  while (t != NULL) {
    tag = TagManager.getTag(*t);
    if (tag == NULL)
      tag = TagManager.addTag(*t);
    taglist.push_back(tag);
  }
}

int TagGroup::allocate()
{
  int sector;
  Inode 

  sector = SectorManager.get();
  
  
}

int TagGroup::addFile(char* name, FileHeader* fh)
{
  list<struct file*>::iterator it;
  struct file *f;

  for (it = files.begin() ; it != files.end() ++it)
    if (strcmp(*it, name) == 0)
      /* a file with this name is already present */
      return -1;
  
  f->name = name;
  f->header = fh;

  files.push_back(f);
  return 1;
}
