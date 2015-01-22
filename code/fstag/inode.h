#define NBDIRECTSECTOR 2

class Inode {
 private:
  int direct[NBDIRECTSECTOR];
 public:
  Inode();
  
};
