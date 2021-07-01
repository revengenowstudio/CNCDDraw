//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <IniFiles.hpp>
#include <StrUtils.hpp>
#include <IOUtils.hpp>
#include "ConfigFormUnit.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TConfigForm *ConfigForm;
bool Initialized;

/* Save previous settings so we don't override custom settings */
int Maxfps;
int Savesettings;
int Hook;
int Minfps;

//---------------------------------------------------------------------------
__fastcall TConfigForm::TConfigForm(TComponent* Owner)
	: TForm(Owner)
{
}

void __fastcall TConfigForm::DisplayBtnClick(TObject *Sender)
{
	DisplayPnl->Visible = true;
	AdvDisplayPnl->Visible = false;
	CompatibilityPnl->Visible = false;
}

void __fastcall TConfigForm::AdvDisplayBtnClick(TObject *Sender)
{
	AdvDisplayPnl->Visible = true;
	DisplayPnl->Visible = false;
	CompatibilityPnl->Visible = false;
}

void __fastcall TConfigForm::CompatibilityBtnClick(TObject *Sender)
{
	CompatibilityPnl->Visible = true;
	AdvDisplayPnl->Visible = false;
	DisplayPnl->Visible = false;
}

void __fastcall TConfigForm::FormCreate(TObject *Sender)
{
	auto *ini = new TIniFile(".\\ddraw.ini");

	/* Display Settings */

	bool windowed = GetBool(ini, "windowed", false);
	bool fullscreen = GetBool(ini, "fullscreen", false);

	if (windowed && fullscreen) {
		PresentationCbx->ItemIndex = 1;
	}
	else if (windowed) {
		PresentationCbx->ItemIndex = 2;
	}
	else {
		PresentationCbx->ItemIndex = 0;
	}

	MaintasChk->State = GetBool(ini, "maintas", false) ? tssOn : tssOff;
	VsyncChk->State = GetBool(ini, "vsync", false) ? tssOn : tssOff;
	AdjmouseChk->State = GetBool(ini, "adjmouse", false) ? tssOn : tssOff;
	DevmodeChk->State = GetBool(ini, "devmode", false) ? tssOff : tssOn;

	/* Advanced Display Settings */

	auto renderer = LowerCase(ini->ReadString("ddraw", "renderer", "auto"));

	if (StartsStr("d", renderer)) {
		RendererCbx->ItemIndex = 1;
	}
	else if (StartsStr("o", renderer)) {
		RendererCbx->ItemIndex = 2;
	}
	else if (StartsStr("s", renderer) || StartsStr("g", renderer)) {
		RendererCbx->ItemIndex = 3;
	}
	else {
		RendererCbx->ItemIndex = 0;
	}

	try
	{
		TStringDynArray list = TDirectory::GetFiles(
			"Shaders",
			"*.glsl",
			TSearchOption::soAllDirectories);

		for (int i = 0; i < list.Length; i++)
			ShaderCbx->AddItem(list[i], NULL);

		auto shader = ini->ReadString("ddraw", "shader", "");
		ShaderCbx->ItemIndex = ShaderCbx->Items->IndexOf(shader);
	}
	catch (...)
	{
	}

	Maxfps = ini->ReadInteger("ddraw", "maxfps", -1);
	MaxfpsChk->State = Maxfps != 0 ? tssOn : tssOff;

	BoxingChk->State = GetBool(ini, "boxing", false) ? tssOn : tssOff;
	BorderChk->State = GetBool(ini, "border", false) ? tssOn : tssOff;

	Savesettings = ini->ReadInteger("ddraw", "savesettings", 1);
	SavesettingsChk->State = Savesettings != 0 ? tssOn : tssOff;

	/* Compatibility Settings */

	int maxgameticks = ini->ReadInteger("ddraw", "maxgameticks", 0);

	switch (maxgameticks) {
	case -1:
		MaxgameticksCbx->ItemIndex = 0;
		break;
	case -2:
		MaxgameticksCbx->ItemIndex = 1;
		break;
	case 0:
		MaxgameticksCbx->ItemIndex = 2;
		break;
	case 1000:
		MaxgameticksCbx->ItemIndex = 3;
		break;
	case 500:
		MaxgameticksCbx->ItemIndex = 4;
		break;
	case 60:
		MaxgameticksCbx->ItemIndex = 5;
		break;
	case 30:
		MaxgameticksCbx->ItemIndex = 6;
		break;
	case 25:
		MaxgameticksCbx->ItemIndex = 7;
		break;
	case 15:
		MaxgameticksCbx->ItemIndex = 8;
		break;
	default:
		MaxgameticksCbx->AddItem(IntToStr(maxgameticks), NULL);
		MaxgameticksCbx->ItemIndex = 9;
		break;
	}


	NoactivateappChk->State = GetBool(ini, "noactivateapp", false) ? tssOn : tssOff;

	Hook = ini->ReadInteger("ddraw", "hook", 4);
	HookChk->State = Hook == 2 ? tssOn : tssOff;

	Minfps = ini->ReadInteger("ddraw", "minfps", 0);
	MinfpsChk->State = Minfps != 0 ? tssOn : tssOff;

	FixpitchChk->State = GetBool(ini, "fixpitch", false) ? tssOn : tssOff;
	NonexclusiveChk->State = GetBool(ini, "nonexclusive", false) ? tssOn : tssOff;

	delete ini;

	Initialized = true;
}

void TConfigForm::SaveSettings()
{
	if (!Initialized)
		return;

	auto *ini = new TIniFile(".\\ddraw.ini");

	/* Display Settings */

	switch(PresentationCbx->ItemIndex) {
	case 0:
		ini->WriteString("ddraw", "windowed", "false");
		ini->WriteString("ddraw", "fullscreen", "false");
		break;
	case 1:
		ini->WriteString("ddraw", "windowed", "true");
		ini->WriteString("ddraw", "fullscreen", "true");
		break;
	case 2:
		ini->WriteString("ddraw", "windowed", "true");
		ini->WriteString("ddraw", "fullscreen", "false");
		break;
	default:
		break;
	}

	ini->WriteString(
		"ddraw",
		"maintas",
		MaintasChk->State == tssOn ? "true" : "false");

	ini->WriteString(
		"ddraw",
		"vsync",
		VsyncChk->State == tssOn ? "true" : "false");

	ini->WriteString(
		"ddraw",
		"adjmouse",
		AdjmouseChk->State == tssOn ? "true" : "false");

	ini->WriteString(
		"ddraw",
		"devmode",
		DevmodeChk->State == tssOn ? "false" : "true");

	/* Advanced Display Settings */

	ini->WriteString("ddraw", "renderer", LowerCase(RendererCbx->Text));
	ini->WriteString("ddraw", "shader", ShaderCbx->Text);

	int maxfps = Maxfps == 0 ? -1 : Maxfps;

	ini->WriteInteger(
		"ddraw",
		"maxfps",
		MaxfpsChk->State == tssOn ? maxfps : 0);

	ini->WriteString(
		"ddraw",
		"boxing",
		BoxingChk->State == tssOn ? "true" : "false");

	ini->WriteString(
		"ddraw",
		"border",
		BorderChk->State == tssOn ? "true" : "false");

	int savesettings = Savesettings == 0 ? 1 : Savesettings;

	ini->WriteInteger(
		"ddraw",
		"savesettings",
		SavesettingsChk->State == tssOn ? savesettings : 0);

	if (Savesettings != 0 && SavesettingsChk->State == tssOff) {
		ini->WriteInteger("ddraw", "width", 0);
		ini->WriteInteger("ddraw", "height", 0);
		ini->WriteInteger("ddraw", "posX", -32000);
		ini->WriteInteger("ddraw", "posY", -32000);
	}

	/* Compatibility Settings */

	switch(MaxgameticksCbx->ItemIndex) {
	case 0:
		ini->WriteInteger("ddraw", "maxgameticks", -1);
		break;
	case 1:
		ini->WriteInteger("ddraw", "maxgameticks", -2);
		break;
	case 2:
		ini->WriteInteger("ddraw", "maxgameticks", 0);
		break;
	case 3:
		ini->WriteInteger("ddraw", "maxgameticks", 1000);
		break;
	case 4:
		ini->WriteInteger("ddraw", "maxgameticks", 500);
		break;
	case 5:
		ini->WriteInteger("ddraw", "maxgameticks", 60);
		break;
	case 6:
		ini->WriteInteger("ddraw", "maxgameticks", 30);
		break;
	case 7:
		ini->WriteInteger("ddraw", "maxgameticks", 25);
		break;
	case 8:
		ini->WriteInteger("ddraw", "maxgameticks", 15);
		break;
	case 9:
		ini->WriteString("ddraw", "maxgameticks", MaxgameticksCbx->Text);
		break;
	default:
		break;
	}

	ini->WriteString(
		"ddraw",
		"noactivateapp",
		NoactivateappChk->State == tssOn ? "true" : "false");

	int hook = Hook != 2 ? Hook : 4;

	ini->WriteInteger(
		"ddraw",
		"hook",
		HookChk->State == tssOn ? 2 : hook);

	if (HookChk->State == tssOn)
		ini->WriteString("ddraw", "renderer", "gdi");

	int minfps = Minfps == 0 ? -1 : Minfps;

	ini->WriteInteger(
		"ddraw",
		"minfps",
		MinfpsChk->State == tssOn ? minfps : 0);

	ini->WriteString(
		"ddraw",
		"fixpitch",
		FixpitchChk->State == tssOn ? "true" : "false");

	ini->WriteString(
		"ddraw",
		"nonexclusive",
		NonexclusiveChk->State == tssOn ? "true" : "false");

	delete ini;
}

bool TConfigForm::GetBool(TIniFile *ini, System::UnicodeString key, bool defValue)
{
	auto s = LowerCase(ini->ReadString("ddraw", key, defValue ? "true" : "false"));
	return s == "true" || s == "yes" || s == "1";
}

void __fastcall TConfigForm::PresentationCbxChange(TObject *Sender)
{
	SaveSettings();
}

void __fastcall TConfigForm::MaintasChkClick(TObject *Sender)
{
	SaveSettings();
}

void __fastcall TConfigForm::VsyncChkClick(TObject *Sender)
{
	SaveSettings();
}

void __fastcall TConfigForm::AdjmouseChkClick(TObject *Sender)
{
	SaveSettings();
}

void __fastcall TConfigForm::DevmodeChkClick(TObject *Sender)
{
	SaveSettings();
}

void __fastcall TConfigForm::RendererCbxChange(TObject *Sender)
{
	SaveSettings();
}

void __fastcall TConfigForm::ShaderCbxChange(TObject *Sender)
{
	SaveSettings();
}

void __fastcall TConfigForm::MaxfpsChkClick(TObject *Sender)
{
	SaveSettings();
}

void __fastcall TConfigForm::BoxingChkClick(TObject *Sender)
{
	SaveSettings();
}

void __fastcall TConfigForm::BorderChkClick(TObject *Sender)
{
	SaveSettings();
}

void __fastcall TConfigForm::SavesettingsChkClick(TObject *Sender)
{
	SaveSettings();
}

void __fastcall TConfigForm::MaxgameticksCbxChange(TObject *Sender)
{
	SaveSettings();
}

void __fastcall TConfigForm::NoactivateappChkClick(TObject *Sender)
{
	SaveSettings();
}

void __fastcall TConfigForm::HookChkClick(TObject *Sender)
{
	SaveSettings();
}

void __fastcall TConfigForm::MinfpsChkClick(TObject *Sender)
{
	SaveSettings();
}

void __fastcall TConfigForm::FixpitchChkClick(TObject *Sender)
{
	SaveSettings();
}

void __fastcall TConfigForm::NonexclusiveChkClick(TObject *Sender)
{
	SaveSettings();
}

