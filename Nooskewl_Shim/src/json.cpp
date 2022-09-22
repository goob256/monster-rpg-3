#ifdef TVOS
#include "Nooskewl_Shim/ios.h"
#endif

#include "Nooskewl_Shim/error.h"
#include "Nooskewl_Shim/json.h"
#include "Nooskewl_Shim/util.h"

using namespace noo;

namespace noo {

namespace util {

JSON::Node *JSON::Node::find(std::string key)
{
	for (size_t i = 0; i < children.size(); i++) {
		if (children[i]->key == key) {
			return children[i];
		}
	}

	return 0;
}

std::string JSON::Node::to_string(int indent)
{
	std::string s(indent, '\t');

	if (key.length() >= 1 && key[0] != '[') {
		s += string_printf("%s: ", key.c_str());
	}

	if (value == "[hash]" || value == "[array]") {
		if (value == "[hash]") {
			s += "{\n";
		}
		else {
			s += "[\n";
		}
		for (size_t i = 0; i < children.size(); i++) {
			s += children[i]->to_string(indent+1);
			if (i < children.size()-1) {
				s += ",";
			}
			s += "\n";
		}
		s += std::string(indent, '\t');
		if (value == "[hash]") {
			s += "}";
		}
		else {
			s += "]";
		}
	}
	else {
		s += value;
	}

	return s;
}

std::string JSON::trim_quotes(std::string s)
{
	if (s[0] == '"') {
		s = s.substr(1);
	}
	if (s[s.length()-1] == '"') {
		s = s.substr(0, s.length()-1);
	}
	return s;
}

JSON::JSON(std::string filename, bool load_from_filesystem) :
	root(0),
	ungot(-1),
	line(1)
{
	read(filename, load_from_filesystem);
}

JSON::JSON(SDL_RWops *file) :
	root(0),
	ungot(-1),
	line(1)
{
	read(file);
}

JSON::~JSON()
{
	destroy(root);
}

void JSON::destroy(Node *node)
{
	for (size_t i = 0; i < node->children.size(); i++) {
		destroy(node->children[i]);
	}
	delete node;
}

void JSON::read(std::string filename, bool load_from_filesystem)
{
	int sz;
	char *bytes;

#ifdef TVOS
	std::string s;
#endif
    
	if (load_from_filesystem) {
#ifdef TVOS
		if (tvos_read_file(filename, s) == false) {
			throw util::FileNotFoundError(filename + " could not be loaded from NSUserDefaults");
		}
		sz = (int)s.length();
		bytes = (char *)s.c_str();
#else
		bytes = slurp_file_from_filesystem(filename, &sz);
#endif
	}
	else {
		bytes = slurp_file(filename, &sz);
	}

	SDL_RWops *file = SDL_RWFromMem(bytes, sz);

	read(file);

	SDL_RWclose(file);

#ifndef TVOS
	delete[] bytes;
#endif
}

void JSON::read(SDL_RWops *file)
{
	skip_whitespace(file);

	int c = read_char(file);

	if (c == EOF) {
		throw util::LoadError("Premature end of file");
	}
	else if (c != '{' && c != '[') {
		throw util::LoadError(util::string_printf("Unexpected symbol on line %d", line));
	}

	root = new Node;
	root->key = "[root]";

	if (c == '{') {
		root->value = "[hash]";
		read_hash(root, file);
	}
	else {
		root->value = "[array]";
		read_array(root, file);
	}
}

JSON::Node *JSON::get_root()
{
	return root;
}

int JSON::read_char(SDL_RWops *file)
{
	if (ungot != -1) {
		int ret = ungot;
		ungot = -1;
		return ret;
	}

	int c = SDL_fgetc(file);
	if (c == '\n') {
		line++;
	}
	return c;
}

void JSON::unget(int c)
{
	ungot = c;
}

void JSON::skip_whitespace(SDL_RWops *file)
{
	while (true) {
		int c = read_char(file);
		if (c == EOF) {
			return;
		}
		else if (!isspace(c)) {
			unget(c);
			return;
		}
	}

	assert(0 && "Error skipping whitespace");
}

std::string JSON::read_token(SDL_RWops *file, bool is_string)
{
	std::string token;
	int prev = -1;

	while (true) {
		int c = read_char(file);

		if (c == EOF) {
			throw util::LoadError("Premature end of file reading token");
		}

		if (is_string) {
			if (c == '"') {
				if (prev == '\\') {
					token = token.substr(0, token.length()-1);
					token += "\"";
				}
				else {
					break;
				}
			}
		}
		else {
			if (c == ',' || c == '}' || c == ']') {
				unget(c);
				break;
			}
		}

		if (!isspace(c) || is_string) {
			char s[2];
			s[0] = c;
			s[1] = 0;

			token += s;
		}
	}

	return token;
}

std::string JSON::read_string(SDL_RWops *file)
{
	return "\"" + read_token(file, true) + "\"";
}

std::string JSON::read_value(SDL_RWops *file)
{
	return read_token(file, false);
}

void JSON::parse_node(Node *node, SDL_RWops *file)
{
	skip_whitespace(file);

	int c = read_char(file);

	if (c == EOF) {
		throw util::LoadError("Premature end of file");
	}
	else if (c == '{') {
		node->value = "[hash]";
	}
	else if (c == '[') {
		node->value = "[array]";
	}
	else if (c == '"') {
		node->value = read_string(file);
	}
	else {
		unget(c);
		node->value = read_value(file);
	}

	if (node->value == "[hash]") {
		read_hash(node, file);
	}
	else if (node->value == "[array]") {
		read_array(node, file);
	}
}

void JSON::read_array(Node *node, SDL_RWops *file)
{
	for (int count = 0;;) {
		skip_whitespace(file);

		int c = read_char(file);

		if (c == EOF) {
			throw util::LoadError("Premature end of file reading array");
		}
		else if (c == ']') {
			return;
		}
		else if (c == ',') {
			continue;
		}

		unget(c);

		Node *json = new Node;
		json->key = "[" + itos(count) + "]";

		parse_node(json, file);
		
		node->children.push_back(json);

		count++;
	}

	assert(0 && "Unknown error reading array\n");
}

void JSON::read_hash(Node *node, SDL_RWops *file)
{
	while (true) {
		skip_whitespace(file);

		int c = read_char(file);

		if (c == EOF) {
			throw util::LoadError("Premature end of file");
		}
		else if (c == '}') {
			return;
		}
		else if (c == ',') {
			continue;
		}
		else if (c != '"') {
			throw util::LoadError(util::string_printf("Parse error on line %d, expected } or \"", line));
		}

		Node *json = new Node;

		json->key = read_string(file);

		skip_whitespace(file);

		if (read_char(file) != ':') {
			throw util::LoadError(util::string_printf(": expected on line %d", line));
		}

		parse_node(json, file);

		node->children.push_back(json);
	}
}

} // End namespace util

} // End namespace noo
