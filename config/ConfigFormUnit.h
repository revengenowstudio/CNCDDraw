//---------------------------------------------------------------------------

#ifndef ConfigFormUnitH
#define ConfigFormUnitH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.WinXCtrls.hpp>
#include <Vcl.Buttons.hpp>
//---------------------------------------------------------------------------
class TConfigForm : public TForm
{
__published:	// Von der IDE verwaltete Komponenten
	TPanel *MenuPnl;
	TPanel *DisplayPnl;
	TSpeedButton *DisplayBtn;
	TSpeedButton *AdvSettingsBtn;
	TSpeedButton *CompatibilityBtn;
	TPanel *AdvDisplayPnl;
	TPanel *CompatibilityPnl;
	TComboBox *PresentationCbx;
	TLabel *PresentationLbl;
	TLabel *AspectRatioLbl;
	TToggleSwitch *AspectRatioChk;
	TLabel *VsyncLbl;
	TToggleSwitch *VsyncChk;
	TLabel *AdjmouseLbl;
	TToggleSwitch *AdjmouseChk;
	TLabel *DevmodeLbl;
	TToggleSwitch *DevmodeChk;
	TComboBox *RendererCbx;
	TLabel *RendererLbl;
	TLabel *BorderLbl;
	TToggleSwitch *BorderChk;
	TLabel *SavesettingsLbl;
	TToggleSwitch *SavesettingsChk;
	TComboBox *ShaderCbx;
	TLabel *ShaderLbl;
	TLabel *MaxfpsLbl;
	TToggleSwitch *MaxfpsChk;
	TLabel *BoxingLbl;
	TToggleSwitch *BoxingChk;
	void __fastcall DisplayBtnClick(TObject *Sender);
	void __fastcall AdvSettingsBtnClick(TObject *Sender);
private:	// Benutzer-Deklarationen
public:		// Benutzer-Deklarationen
	__fastcall TConfigForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TConfigForm *ConfigForm;
//---------------------------------------------------------------------------
#endif
