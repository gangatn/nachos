using namespace std;
class TagMap {
	private:
		list<Tag> tags;
	public:
		Tag* getTag(const char * name);
		void addTag(Tag *tag);
};
