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
	TSpeedButton *AdvDisplayBtn;
	TSpeedButton *CompatibilityBtn;
	TPanel *AdvDisplayPnl;
	TPanel *CompatibilityPnl;
	TComboBox *PresentationCbx;
	TLabel *PresentationLbl;
	TLabel *MaintasLbl;
	TToggleSwitch *MaintasChk;
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
	void __fastcall AdvDisplayBtnClick(TObject *Sender);
	void __fastcall CompatibilityBtnClick(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall PresentationCbxChange(TObject *Sender);
	void __fastcall MaintasChkClick(TObject *Sender);
	void __fastcall VsyncChkClick(TObject *Sender);
	void __fastcall AdjmouseChkClick(TObject *Sender);
	void __fastcall DevmodeChkClick(TObject *Sender);
	void __fastcall RendererCbxChange(TObject *Sender);
	void __fastcall ShaderCbxChange(TObject *Sender);
	void __fastcall MaxfpsChkClick(TObject *Sender);
	void __fastcall BoxingChkClick(TObject *Sender);
	void __fastcall BorderChkClick(TObject *Sender);
	void __fastcall SavesettingsChkClick(TObject *Sender);
private:	// Benutzer-Deklarationen
	void SaveSettings();
public:		// Benutzer-Deklarationen
	__fastcall TConfigForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TConfigForm *ConfigForm;
//---------------------------------------------------------------------------
#endif
