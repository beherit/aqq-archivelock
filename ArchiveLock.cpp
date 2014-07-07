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

#include <vcl.h>
#include <windows.h>
#pragma hdrstop
#pragma argsused
#include <PluginAPI.h>
#include <inifiles.hpp>
#include <IdHashMessageDigest.hpp>
#include "ChangePassFrm.h"
#include "UnlockFrm.h"

int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{
  return 1;
}
//---------------------------------------------------------------------------

//Uchwyt-do-form-------------------------------------------------------------
TChangePassForm *hChangePassForm;
//Struktury-glowne-----------------------------------------------------------
TPluginLink PluginLink;
TPluginInfo PluginInfo;
//Uchwyt-do-okna-archiwum----------------------------------------------------
HWND hFrmArch;
//Uchwyt-do-okna-postepo-----------------------------------------------------
HWND hFrmProgress;
//Uchwyt-do-okna-timera------------------------------------------------------
HWND hTimerFrm;
//Informacja-o-widocznym-oknie-postepu---------------------------------------
bool FrmProgressExists = false;
//Informacja-o-ukryciu-okna-postepu------------------------------------------
bool HideFrmProgress = false;
//Zdefiniowane-haslo---------------------------------------------------------
UnicodeString UserPassword;
//TIMERY---------------------------------------------------------------------
#define TIMER_BLOCK_FRMARCH 10
#define TIMER_CHKFRMPROGRESS 20
//FORWARD-AQQ-HOOKS----------------------------------------------------------
INT_PTR __stdcall OnBeforePluginUnload(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall OnColorChange(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall OnThemeChanged(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall OnWindowEvent(WPARAM wParam, LPARAM lParam);
//FORWARD-TIMER--------------------------------------------------------------
LRESULT CALLBACK TimerFrmProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
//---------------------------------------------------------------------------

//Pobieranie sciezki katalogu prywatnego wtyczek
UnicodeString GetPluginUserDir()
{
  return StringReplace((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_GETPLUGINUSERDIR,0,0), "\\", "\\\\", TReplaceFlags() << rfReplaceAll);
}
//---------------------------------------------------------------------------

//Pobieranie sciezki kompozycji
UnicodeString GetThemeDir()
{
  return StringReplace((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_GETTHEMEDIR,0,0), "\\", "\\\\", TReplaceFlags() << rfReplaceAll);
}
//---------------------------------------------------------------------------
UnicodeString GetDefaultThemeDir()
{
  return StringReplace((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_GETAPPPATH,0,0), "\\", "\\\\", TReplaceFlags() << rfReplaceAll) + "\\\\System\\\\Shared\\\\Themes\\\\Standard";
}
//---------------------------------------------------------------------------

//Pobieranie sciezki skorki kompozycji
UnicodeString GetThemeSkinDir()
{
  return StringReplace((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_GETTHEMEDIR,0,0), "\\", "\\\\", TReplaceFlags() << rfReplaceAll) + "\\\\Skin";
}
//---------------------------------------------------------------------------

//Sprawdzanie czy  wlaczona jest zaawansowana stylizacja okien
bool ChkSkinEnabled()
{
  TStrings* IniList = new TStringList();
  IniList->SetText((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_FETCHSETUP,0,0));
  TMemIniFile *Settings = new TMemIniFile(ChangeFileExt(Application->ExeName, ".INI"));
  Settings->SetStrings(IniList);
  delete IniList;
  UnicodeString SkinsEnabled = Settings->ReadString("Settings","UseSkin","1");
  delete Settings;
  return StrToBool(SkinsEnabled);
}
//---------------------------------------------------------------------------

//Pobieranie ustawien animacji AlphaControls
bool ChkThemeAnimateWindows()
{
  TStrings* IniList = new TStringList();
  IniList->SetText((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_FETCHSETUP,0,0));
  TMemIniFile *Settings = new TMemIniFile(ChangeFileExt(Application->ExeName, ".INI"));
  Settings->SetStrings(IniList);
  delete IniList;
  UnicodeString AnimateWindowsEnabled = Settings->ReadString("Theme","ThemeAnimateWindows","1");
  delete Settings;
  return StrToBool(AnimateWindowsEnabled);
}
//---------------------------------------------------------------------------
bool ChkThemeGlowing()
{
  TStrings* IniList = new TStringList();
  IniList->SetText((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_FETCHSETUP,0,0));
  TMemIniFile *Settings = new TMemIniFile(ChangeFileExt(Application->ExeName, ".INI"));
  Settings->SetStrings(IniList);
  delete IniList;
  UnicodeString GlowingEnabled = Settings->ReadString("Theme","ThemeGlowing","1");
  delete Settings;
  return StrToBool(GlowingEnabled);
}
//---------------------------------------------------------------------------

//Pobieranie ustawien koloru AlphaControls
int GetHUE()
{
  return (int)PluginLink.CallService(AQQ_SYSTEM_COLORGETHUE,0,0);
}
//---------------------------------------------------------------------------
int GetSaturation()
{
  return (int)PluginLink.CallService(AQQ_SYSTEM_COLORGETSATURATION,0,0);
}
//---------------------------------------------------------------------------

//Obliczanie MD5 ciagu znakow
UnicodeString MD5(UnicodeString Text)
{
  TIdHashMessageDigest5* idmd5 = new TIdHashMessageDigest5();
  UnicodeString Result = idmd5->HashStringAsHex(Text.LowerCase()).LowerCase();
  delete idmd5;
  return Result;
}
//---------------------------------------------------------------------------

//Procka okna timera
LRESULT CALLBACK TimerFrmProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  //Notfikacja timera
  if(uMsg==WM_TIMER)
  {
	//Zamykanie okna archiwum
	if(wParam==TIMER_BLOCK_FRMARCH)
	{
	  //Ukrywanie okna archiwum
	  if(IsWindowVisible(hFrmArch))
	  {
		//Zatrzymanie timera
		KillTimer(hTimerFrm,TIMER_BLOCK_FRMARCH);
		//Ukrycie okna
		ShowWindow(hFrmArch,SW_HIDE);
		//Ponowne wlaczenie timera z nowym interwalem
		SetTimer(hTimerFrm,TIMER_BLOCK_FRMARCH,100,(TIMERPROC)TimerFrmProc);
	  }
	  //Zamykanie okna archiwum po wczytaniu danych
	  if((IsWindow(hFrmArch))&&(!FrmProgressExists))
	  {
		//Zatrzymanie timera
		KillTimer(hTimerFrm,TIMER_BLOCK_FRMARCH);
		//Zamkniecie okna archiwum
		PostMessage(hFrmArch, WM_CLOSE, 0, 0);
	  }
	}
	//Szukanie okna postepu
	else if(wParam==TIMER_CHKFRMPROGRESS)
	{
	  //Ukrywanie okna postepu
	  if(IsWindowVisible(hFrmProgress))
	  {
		//Zatrzymanie timera
		KillTimer(hTimerFrm,TIMER_CHKFRMPROGRESS);
		//Ukrycie okna
		ShowWindow(hFrmProgress,SW_HIDE);
		//Ponowne wlaczenie timera z nowym interwalem
		SetTimer(hTimerFrm,TIMER_CHKFRMPROGRESS,100,(TIMERPROC)TimerFrmProc);
	  }
	  //Okno postepu zostalo zamkniete
	  if(!IsWindow(hFrmProgress))
	  {
		//Zatrzymanie timera
		KillTimer(hTimerFrm,TIMER_CHKFRMPROGRESS);
		//Odznaczenie zamkniecia okna postepu
		FrmProgressExists = false;
      }
    }

	return 0;
  }

  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
//---------------------------------------------------------------------------

//Hook na wyladowanie wtyczki przez usera
INT_PTR __stdcall OnBeforePluginUnload(WPARAM wParam, LPARAM lParam)
{
  //Pobieranie uchwytu wtyczki do wyladowania
  HINSTANCE hinst = (HINSTANCE)wParam;
  //Zlecono wyladowanie wtyczki ArchiveLock
  if(hinst==HInstance)
  {
    //Przypisanie uchwytu do formy
	Application->Handle = (HWND)UnlockForm;
	TUnlockForm *hUnlockForm = new TUnlockForm(Application);
	//Ustawienie danych na formie
	hUnlockForm->Caption = "Zabezpieczone wy³¹czenie wtyczki";
	hUnlockForm->PassEdit->BoundLabel->Caption = "Podaj has³o, aby wy³¹czyæ wtyczkê:";
	//Pokaznie okna
	hUnlockForm->ShowModal();
	//Pobranie informacji o prawidlowosci hasla
	bool Unlocked = hUnlockForm->Unlocked;
	//Usuniecie uchwytu do formy
	delete hUnlockForm;
	//Zablokowanie wyladowania wtyczki
	if(!Unlocked) return 1;
  }

  return 0;
}
//---------------------------------------------------------------------------

//Hook na zmiane kolorystyki AlphaControls
INT_PTR __stdcall OnColorChange(WPARAM wParam, LPARAM lParam)
{
  //Okno zmiany hasla zostalo juz stworzone
  if(hChangePassForm)
  {
	//Wlaczona zaawansowana stylizacja okien
	if(ChkSkinEnabled())
	{
	  hChangePassForm->sSkinManager->HueOffset = wParam;
	  hChangePassForm->sSkinManager->Saturation = lParam;
	}
  }

  return 0;
}
//---------------------------------------------------------------------------

//Hook na zmiane kompozycji
INT_PTR __stdcall OnThemeChanged(WPARAM wParam, LPARAM lParam)
{
  //Okno zmiany hasla zostalo juz stworzone
  if(hChangePassForm)
  {
    //Wlaczona zaawansowana stylizacja okien
	if(ChkSkinEnabled())
	{
	  //Pobieranie sciezki nowej aktywnej kompozycji
	  UnicodeString ThemeDir = StringReplace((wchar_t*)lParam, "\\", "\\\\", TReplaceFlags() << rfReplaceAll);
	  UnicodeString ThemeSkinDir = ThemeDir + "\\\\Skin";
	  //Plik zaawansowanej stylizacji okien istnieje
	  if(FileExists(ThemeSkinDir + "\\\\Skin.asz"))
	  {
		//Dane pliku zaawansowanej stylizacji okien
		ThemeSkinDir = StringReplace(ThemeSkinDir, "\\\\", "\\", TReplaceFlags() << rfReplaceAll);
		hChangePassForm->sSkinManager->SkinDirectory = ThemeSkinDir;
		hChangePassForm->sSkinManager->SkinName = "Skin.asz";
		//Ustawianie animacji AlphaControls
		if(ChkThemeAnimateWindows()) hChangePassForm->sSkinManager->AnimEffects->FormShow->Time = 200;
		else hChangePassForm->sSkinManager->AnimEffects->FormShow->Time = 0;
		hChangePassForm->sSkinManager->Effects->AllowGlowing = ChkThemeGlowing();
		//Zmiana kolorystyki AlphaControls
		hChangePassForm->sSkinManager->HueOffset = GetHUE();
		hChangePassForm->sSkinManager->Saturation = GetSaturation();
		//Aktywacja skorkowania AlphaControls
		hChangePassForm->sSkinManager->Active = true;
	  }
	  //Brak pliku zaawansowanej stylizacji okien
	  else hChangePassForm->sSkinManager->Active = false;
	}
	//Zaawansowana stylizacja okien wylaczona
	else hChangePassForm->sSkinManager->Active = false;
  }

  return 0;
}
//---------------------------------------------------------------------------

//Hook na zamkniecie/otwarcie okien
INT_PTR __stdcall OnWindowEvent(WPARAM wParam, LPARAM lParam)
{
  //Pobranie informacji o oknie i eventcie
  TPluginWindowEvent WindowEvent = *(PPluginWindowEvent)lParam;
  int Event = WindowEvent.WindowEvent;
  UnicodeString ClassName = (wchar_t*)WindowEvent.ClassName;

  //Otworzenie okna archiwum
  if((ClassName=="TfrmArch")&&(Event==WINDOW_EVENT_CREATE))
  {
	//Przypisanie uchwytu do formy
	Application->Handle = (HWND)UnlockForm;
	TUnlockForm *hUnlockForm = new TUnlockForm(Application);
	//Pokaznie okna
	hUnlockForm->ShowModal();
	//Zablokowanie dostepu do archiwum
	if(!hUnlockForm->Unlocked)
	{
	  //Przypisanie uchwytu do okna archiwum
	  hFrmArch = (HWND)(int)WindowEvent.Handle;
	  //Ukrywanie okna postepu
	  HideFrmProgress = true;
	  //Wlaczenie timera zamykania okna archiwum
	  SetTimer(hTimerFrm,TIMER_BLOCK_FRMARCH,10,(TIMERPROC)TimerFrmProc);
	}
	//Nie ukrywanie okna postepu
	else
	 HideFrmProgress = false;
	//Usuniecie uchwytu do formy
	delete hUnlockForm;
  }
  //Zamkniecie okna archiwum
  if((ClassName=="TfrmArch")&&(Event==WINDOW_EVENT_CLOSE))
  {
    //Przypisanie uchwytu do okna archiwum
	HWND hwnd = (HWND)(int)WindowEvent.Handle;
	//Ukrycie okna
	ShowWindow(hwnd,SW_HIDE);
  }

  //Otworzenie okna postepu
  if((ClassName=="TfrmProgress")&&(Event==WINDOW_EVENT_CREATE))
  {
	//Odznaczenie utworzenia okna postepu
	FrmProgressExists = true;
	//Ukrywanie okna postepu
	if(HideFrmProgress)
	{
	  //Przypisanie uchwytu do okna postepu
	  hFrmProgress = (HWND)(int)WindowEvent.Handle;
	  //Wlaczenie timera szukania okna postepu
	  SetTimer(hTimerFrm,TIMER_CHKFRMPROGRESS,10,(TIMERPROC)TimerFrmProc);
	}
  }
  //Zamkniecie okna postepu
  if((ClassName=="TfrmProgress")&&(Event==WINDOW_EVENT_CLOSE))
   //Odznaczenie usuniecia okna postepu
   FrmProgressExists = false;

  return 0;
}
//---------------------------------------------------------------------------

//Odczyt ustawien
void LoadSettings()
{
  TStrings* IniList = new TStringList();
  IniList->SetText((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_FETCHSETUP,0,0));
  TMemIniFile *Settings = new TMemIniFile(ChangeFileExt(Application->ExeName, ".INI"));
  Settings->SetStrings(IniList);
  delete IniList;
  UserPassword = Settings->ReadString("ArchiveLock","Password","dce7c4174ce9323904a934a486c41288");
  delete Settings;
}
//---------------------------------------------------------------------------

//Pobranie aktualnego hasla
UnicodeString GetPassword()
{
  return UserPassword;
}
//---------------------------------------------------------------------------

//Zapis nowego hasla w ustawieniach
void SetPassword(UnicodeString Password)
{
  //Nowe ustawienia
  TSaveSetup SaveSetup;
  SaveSetup.Section = L"ArchiveLock";
  SaveSetup.Ident = L"Password";
  SaveSetup.Value = MD5(Password).w_str();
  //Zapis ustawien
  PluginLink.CallService(AQQ_FUNCTION_SAVESETUP,0,(LPARAM)(&SaveSetup));
  //Zapisywanie hasla do zmiennej
  UserPassword = MD5(Password).w_str();
}
//---------------------------------------------------------------------------

//Zapisywanie zasobow
void ExtractRes(wchar_t* FileName, wchar_t* ResName, wchar_t* ResType)
{
  TPluginTwoFlagParams PluginTwoFlagParams;
  PluginTwoFlagParams.cbSize = sizeof(TPluginTwoFlagParams);
  PluginTwoFlagParams.Param1 = ResName;
  PluginTwoFlagParams.Param2 = ResType;
  PluginTwoFlagParams.Flag1 = (int)HInstance;
  PluginLink.CallService(AQQ_FUNCTION_SAVERESOURCE,(WPARAM)&PluginTwoFlagParams,(LPARAM)FileName);
}
//---------------------------------------------------------------------------

//Obliczanie sumy kontrolnej pliku
UnicodeString MD5File(UnicodeString FileName)
{
  if(FileExists(FileName))
  {
	UnicodeString Result;
    TFileStream *fs;

	fs = new TFileStream(FileName, fmOpenRead | fmShareDenyWrite);
	try
	{
	  TIdHashMessageDigest5 *idmd5= new TIdHashMessageDigest5();
	  try
	  {
	    Result = idmd5->HashStreamAsHex(fs);
	  }
	  __finally
	  {
	    delete idmd5;
	  }
    }
	__finally
    {
	  delete fs;
    }

    return Result;
  }
  else return 0;
}
//---------------------------------------------------------------------------

extern "C" INT_PTR __declspec(dllexport) __stdcall Load(PPluginLink Link)
{
  //Linkowanie wtyczki z komunikatorem
  PluginLink = *Link;
  //Pobranie sciezki do katalogu prywatnego uzytkownika
  UnicodeString PluginUserDir = GetPluginUserDir();
  //Wypakiwanie ikonki ArchiveLock.dll.png
  //167BFD9111AD9E880753A41D531BD622
  if(!DirectoryExists(PluginUserDir + "\\\\Shared"))
   CreateDir(PluginUserDir + "\\\\Shared");
  if(!FileExists(PluginUserDir + "\\\\Shared\\\\ArchiveLock.dll.png"))
   ExtractRes((PluginUserDir + "\\\\Shared\\\\ArchiveLock.dll.png").w_str(),L"SHARED",L"DATA");
  else if(MD5File(PluginUserDir + "\\\\Shared\\\\ArchiveLock.dll.png")!="167BFD9111AD9E880753A41D531BD622")
   ExtractRes((PluginUserDir + "\\\\Shared\\\\ArchiveLock.dll.png").w_str(),L"SHARED",L"DATA");
  //Hook na wyladowanie wtyczki przez usera
  PluginLink.HookEvent(AQQ_SYSTEM_PLUGIN_BEFOREUNLOAD,OnBeforePluginUnload);
  //Hook na zmiane kolorystyki AlphaControls
  PluginLink.HookEvent(AQQ_SYSTEM_COLORCHANGE,OnColorChange);
  //Hook na zmiane kompozycji
  PluginLink.HookEvent(AQQ_SYSTEM_THEMECHANGED, OnThemeChanged);
  //Hook na zamkniecie/otwarcie okien
  PluginLink.HookEvent(AQQ_SYSTEM_WINDOWEVENT,OnWindowEvent);
  //Wczytanie ustawien
  LoadSettings();
  //Rejestowanie klasy okna timera
  WNDCLASSEX wincl;
  wincl.cbSize = sizeof (WNDCLASSEX);
  wincl.style = 0;
  wincl.lpfnWndProc = TimerFrmProc;
  wincl.cbClsExtra = 0;
  wincl.cbWndExtra = 0;
  wincl.hInstance = HInstance;
  wincl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
  wincl.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
  wincl.lpszMenuName = NULL;
  wincl.lpszClassName = L"TArchiveLockTimer";
  wincl.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
  RegisterClassEx(&wincl);
  //Tworzenie okna timera
  hTimerFrm = CreateWindowEx(0, L"TArchiveLockTimer", L"",	0, 0, 0, 0, 0, NULL, NULL, HInstance, NULL);

  return 0;
}
//---------------------------------------------------------------------------

extern "C" INT_PTR __declspec(dllexport) __stdcall Unload()
{
  //Wyladowanie timerow
  for(int TimerID=10;TimerID<=20;TimerID=TimerID+10) KillTimer(hTimerFrm,TimerID);
  //Usuwanie okna timera
  DestroyWindow(hTimerFrm);
  //Wyrejestowanie klasy okna timera
  UnregisterClass(L"TArchiveLockTimer",HInstance);
  //Wyladowanie wszystkich hookow
  PluginLink.UnhookEvent(OnBeforePluginUnload);
  PluginLink.UnhookEvent(OnColorChange);
  PluginLink.UnhookEvent(OnThemeChanged);
  PluginLink.UnhookEvent(OnWindowEvent);

  return 0;
}
//---------------------------------------------------------------------------

//Ustawienia wtyczki
extern "C" INT_PTR __declspec(dllexport)__stdcall Settings()
{
  //Przypisanie uchwytu do formy zmiany hasla
  if(!hChangePassForm)
  {
	Application->Handle = (HWND)ChangePassForm;
	hChangePassForm = new TChangePassForm(Application);
  }
  //Pokaznie okna zmiany hasla
  hChangePassForm->Show();

  return 0;
}
//---------------------------------------------------------------------------

//Informacje o wtyczce
extern "C" PPluginInfo __declspec(dllexport) __stdcall AQQPluginInfo(DWORD AQQVersion)
{
  PluginInfo.cbSize = sizeof(TPluginInfo);
  PluginInfo.ShortName = L"ArchiveLock";
  PluginInfo.Version = PLUGIN_MAKE_VERSION(1,0,2,0);
  PluginInfo.Description = L"Wtyczka blokuje dostêp do archiwum rozmów na has³o. \n\nDomyœlne has³o to \"lock\".";
  PluginInfo.Author = L"Krzysztof Grochocki";
  PluginInfo.AuthorMail = L"kontakt@beherit.pl";
  PluginInfo.Copyright = L"Krzysztof Grochocki";
  PluginInfo.Homepage = L"http://beherit.pl";
  PluginInfo.Flag = 0;
  PluginInfo.ReplaceDefaultModule = 0;

  return &PluginInfo;
}
//---------------------------------------------------------------------------
