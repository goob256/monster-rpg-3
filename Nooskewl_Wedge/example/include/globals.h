#ifndef GLOBALS_H
#define GLOBALS_H

#include <Nooskewl_Wedge/globals.h>

class Globals : public wedge::Globals
{
public:
	Globals();
	virtual ~Globals();

	void do_dialogue(std::string tag, std::string text, wedge::Dialogue_Type type, wedge::Dialogue_Position position, wedge::Step *monitor);
	bool add_title_gui();
	
	class Instance : public wedge::Globals::Instance
	{
	public:
		Instance(util::JSON::Node *root);
		virtual ~Instance();
	};

	audio::Sound *melee;

	bool started;
};

#endif // GLOBALS_H
