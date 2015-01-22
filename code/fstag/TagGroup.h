#include <list>

struct file {
	char *filename;
	FileHeader *fileheader;
};

class TagGroup {
	private:
		std::list<struct file*> files;
		std::list<Tag> tags;
	public:
		TagGroup();
		addTag(Tag *tag);
		addFile(FileHeader *fh);
};
