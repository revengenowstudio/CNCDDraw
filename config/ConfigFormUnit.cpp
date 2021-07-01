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

	auto s = LowerCase(ini->ReadString("ddraw", "windowed", "false"));
	bool windowed = s == "true" || s == "yes" || s == "1";

	s = LowerCase(ini->ReadString("ddraw", "fullscreen", "false"));
	bool fullscreen = s == "true" || s == "yes" || s == "1";

	s = LowerCase(ini->ReadString("ddraw", "nonexclusive", "false"));
	bool nonexclusive = s == "true" || s == "yes" || s == "1";


	if (windowed && fullscreen) {
		PresentationCbx->ItemIndex = 2;
	}
	else if (windowed) {
		PresentationCbx->ItemIndex = 3;
	}
	else if (nonexclusive) {
		PresentationCbx->ItemIndex = 1;
	}
	else {
		PresentationCbx->ItemIndex = 0;
	}


	s = LowerCase(ini->ReadString("ddraw", "maintas", "false"));
	MaintasChk->State = s == "true" || s == "yes" || s == "1" ? tssOn : tssOff;

	s = LowerCase(ini->ReadString("ddraw", "vsync", "false"));
	VsyncChk->State = s == "true" || s == "yes" || s == "1" ? tssOn : tssOff;

	s = LowerCase(ini->ReadString("ddraw", "adjmouse", "false"));
	AdjmouseChk->State = s == "true" || s == "yes" || s == "1" ? tssOn : tssOff;

	s = LowerCase(ini->ReadString("ddraw", "devmode", "false"));
	DevmodeChk->State = s == "true" || s == "yes" || s == "1" ? tssOff : tssOn;

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


	int maxfps = ini->ReadInteger("ddraw", "maxfps", -1);
	MaxfpsChk->State = maxfps != 0 ? tssOn : tssOff;

	s = LowerCase(ini->ReadString("ddraw", "boxing", "false"));
	BoxingChk->State = s == "true" || s == "yes" || s == "1" ? tssOn : tssOff;

	s = LowerCase(ini->ReadString("ddraw", "border", "false"));
	BorderChk->State = s == "true" || s == "yes" || s == "1" ? tssOn : tssOff;

	int savesettings = ini->ReadInteger("ddraw", "savesettings", 1);
	SavesettingsChk->State = savesettings != 0 ? tssOn : tssOff;

	delete ini;

	Initialized = true;
}

void TConfigForm::SaveSettings()
{
	if (!Initialized)
		return;

	auto *ini = new TIniFile(".\\ddraw.ini");

	/* Display Settings */

	switch(PresentationCbx->ItemIndex)
	{
	case 0:
		ini->WriteString("ddraw", "windowed", "false");
		ini->WriteString("ddraw", "fullscreen", "false");
		ini->WriteString("ddraw", "nonexclusive", "false");
		break;
	case 1:
		ini->WriteString("ddraw", "windowed", "false");
		ini->WriteString("ddraw", "fullscreen", "false");
		ini->WriteString("ddraw", "nonexclusive", "true");
		break;
	case 2:
		ini->WriteString("ddraw", "windowed", "true");
		ini->WriteString("ddraw", "fullscreen", "true");
		ini->WriteString("ddraw", "nonexclusive", "false");
		break;
	case 3:
		ini->WriteString("ddraw", "windowed", "true");
		ini->WriteString("ddraw", "fullscreen", "false");
		ini->WriteString("ddraw", "nonexclusive", "false");
		break;
	default:
		break;
	}

	ini->WriteString(
		"ddraw",
		"renderer",
		LowerCase(RendererCbx->Text));

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

	ini->WriteInteger(
		"ddraw",
		"maxfps",
		MaxfpsChk->State == tssOn ? -1 : 0);

	ini->WriteString(
		"ddraw",
		"boxing",
		BoxingChk->State == tssOn ? "true" : "false");

	ini->WriteString(
		"ddraw",
		"border",
		BorderChk->State == tssOn ? "true" : "false");

	ini->WriteInteger(
		"ddraw",
		"savesettings",
		SavesettingsChk->State == tssOn ? 1 : 0);

	delete ini;
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

