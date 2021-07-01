//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "ConfigFormUnit.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TConfigForm *ConfigForm;
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

void __fastcall TConfigForm::AdvSettingsBtnClick(TObject *Sender)
{
	AdvDisplayPnl->Visible = true;
	DisplayPnl->Visible = false;
	CompatibilityPnl->Visible = false;
}




