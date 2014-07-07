//---------------------------------------------------------------------------
// Copyright (C) 2014 Krzysztof Grochocki
//
// This file is part of ArchiveLock
//
// ArchiveLock is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3, or (at your option)
// any later version.
//
// ArchiveLock is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GNU Radio; see the file COPYING. If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street,
// Boston, MA 02110-1301, USA.
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "UnlockFrm.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "acPNG"
#pragma link "sButton"
#pragma link "sEdit"
#pragma link "sSkinManager"
#pragma link "sSkinProvider"
#pragma resource "*.dfm"
TUnlockForm *UnlockForm;
//---------------------------------------------------------------------------
__declspec(dllimport)UnicodeString GetThemeSkinDir();
__declspec(dllimport)UnicodeString GetThemeDir();
__declspec(dllimport)UnicodeString GetDefaultThemeDir();
__declspec(dllimport)bool ChkSkinEnabled();
__declspec(dllimport)bool ChkThemeAnimateWindows();
__declspec(dllimport)bool ChkThemeGlowing();
__declspec(dllimport)int GetHUE();
__declspec(dllimport)int GetSaturation();
__declspec(dllimport)UnicodeString MD5(UnicodeString Text);
__declspec(dllimport)UnicodeString GetPassword();
//---------------------------------------------------------------------------
__fastcall TUnlockForm::TUnlockForm(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------

void __fastcall TUnlockForm::WMTransparency(TMessage &Message)
{
  Application->ProcessMessages();
  if(sSkinManager->Active) sSkinProvider->BorderForm->UpdateExBordersPos(true,(int)Message.LParam);
}
//---------------------------------------------------------------------------

void __fastcall TUnlockForm::FormCreate(TObject *Sender)
{
  //Wlaczona zaawansowana stylizacja okien
  if(ChkSkinEnabled())
  {
	UnicodeString ThemeSkinDir = GetThemeSkinDir();
	//Plik zaawansowanej stylizacji okien istnieje
	if(FileExists(ThemeSkinDir + "\\\\Skin.asz"))
	{
	  //Dane pliku zaawansowanej stylizacji okien
	  ThemeSkinDir = StringReplace(ThemeSkinDir, "\\\\", "\\", TReplaceFlags() << rfReplaceAll);
	  sSkinManager->SkinDirectory = ThemeSkinDir;
	  sSkinManager->SkinName = "Skin.asz";
	  //Ustawianie animacji AlphaControls
	  if(ChkThemeAnimateWindows()) sSkinManager->AnimEffects->FormShow->Time = 200;
	  else sSkinManager->AnimEffects->FormShow->Time = 0;
	  sSkinManager->Effects->AllowGlowing = ChkThemeGlowing();
	  //Zmiana kolorystyki AlphaControls
	  sSkinManager->HueOffset = GetHUE();
	  sSkinManager->Saturation = GetSaturation();
      //Aktywacja skorkowania AlphaControls
	  sSkinManager->Active = true;
	}
	//Brak pliku zaawansowanej stylizacji okien
	else sSkinManager->Active = false;
  }
  //Zaawansowana stylizacja okien wylaczona
  else sSkinManager->Active = false;
}
//---------------------------------------------------------------------------

void __fastcall TUnlockForm::FormShow(TObject *Sender)
{
  //Z gory zablokowanie dostepu do archiwum
  Unlocked = false;
  //Wczytanie grafiki klodki z aktywnej kompozycji
  SecureImage->Picture->Bitmap->TransparentColor = clBlack;
  if(FileExists(GetThemeDir()+"////Graphics////Secure.png"))
   SecureImage->Picture->Bitmap->LoadFromFile(GetThemeDir()+"////Graphics////Secure.png");
  else if(FileExists(GetDefaultThemeDir()+"////Graphics////Secure.png"))
   SecureImage->Picture->Bitmap->LoadFromFile(GetDefaultThemeDir()+"////Graphics////Secure.png");
}
//---------------------------------------------------------------------------

void __fastcall TUnlockForm::aExitExecute(TObject *Sender)
{
  //Zamkniecie formy
  Close();
}
//---------------------------------------------------------------------------

void __fastcall TUnlockForm::aUnlockExecute(TObject *Sender)
{
  //Sprawdzanie poprawnosci hasla
  if(MD5(PassEdit->Text)==GetPassword())
  {
	//Odblokowanie dostepu do archiwum
	Unlocked = true;
	//Zamkniecie formy
	Close();
  }
  else Application->MessageBox(L"B³êdne has³o",L"B³¹d",MB_ICONWARNING);
}
//---------------------------------------------------------------------------

void __fastcall TUnlockForm::sSkinManagerSysDlgInit(TacSysDlgData DlgData, bool &AllowSkinning)
{
  AllowSkinning = false;
}
//---------------------------------------------------------------------------

