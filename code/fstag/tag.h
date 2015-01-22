#include <list>
#define NAMELENGTH 32

class Tag {
	private:
		char name[NAMELENGTH];
		std::list<TagGroup> taggroups;
}
