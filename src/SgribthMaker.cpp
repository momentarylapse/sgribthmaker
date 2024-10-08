#include "SgribthMaker.h"
#include "SgribthMakerWindow.h"
#include "HighlightScheme.h"
#include "parser/BaseParser.h"
#include "lib/kaba/kaba.h"
#include "lib/hui/config.h"



string AppTitle = "SgribthMaker";
string AppVersion = "0.5.1.0";

//#define ALLOW_LOGGING			true
#define ALLOW_LOGGING			false

SgribthMaker* sgribthmaker;


SgribthMaker::SgribthMaker() :
	hui::Application("sgribthmaker", "English", hui::Flags::SILENT)
{
	set_property("name", AppTitle);
	set_property("version", AppVersion);
	set_property("comment", _("Text editor and kaba compiler"));
	set_property("website", "http://michi.is-a-geek.org/michisoft");
	set_property("copyright", "© 2006-2024 by MichiSoft TM");
	set_property("author", "Michael Ankele <michi@lupina.de>");

	kaba::init();

	win = nullptr;
	sgribthmaker = this;
}


hui::AppStatus SgribthMaker::on_startup(const Array<string> &arg) {

	InitParser();
	HighlightScheme::default_scheme = HighlightScheme::get(hui::config.get_str("HighlightScheme", "default"));

	win = new SgribthMakerWindow();

	if (arg.num > 1) {
		for (int i=1; i<arg.num; i++)
			win->load_from_file(Path(arg[i]).absolute().canonical());
	} else {
		win->create_new_document();
	}
	return hui::AppStatus::RUN;
}

HUI_EXECUTE(SgribthMaker)
