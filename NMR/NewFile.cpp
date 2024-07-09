#include "NewFile.h"
#include "OS.h"
void BackEvent(float a) {
	if (MC)
		SC->CurrentPage = 0;
}


struct Editable {
	std::string Cache;
	std::string Data;
	bool Opened;
	
};
bool HoverState[3];
Editable name;
Editable Location;
Editable Details;
float OpenTime = 0.f;
void ContinueEvent(float a) {
	//continue
}

// ...




Hoverable Back(Vec2(1020, 830), Vec2(70, 40), BackEvent);
Hoverable Continue(Vec2(1100,830), Vec2(70, 40), ContinueEvent);



Hoverable PName(Vec2(60, 240), Vec2(700, 40), [](float a) {
	if(!name.Opened)
		HoverState[0] = true;
	});
Hoverable Loc(Vec2(60, 240 + 100), Vec2(700, 40), [](float a) {
	if (!Location.Opened)
		HoverState[1] = true;
	});
Hoverable Det(Vec2(60, 240 + 200), Vec2(700, 40), [](float a) {
	if (!Details.Opened)
		HoverState[2] = true;
	});
Hoverable Open(Vec2(60 + 700 + 20, 240 + 100), Vec2(50, 40), [](float a) {
	if (MC) {
		OS::SaveFolderDialog(Location.Data,name.Data);
	}
	});

void NFRender() {
	HoverState[0] = false;
	HoverState[1] = false;
	HoverState[2] = false;
	Continue.Update();
	Back.Update();
	PName.Update();
	Loc.Update();
	Det.Update();
	Open.Update();
	R::FilledRoundedRect(Back.p[0].x, Back.p[0].y, Back.p[1].x, Back.p[1].y, Col(0, 0, 0, 70 + 100 * Back.a), 7.f);
	R::RoundedRect(Back.p[0].x, Back.p[0].y, Back.p[1].x, Back.p[1].y, Col(255, 255, 255, 40 + 20 * Continue.a), 1.f, 7.f);
	R::DrawString(Back.p[0].x + Back.p[1].x * 0.5f, Back.p[0].y + Back.p[1].y * 0.5f, Col(255, 255, 255, 200 + 55 * Back.a), F::SH, R::centered_xy, "Back");
	R::FilledRoundedRect(Continue.p[0].x, Continue.p[0].y,Continue.p[1].x, Continue.p[1].y, Col(0, 0, 0, 70 + 100 * Continue.a), 7.f);
	R::RoundedRect(Continue.p[0].x, Continue.p[0].y, Continue.p[1].x, Continue.p[1].y, Col(255, 255, 255, 40 + 20 * Continue.a), 1.f, 7.f);
	R::DrawString(Continue.p[0].x + Continue.p[1].x * 0.5f, Continue.p[0].y + Continue.p[1].y * 0.5f, Col(255,255,255,200+55* Continue.a), F::SH, R::centered_xy, "Next");
	R::DrawString(30, 30, Col(255, 255, 255, 255), Fonts::H, 0, "Create a new NMR Project");


	

	if (MC) {

		name.Opened = HoverState[0];
		Location.Opened = HoverState[1];
		Details.Opened = HoverState[2];
		OpenTime = App->Curtime;
	}
	name.Cache = name.Data;
	Location.Cache = Location.Data;
	Details.Cache = Details.Data;

	//A!MC fix this must be added functionality for copy, paste
	//open folder too
	//and no unrecognized characters
	//also check for names so it apply with windows
	if (name.Opened) {
		PName.a = 1.f;


		if (((int)floorf((App->Curtime - OpenTime) / 110.f)) % 8 < 4)
			name.Cache += "|";

		auto Characters = App->GetInputCharacters();

		for (auto& Char : Characters) {
			if (Char == VK_RETURN) {
				name.Opened = false;
				break;
			}
			else if (Char == VK_ESCAPE) {
				name.Opened = false;
				break;
			}
			else if (Char == VK_BACK)
				name.Data = name.Data.substr(0, name.Data.size() - 1);
			else
				name.Data += Char;

		}
	}
	else if (Location.Opened) {
		Loc.a = 1.f;
		if (((int)floorf((App->Curtime - OpenTime) / 110.f)) % 8 < 4)
			Location.Cache += "|";

		auto Characters = App->GetInputCharacters();

		for (auto& Char : Characters) {
			if (Char == VK_RETURN) {
				Location.Opened = false;
				break;
			}
			else if (Char == VK_ESCAPE) {
				Location.Opened = false;
				break;
			}
			else if (Char == VK_BACK)
				Location.Data = Location.Data.substr(0, Location.Data.size() - 1);
			else
				Location.Data += Char;

		}
	}
	else if (Details.Opened) {
		Det.a = 1.f;
		if (((int)floorf((App->Curtime - OpenTime) / 110.f)) % 8 < 4)
			Details.Cache += "|";

		auto Characters = App->GetInputCharacters();

		for (auto& Char : Characters) {
			if (Char == VK_RETURN) {
				Details.Opened = false;
				break;
			}
			else if (Char == VK_ESCAPE) {
				Details.Opened = false;
				break;
			}
			else if (Char == VK_BACK)
				Details.Data = Details.Data.substr(0, Details.Data.size() - 1);
			else
				Details.Data += Char;

		}
	}



	R::DrawString(60, 200, Col(255, 255, 255, 255), Fonts::SH, 0, "Project name");
	R::FilledRoundedRect(PName.p[0].x, PName.p[0].y, PName.p[1].x, PName.p[1].y, Col(0, 0, 0, 50 + 30 * PName.a), 7.f);
	R::RoundedRect(PName.p[0].x, PName.p[0].y, PName.p[1].x, PName.p[1].y, Col(255, 255, 255, 40 + 20 * PName.a), 1.f, 7.f);
	R::DrawString(60, 300, Col(255, 255, 255, 255), Fonts::SH, 0, "Location");
	R::FilledRoundedRect(Loc.p[0].x, Loc.p[0].y, Loc.p[1].x, Loc.p[1].y, Col(0, 0, 0, 50 + 30 * Loc.a), 7.f);
	R::RoundedRect(Loc.p[0].x, Loc.p[0].y, Loc.p[1].x, Loc.p[1].y, Col(255, 255, 255, 40 + 20 * Loc.a), 1.f, 7.f);

	R::FilledRoundedRect(Open.p[0].x, Open.p[0].y, Open.p[1].x, Open.p[1].y, Col(0, 0, 0, 70 + 100 * Open.a), 7.f);
	R::RoundedRect(Open.p[0].x, Open.p[0].y, Open.p[1].x, Open.p[1].y, Col(255, 255, 255, 40 + 20 * Open.a), 1.f,7.f);
	R::DrawString(Open.p[0].x + Open.p[1].x*0.5f, Open.p[0].y + Open.p[1].y * 0.5f, Col(255, 255, 255, 255), Fonts::ICS, R::centered_xy, "G");

	R::DrawString(60, 400, Col(255, 255, 255, 255), Fonts::SH, 0, "Details");
	R::FilledRoundedRect(Det.p[0].x, Det.p[0].y, Det.p[1].x, Det.p[1].y, Col(0, 0, 0, 50 + 30 * Det.a), 7.f);
	R::RoundedRect(Det.p[0].x, Det.p[0].y, Det.p[1].x, Det.p[1].y, Col(255, 255, 255, 40 + 20 * Det.a), 1.f, 7.f);

	R::DrawString(PName.p[0].x + 10, PName.p[0].y + PName.p[1].y* 0.5f, Col(255, 255, 255, 255), Fonts::TX, R::centered_y, name.Cache.c_str());
	R::DrawString(Loc.p[0].x + 10, Loc.p[0].y + Loc.p[1].y * 0.5f, Col(255, 255, 255, 255), Fonts::TX, R::centered_y, Location.Cache.c_str());
	R::DrawString(Det.p[0].x + 10, Det.p[0].y + Det.p[1].y * 0.5f, Col(255, 255, 255, 255), Fonts::TX, R::centered_y, Details.Cache.c_str());
}

void NFStart() {
	Open.a = 0.f;
	Continue.a = 0.f;
	Back.a = 0.f;
	PName.a = 0.f;
	Loc.a = 0.f;
	Det.a = 0.f;
	name.Opened = false;
	Location.Opened = false;
	Details.Opened = false;
	//fix this must be changed late A!MC
	name.Data = "Project1";
	Location.Data = App->DefaultSearchFolder;
	Details.Data = "New Project";
	OpenTime = 0.f;
}