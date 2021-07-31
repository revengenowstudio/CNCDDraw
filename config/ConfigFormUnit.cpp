//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <IniFiles.hpp>
#include <StrUtils.hpp>
#include <IOUtils.hpp>
#include <SysUtils.hpp>
#include "ConfigFormUnit.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TConfigForm *ConfigForm;
bool Initialized;
bool IsEnglish;

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

void __fastcall TConfigForm::LanguageLblClick(TObject *Sender)
{
	auto *ini = new TIniFile(".\\ddraw.ini");
	ini->WriteString("ddraw", "configlang", IsEnglish ? "auto" : "english");
	delete ini;

	ShellExecute(
		NULL,
		L"open",
		Application->ExeName.w_str(),
		NULL,
		NULL,
		SW_SHOWNORMAL);

	Application->Terminate();
}

void TConfigForm::ApplyTranslation(TIniFile *ini)
{
	auto lang = LowerCase(ini->ReadString("ddraw", "configlang", "auto"));
	int priID = SysLocale.PriLangID;

	if (lang == "chinese" || (lang == "auto" && priID == LANG_CHINESE)) {
		LanguageLbl->Visible = true;

		/* -Chinese Simplified- made by universal963 @ github */

		ConfigForm->Caption = L"cnc-ddraw 配置";
		DisplayBtn->Caption = L"显示设置";
		AdvancedBtn->Caption = L"高级设置";
		CompatibilityBtn->Caption = L"兼容性设置";
		PresentationLbl->Caption = L"显示方式";
		MaintasLbl->Caption = L"保持纵横比";
		VsyncLbl->Caption = L"打开垂直同步";
		AdjmouseLbl->Caption = L"调整鼠标灵敏度";
		DevmodeLbl->Caption = L"锁定光标到窗口/屏幕";
		RendererLbl->Caption = L"渲染器";
		BorderLbl->Caption = L"在窗口模式下显示窗口边框";
		SavesettingsLbl->Caption = L"记住窗口位置和大小";
		ShaderLbl->Caption = L"OpenGL着色器";
		MaxfpsLbl->Caption = L"限制帧率";
		BoxingLbl->Caption = L"打开窗盒显示/整数缩放";
		MaxgameticksLbl->Caption = L"限制游戏速率";
		NoactivateappLbl->Caption = L"修复损坏的Alt+Tab功能";
		HookLbl->Caption = L"修复损坏的窗口模式或拉伸";
		MinfpsLbl->Caption = L"强制高FPS / 修复使用Freesync/G-Sync的卡顿问题";
		FixpitchLbl->Caption = L"修复倾斜撕裂显示的问题";
		NonexclusiveLbl->Caption = L"修复不显示的视频/UI元素";

		RendererCbx->Items->Clear();
		RendererCbx->AddItem(L"自动", NULL);
		RendererCbx->AddItem(L"Direct3D9", NULL);
		RendererCbx->AddItem(L"OpenGL", NULL);
		RendererCbx->AddItem(L"GDI", NULL);

		PresentationCbx->Items->Clear();
		PresentationCbx->AddItem(L"全屏", NULL);
		PresentationCbx->AddItem(L"拉伸至全屏", NULL);
		PresentationCbx->AddItem(L"无边框", NULL);
		PresentationCbx->AddItem(L"窗口化", NULL);

		MaxgameticksCbx->Items->Clear();
		MaxgameticksCbx->AddItem(L"无限制", NULL);
		MaxgameticksCbx->AddItem(L"与显示器刷新率同步", NULL);
		MaxgameticksCbx->AddItem(L"模拟60hz刷新率显示器", NULL);
		MaxgameticksCbx->AddItem(L"1000tick每秒", NULL);
		MaxgameticksCbx->AddItem(L"500tick每秒", NULL);
		MaxgameticksCbx->AddItem(L"60tick每秒", NULL);
		MaxgameticksCbx->AddItem(L"30tick每秒", NULL);
		MaxgameticksCbx->AddItem(L"25tick每秒", NULL);
		MaxgameticksCbx->AddItem(L"15tick每秒", NULL);
	}
	else if (lang == "spanish" || (lang == "auto" && priID == LANG_SPANISH)) {
		LanguageLbl->Visible = true;

		/* -Spanish- made by c-sanchez @ github */

		ConfigForm->Caption = L"Ajustes de cnc-ddraw";
		DisplayBtn->Caption = L"Ajustes de pantalla";
		AdvancedBtn->Caption = L"Ajustes avanzados";
		CompatibilityBtn->Caption = L"Ajustes de compatibilidad";
		PresentationLbl->Caption = L"Presentación";
		MaintasLbl->Caption = L"Mantener la relación de aspecto";
		VsyncLbl->Caption = L"Activar VSync";
		AdjmouseLbl->Caption = L"Ajustar sensibilidad de ratón";
		DevmodeLbl->Caption = L"Bloquear cursor a la ventana / pantalla";
		RendererLbl->Caption = L"Renderizador";
		BorderLbl->Caption = L"Mostrar bordes en modo ventana";
		SavesettingsLbl->Caption = L"Recordar posición y tamaño de ventana";
		ShaderLbl->Caption = L"Sombreador OpenGL";
		MaxfpsLbl->Caption = L"Limitar velocidad de fotogramas";
		BoxingLbl->Caption = L"Activar encajado de ventanas / escalado de enteros";
		MaxgameticksLbl->Caption = L"Limitar velocidad de juego";
		NoactivateappLbl->Caption = L"Corregir Alt+Tab roto";
		HookLbl->Caption = L"Corregir modo ventana o ampliación de escala";
		MinfpsLbl->Caption = L"Forzar un alto FPS / Corregir retrasos en Freesync/G-Sync";
		FixpitchLbl->Caption = L"Corregir problemas de visualización de dibujos en diagonal";
		NonexclusiveLbl->Caption = L"Corregir vídeos / elementos de interfaz invisibles";

		RendererCbx->Items->Clear();
		RendererCbx->AddItem(L"Automático", NULL);
		RendererCbx->AddItem(L"Direct3D9", NULL);
		RendererCbx->AddItem(L"OpenGL", NULL);
		RendererCbx->AddItem(L"GDI", NULL);

		PresentationCbx->Items->Clear();
		PresentationCbx->AddItem(L"Pantalla completa", NULL);
		PresentationCbx->AddItem(L"Pantalla completa ampliada", NULL);
		PresentationCbx->AddItem(L"Sin bordes", NULL);
		PresentationCbx->AddItem(L"Ventana", NULL);

		MaxgameticksCbx->Items->Clear();
		MaxgameticksCbx->AddItem(L"Sin límite", NULL);
		MaxgameticksCbx->AddItem(L"Sincronizar con tasa de refresco de monitor", NULL);
		MaxgameticksCbx->AddItem(L"Emular monitor con tasa de refresco de 60hz", NULL);
		MaxgameticksCbx->AddItem(L"1000 tics por segundo", NULL);
		MaxgameticksCbx->AddItem(L"500 tics por segundo", NULL);
		MaxgameticksCbx->AddItem(L"60 tics por segundo", NULL);
		MaxgameticksCbx->AddItem(L"30 tics por segundo", NULL);
		MaxgameticksCbx->AddItem(L"25 tics por segundo", NULL);
		MaxgameticksCbx->AddItem(L"15 tics por segundo", NULL);
	}
	else if (lang == "german" || (lang == "auto" && priID == LANG_GERMAN)) {
		LanguageLbl->Visible = true;

		/* -German- made by helgo1506 @ github */

		ConfigForm->Caption = L"cnc-ddraw Konfiguration";
		DisplayBtn->Caption = L"Anzeigeeinstellungen";
		AdvancedBtn->Caption = L"Erweiterte Einstellungen";
		CompatibilityBtn->Caption = L"Kompatibilitätseinstellungen";
		PresentationLbl->Caption = L"Presentation";
		MaintasLbl->Caption = L"Erhalte Seitenverhältnis";
		VsyncLbl->Caption = L"VSync aktiveren";
		AdjmouseLbl->Caption = L"Mausempfindlichkeit anpassen";
		DevmodeLbl->Caption = L"Sperre Cursor zu Fenster / Bildschirm"; //Not 100% sure, if not a better translation exists
		RendererLbl->Caption = L"Renderer";
		BorderLbl->Caption = L"Zeige Fensterränder in Fenstermodus";
		SavesettingsLbl->Caption = L"Fensterposition und Größe merken";
		ShaderLbl->Caption = L"OpenGL shader";
		MaxfpsLbl->Caption = L"Limitiere Aktualisierungsrate";
		BoxingLbl->Caption = L"Fensterboxing / Integer Skalierung aktivieren"; //Not 100% sure if "windowboxing" can be translated better.
		MaxgameticksLbl->Caption = L"Spielgeschwindigkeit limitieren";
		NoactivateappLbl->Caption = L"Fehlerhaftes Alt+Tab reparieren"; //The first word can be ignored if its to long (eng word "Fix"
		HookLbl->Caption = L"Fehlerhafter Fenstermodus oder Hochskalierung reparieren"; //The first word can be ignored if its to long (eng word "Fix")
		MinfpsLbl->Caption = L"Erzwinge Hohe FPS / Repariere Stottern bei Freesync/G-Sync";
		FixpitchLbl->Caption = L"Diagonal dargestellte Zeichnungsfehler reparieren";
		NonexclusiveLbl->Caption = L"Unsichtbare Videos / UI Elemente reparieren";

		RendererCbx->Items->Clear();
		RendererCbx->AddItem(L"Automatisch", NULL);
		RendererCbx->AddItem(L"Direct3D9", NULL);
		RendererCbx->AddItem(L"OpenGL", NULL);
		RendererCbx->AddItem(L"GDI", NULL);

		PresentationCbx->Items->Clear();
		PresentationCbx->AddItem(L"Vollbild", NULL);
		PresentationCbx->AddItem(L"Hochskaliertes Vollbild", NULL);
		PresentationCbx->AddItem(L"Ränderfreies Fenster", NULL);
		PresentationCbx->AddItem(L"Fenster", NULL);

		MaxgameticksCbx->Items->Clear();
		MaxgameticksCbx->AddItem(L"Unlimitiert", NULL);
		MaxgameticksCbx->AddItem(L"Sync mit Bildschirmaktualisierungsrate", NULL);
		MaxgameticksCbx->AddItem(L"Emuliere 60hz Bildschirmaktualisierungsrate", NULL);
		MaxgameticksCbx->AddItem(L"1000 Ticks pro Sekunde", NULL);
		MaxgameticksCbx->AddItem(L"500 Ticks pro Sekunde", NULL);
		MaxgameticksCbx->AddItem(L"60 Ticks pro Sekunde", NULL);
		MaxgameticksCbx->AddItem(L"30 Ticks pro Sekunde", NULL);
		MaxgameticksCbx->AddItem(L"25 Ticks pro Sekunde", NULL);
		MaxgameticksCbx->AddItem(L"15 Ticks pro Sekunde", NULL);
	}
	else {
		IsEnglish = true;
		UnicodeString name = "";

		try {
			int lcid = Languages()->IndexOf(SysLocale.DefaultLCID);
			name = SplitString(Languages()->Name[lcid].w_str(), L" (")[0];
		} catch (...) {
		}

		if (priID == LANG_CHINESE) {
			LanguageLbl->Visible = true;
			LanguageLbl->Caption = name == "" ? "Chinese" : name;
		}
		else if (priID == LANG_SPANISH) {
			LanguageLbl->Visible = true;
			LanguageLbl->Caption = name == "" ? "Spanish" : name;
		}
		else if (priID == LANG_GERMAN) {
			LanguageLbl->Visible = true;
			LanguageLbl->Caption = name == "" ? "German" : name;
		}

		/*
		ConfigForm->Caption = L"cnc-ddraw config";
		DisplayBtn->Caption = L"Display Settings";
		AdvancedBtn->Caption = L"Advanced Settings";
		CompatibilityBtn->Caption = L"Compatibility Settings";
		PresentationLbl->Caption = L"Presentation";
		MaintasLbl->Caption = L"Maintain aspect ratio";
		VsyncLbl->Caption = L"Enable VSync";
		AdjmouseLbl->Caption = L"Adjust mouse sensitivity";
		DevmodeLbl->Caption = L"Lock cursor to window / screen";
		RendererLbl->Caption = L"Renderer";
		BorderLbl->Caption = L"Show window borders in windowed mode";
		SavesettingsLbl->Caption = L"Remember window position and size";
		ShaderLbl->Caption = L"OpenGL shader";
		MaxfpsLbl->Caption = L"Limit frame rate";
		BoxingLbl->Caption = L"Enable windowboxing / integer scaling";
		MaxgameticksLbl->Caption = L"Limit game speed";
		NoactivateappLbl->Caption = L"Fix broken Alt+Tab";
		HookLbl->Caption = L"Fix broken windowed mode or upscaling";
		MinfpsLbl->Caption = L"Force high FPS / Fix stuttering on Freesync/G-Sync";
		FixpitchLbl->Caption = L"Fix diagonally displayed drawing issues";
		NonexclusiveLbl->Caption = L"Fix invisible videos / UI elements";

		RendererCbx->Items->Clear();
		RendererCbx->AddItem(L"Automatic", NULL);
		RendererCbx->AddItem(L"Direct3D9", NULL);
		RendererCbx->AddItem(L"OpenGL", NULL);
		RendererCbx->AddItem(L"GDI", NULL);

		PresentationCbx->Items->Clear();
		PresentationCbx->AddItem(L"Fullscreen", NULL);
		PresentationCbx->AddItem(L"Fullscreen Upscaled", NULL);
		PresentationCbx->AddItem(L"Borderless", NULL);
		PresentationCbx->AddItem(L"Windowed", NULL);

		MaxgameticksCbx->Items->Clear();
		MaxgameticksCbx->AddItem(L"No limit", NULL);
		MaxgameticksCbx->AddItem(L"Sync with monitor refresh rate", NULL);
		MaxgameticksCbx->AddItem(L"Emulate 60hz refresh rate monitor", NULL);
		MaxgameticksCbx->AddItem(L"1000 ticks per second", NULL);
		MaxgameticksCbx->AddItem(L"500 ticks per second", NULL);
		MaxgameticksCbx->AddItem(L"60 ticks per second", NULL);
		MaxgameticksCbx->AddItem(L"30 ticks per second", NULL);
		MaxgameticksCbx->AddItem(L"25 ticks per second", NULL);
		MaxgameticksCbx->AddItem(L"15 ticks per second", NULL);
		*/
	}
}

void __fastcall TConfigForm::DisplayBtnClick(TObject *Sender)
{
	DisplayPnl->Visible = true;
	AdvancedPnl->Visible = false;
	CompatibilityPnl->Visible = false;
}

void __fastcall TConfigForm::AdvancedBtnClick(TObject *Sender)
{
	AdvancedPnl->Visible = true;
	DisplayPnl->Visible = false;
	CompatibilityPnl->Visible = false;
}

void __fastcall TConfigForm::CompatibilityBtnClick(TObject *Sender)
{
	CompatibilityPnl->Visible = true;
	AdvancedPnl->Visible = false;
	DisplayPnl->Visible = false;
}

void __fastcall TConfigForm::FormCreate(TObject *Sender)
{
	auto *ini = new TIniFile(".\\ddraw.ini");

	ApplyTranslation(ini);

	/* Display Settings */

	bool windowed = GetBool(ini, "windowed", false);
	bool fullscreen = GetBool(ini, "fullscreen", false);

	if (windowed && fullscreen) {
		PresentationCbx->ItemIndex = 2;
	}
	else if (windowed) {
		PresentationCbx->ItemIndex = 3;
	}
	else if (fullscreen) {
		PresentationCbx->ItemIndex = 1;
	}
	else {
		PresentationCbx->ItemIndex = 0;
	}

	MaintasChk->State = GetBool(ini, "maintas", false) ? tssOn : tssOff;
	VsyncChk->State = GetBool(ini, "vsync", false) ? tssOn : tssOff;
	AdjmouseChk->State = GetBool(ini, "adjmouse", true) ? tssOn : tssOff;
	DevmodeChk->State = GetBool(ini, "devmode", false) ? tssOff : tssOn;

	/* Advanced Settings */

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
	BorderChk->State = GetBool(ini, "border", true) ? tssOn : tssOff;

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
		ini->WriteString("ddraw", "windowed", "false");
		ini->WriteString("ddraw", "fullscreen", "true");
		break;
	case 2:
		ini->WriteString("ddraw", "windowed", "true");
		ini->WriteString("ddraw", "fullscreen", "true");
		break;
	case 3:
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

	/* Advanced Settings */

	switch(RendererCbx->ItemIndex) {
	case 0:
		ini->WriteString("ddraw", "renderer", "auto");
		break;
	case 1:
		ini->WriteString("ddraw", "renderer", "direct3d9");
		break;
	case 2:
		ini->WriteString("ddraw", "renderer", "opengl");
		break;
	case 3:
		ini->WriteString("ddraw", "renderer", "gdi");
		break;
	default:
		break;
	}

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

	if (HookChk->State == tssOn && Hook != 2)
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

void __fastcall TConfigForm::PboxPaint(TObject *Sender)
{
	TPaintBox *pbox = static_cast<TPaintBox*>(Sender);
	pbox->Canvas->Rectangle(pbox->ClientRect);
}

