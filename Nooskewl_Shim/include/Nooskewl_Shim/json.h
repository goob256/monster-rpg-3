#ifndef NOO_JSON_H
#define NOO_JSON_H

#include "Nooskewl_Shim/main.h"

namespace noo {

namespace util {

class NOOSKEWL_SHIM_EXPORT JSON {
public:
	static std::string trim_quotes(std::string s);

	struct NOOSKEWL_SHIM_EXPORT Node {
		std::string key;
		std::string value; // can be [array]
		std::vector<Node *> children;

		Node *find(std::string key);

		std::string to_string(int indent = 0);
	};

	JSON(std::string filename, bool load_from_filesystem = false);
	JSON(SDL_RWops *file);
	~JSON();

	Node *get_root();

private:
	void read(std::string filename, bool load_from_filesystem = false);
	void read(SDL_RWops *file);
	int read_char(SDL_RWops *file);
	void unget(int c);
	void skip_whitespace(SDL_RWops *file);
	std::string read_token(SDL_RWops *file, bool is_string);
	std::string read_string(SDL_RWops *file);
	std::string read_value(SDL_RWops *file);
	void read_array(Node *node, SDL_RWops *file);
	void read_hash(Node *node, SDL_RWops *file);
	void parse_node(Node *node, SDL_RWops *file);
	void read(Node *node, SDL_RWops *file);
	void destroy(Node *node);

	Node *root;
	int ungot;
	int line;
};

} // End namespace util

} // End namespace noo

#endif // NOO_JSON_H
