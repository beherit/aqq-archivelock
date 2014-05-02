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
#include "ChangePassFrm.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "acPNG"
#pragma link "sButton"
#pragma link "sEdit"
#pragma link "sSkinManager"
#pragma link "sSkinProvider"
#pragma resource "*.dfm"
TChangePassForm *ChangePassForm;
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
__declspec(dllimport)void SetPassword(UnicodeString Password);
//---------------------------------------------------------------------------
__fastcall TChangePassForm::TChangePassForm(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------

void __fastcall TChangePassForm::WMTransparency(TMessage &Message)
{
  Application->ProcessMessages();
  if(sSkinManager->Active) sSkinProvider->BorderForm->UpdateExBordersPos(true,(int)Message.LParam);
}
//---------------------------------------------------------------------------

void __fastcall TChangePassForm::FormCreate(TObject *Sender)
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

void __fastcall TChangePassForm::FormShow(TObject *Sender)
{
  //Kasowanie danych z pol
  OldPassEdit->Text = "";
  NewPassEdit->Text = "";
  ConfirmNewPassEdit->Text = "";
  //Ustawianie fokusu
  OldPassEdit->SetFocus();
  //Wczytanie grafiki klodki z aktywnej kompozycji
  SecureImage->Picture->Bitmap->TransparentColor = clBlack;
  if(FileExists(GetThemeDir()+"////Graphics////Secure.png"))
   SecureImage->Picture->Bitmap->LoadFromFile(GetThemeDir()+"////Graphics////Secure.png");
  else if(FileExists(GetDefaultThemeDir()+"////Graphics////Secure.png"))
   SecureImage->Picture->Bitmap->LoadFromFile(GetDefaultThemeDir()+"////Graphics////Secure.png");
}
//---------------------------------------------------------------------------

void __fastcall TChangePassForm::aExitExecute(TObject *Sender)
{
  Close();
}
//---------------------------------------------------------------------------

void __fastcall TChangePassForm::aChangePassExecute(TObject *Sender)
{
  //Sprawdzenie akualnego hasla
  if(MD5(OldPassEdit->Text)==GetPassword())
  {
	//Sprawdzanie poprawnosci nowego hasla #1
	if((!NewPassEdit->Text.IsEmpty())&&(!ConfirmNewPassEdit->Text.IsEmpty()))
	{
	  //Sprawdzanie poprawnosci nowego hasla #2
	  if(NewPassEdit->Text==ConfirmNewPassEdit->Text)
	  {
		//Zmiana has�a
		SetPassword(NewPassEdit->Text);
		//Zamkniecie formy
		Close();
	  }
	  else Application->MessageBox(L"Nowe has�a si� r�ni�",L"B��d",MB_ICONWARNING);
	}
	else Application->MessageBox(L"Podaj nowe has�a",L"B��d",MB_ICONWARNING);
  }
  else Application->MessageBox(L"B��dne stare has�o",L"B��d",MB_ICONWARNING);
}
//---------------------------------------------------------------------------
