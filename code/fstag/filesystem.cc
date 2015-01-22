#include "filesystem.h"
#include "synchdisk.h"

using namespace std;
int Filesystem::init(){

}

int Filesystem::create(char* name, char** tags) {
	// on récupére un secteur pour allouer un inode correspondant à un fichier
	char ** t;
	int fileheader, fileinode;
	Tag *tag;
	TagGroup *taggroup;

	fileheadersector = bitmap.Find();
	inodesector = bitmap.Find();

	tag = tagmap.getTag(tags[0]); // need to test if NULL

	if ((taggroup = tag.getGroup(tags)) == NULL)
		/* we need to create a group
		 * for this tag group
		 */
		taggroup = new TagGroup(tags);

	/* add the new file in the tag group */
	FileHeader fileheader = new FileHeader(taggroup);
	taggroup->addFile(name, fileheader);
}

int Filesystem::remove(char* name, char** tags) {

}
