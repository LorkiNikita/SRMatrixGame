// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "stdafx.h"
#include "MatrixGameDll.hpp"
#include "MatrixGame.h"
#include "MatrixMap.hpp"
#include "MatrixFormGame.hpp"
#include "MatrixCamera.hpp"
#include "Interface/CInterface.h"

#include <stdio.h>
#include <time.h>

#include <windows.h>
#include <excpt.h>
#include <DbgHelp.h>
#pragma comment(lib, "dbghelp.lib")

SMGDRobotInterface g_RobotInterface;
SMGDRangersInterface* g_RangersInterface = nullptr;
int g_ExitState = ES_EXIT_TO_WINDOWS;

int GetFilePathB(OUT char* path, IN int len, IN const char* filename)
{
    if(!len) return 0;

    --len; // rezerv for null terminator

    if(len < 0) len = MAX_PATH;

    int ln = lstrlen(filename);
    int pos = ln - 1;
    bool found = false;
    while(pos >= 0)
    {
        if(filename[pos] == '\\') found = true;
        else if(found) break;
        --pos;
    }

    if(pos < 0) pos = ln - 1;

    ++pos;
    if(len > pos) len = pos;

    if(filename != path) memcpy(path, filename, len);

    path[len] = '\0';

    return len;
}


long __stdcall ExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo)
{
  /*char path[MAX_PATH];
  GetModuleFileNameA(nullptr, path, sizeof(path));
  GetFilePathB(path, sizeof(path), path);

  SYSTEMTIME time;
  GetLocalTime(&time);

  char fn[MAX_PATH];
  sprintf_s(fn, MAX_PATH,
            "%s\\crash_dump_%04d-%02d-%02d_%02d-%02d-%02d.dmp",
            path,
            time.wYear, time.wMonth, time.wDay,
            time.wHour, time.wMinute, time.wSecond);

  HANDLE hFile = CreateFileA(fn,
                             GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             nullptr,
                             CREATE_ALWAYS,
                             FILE_ATTRIBUTE_NORMAL,
                             nullptr);

  if (hFile != INVALID_HANDLE_VALUE)
  {
    MINIDUMP_EXCEPTION_INFORMATION info;

    info.ThreadId = GetCurrentThreadId();
    info.ExceptionPointers = pExceptionInfo;
    info.ClientPointers = FALSE;

    if (FALSE == MiniDumpWriteDump(GetCurrentProcess(),
                                   GetCurrentProcessId(),
                                   hFile,
                                   MiniDumpWithFullMemory,
                                   &info,
                                   nullptr,
                                   nullptr))
    {
      //DebugPrint("Не удалось записать файл дампа '%'\n", fn);
    }

    CloseHandle(hFile);
  }
  else
  {
    //
    //DebugPrint("Не удалось создать файл дампа '%'\n", fn);
  }
  */
  return EXCEPTION_EXECUTE_HANDLER;
}

BOOL APIENTRY DllMain( HANDLE hModule, 
                       dword  ul_reason_for_call, 
                       LPVOID lpReserved
                     )
{
	SetUnhandledExceptionFilter(ExceptionHandler);
    switch(ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

void __stdcall Init(SMGDRangersInterface* ri)
{
    g_RangersInterface = ri;
}

void __stdcall Deinit()
{
    g_RangersInterface = nullptr;
}

int __stdcall Support()
{

    //g_D3D = Direct3DCreate9(D3D_SDK_VERSION);
    g_D3D = Direct3DCreate9(31);
    
    if(g_D3D == nullptr) return SUPE_DIRECTX;


    D3DCAPS9 caps;
    if(D3D_OK != g_D3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps))
    {
        g_D3D->Release();
        g_D3D = nullptr;
        return SUPE_DIRECTX;
    }

    if(caps.MaxSimultaneousTextures < 2) return SUPE_VIDEOHARDWARE;

    if(caps.MaxTextureWidth < 2048) return SUPE_VIDEOHARDWARE;
    if(caps.MaxTextureHeight < 2048) return SUPE_VIDEOHARDWARE;
    
    if(caps.MaxStreams == 0) return SUPE_VIDEODRIVER;
    g_D3D->Release();
    g_D3D = nullptr;
    return SUPE_OK;
}

// 0-exit to windows (terminate)
// 1-exit to main menu
// 2-loss
// 3-win

void protectedBlock1(CFormMatrixGame* formgame)
{
	__try {
		g_ExitState = ES_EXIT_TO_WINDOWS;
		FormChange(formgame);

		timeBeginPeriod(1);

		SETFLAG(g_Flags, GFLAG_APPACTIVE);
		RESETFLAG(g_Flags, GFLAG_EXITLOOP);
		
		L3GRun();
	}
    //Если случился краш
	__except(EXCEPTION_EXECUTE_HANDLER/*ExceptionHandler(GetExceptionInformation())*/)
    {
        SETFLAG(g_Flags, GFLAG_EXITLOOP);
        g_ExitState += 100;
    }
}

void protectedBlock2(CFormMatrixGame* formgame, SRobotGameState* rgs)
{
	__try {
		rgs->m_Time=g_MatrixMap->GetPlayerSide()->GetStatValue(STAT_TIME);
        rgs->m_BuildRobot=g_MatrixMap->GetPlayerSide()->GetStatValue(STAT_ROBOT_BUILD);
        rgs->m_KillRobot=g_MatrixMap->GetPlayerSide()->GetStatValue(STAT_ROBOT_KILL);
        rgs->m_BuildTurret=g_MatrixMap->GetPlayerSide()->GetStatValue(STAT_TURRET_BUILD);
        rgs->m_KillTurret=g_MatrixMap->GetPlayerSide()->GetStatValue(STAT_TURRET_KILL);
        rgs->m_KillBuilding=g_MatrixMap->GetPlayerSide()->GetStatValue(STAT_BUILDING_KILL);
	}
	__except(EXCEPTION_EXECUTE_HANDLER){}

	__try {
        timeEndPeriod(1);

        MatrixGameDeinit();
	}
	__except(EXCEPTION_EXECUTE_HANDLER){}

	__try {
        FormChange(nullptr);
        HDelete(CFormMatrixGame, formgame, nullptr);
	}
	__except(EXCEPTION_EXECUTE_HANDLER){}


	__try {
        g_Cache->Clear();
	}
	__except(EXCEPTION_EXECUTE_HANDLER){}


	__try {
        L3GDeinit();
	}
	__except(EXCEPTION_EXECUTE_HANDLER){}


	__try {
        CacheDeinit();
	}
	__except(EXCEPTION_EXECUTE_HANDLER){}


#ifdef MEM_SPY_ENABLE
	__try {
        CMain::BaseDeInit();
	}
	__except(EXCEPTION_EXECUTE_HANDLER){}
#endif
}

int __stdcall Run(HINSTANCE hinst, HWND hwnd, wchar* map, SRobotsSettings* set, wchar* lang, wchar* txt_start, wchar* txt_win, wchar* txt_loss, wchar* planet, SRobotGameState* rgs)
{
    //try {

    srand((unsigned)time(nullptr));
    SetMaxCameraDistance(set->m_MaxDistance);
    MatrixGameInit(hinst, hwnd, map, set, lang, txt_start, txt_win, txt_loss, planet);
    CFormMatrixGame* formgame = HNew(nullptr) CFormMatrixGame();

    protectedBlock1(formgame);
    protectedBlock2(formgame, rgs);


    /*
    FormChange(formgame);
    timeBeginPeriod(1);
    SETFLAG(g_Flags, GFLAG_APPACTIVE);
    RESETFLAG(g_Flags, GFLAG_EXITLOOP);

    L3GRun();
    */


    /*
    rgs->m_Time=g_MatrixMap->GetPlayerSide()->GetStatValue(STAT_TIME);
    rgs->m_BuildRobot=g_MatrixMap->GetPlayerSide()->GetStatValue(STAT_ROBOT_BUILD);
    rgs->m_KillRobot=g_MatrixMap->GetPlayerSide()->GetStatValue(STAT_ROBOT_KILL);
    rgs->m_BuildTurret=g_MatrixMap->GetPlayerSide()->GetStatValue(STAT_TURRET_BUILD);
    rgs->m_KillTurret=g_MatrixMap->GetPlayerSide()->GetStatValue(STAT_TURRET_KILL);
    rgs->m_KillBuilding=g_MatrixMap->GetPlayerSide()->GetStatValue(STAT_BUILDING_KILL);
    timeEndPeriod(1);
    MatrixGameDeinit();
    FormChange(nullptr);
    HDelete(CFormMatrixGame, formgame, nullptr);
    g_Cache->Clear();
    L3GDeinit();
    CacheDeinit();
#ifdef MEM_SPY_ENABLE
        CMain::BaseDeInit();
#endif
        //}
     */




        /*
        catch(CException* ex)
        {
    #ifdef ENABLE_HISTORY
            CDebugTracer::SaveHistory();
    #endif
            g_Cache->Clear();
            L3GDeinit();
            CStr exs(ex->Info());
            CBuf exb;
            exb.StrNZ(exs);
            char lpPath[MAX_PATH];
            strcpy(lpPath,PathToOutputFiles("exception.txt"));
            size_t size = strlen(lpPath) + 1;
            wchar_t* wa = new wchar_t[size];
            mbstowcs(wa,lpPath,size);
            exb.SaveInFile(wa);//L"exception.txt");
            MessageBox(nullptr,exs.Get(),"Exception:",MB_OK);
            delete ex;
        }
        catch(...)
        {
            ClipCursor(nullptr);
    #ifdef ENABLE_HISTORY
            MessageBox(nullptr,"Unknown bug (history has saved) :(", "Exception:", MB_OK);
            CDebugTracer::SaveHistory();
    #else
            MessageBox(nullptr,"Dont panic! This is just unknown bug happens."
                "\nCode date: " __DATE__

                ,"Game crashed :(",MB_OK);
    #endif
        }
        */
        //__except(EXCEPTION_CONTINUE_EXECUTION){}

    ClipCursor(nullptr);

    if(FLAG(g_Flags, GFLAG_EXITLOOP))
    {
        int error = g_ExitState >= 100 ? 100 : 0;
        switch(g_ExitState % 100)
        {
            case ES_EXIT_TO_WINDOWS: return 0 + error; //Полный выход из игры (если игра ловит такой маркер (100), предлагает засчитать победу или дропнуть игру)
            case ES_EXIT_TO_MAIN_MENU: return 1 + error; //Выход в главное меню игры
            case ES_EXIT_AFTER_WIN: return 3 + error; //Победа
            case ES_EXIT_AFTER_LOSE: return 2 + error; //Поражение
            case ES_EXIT_AFTER_SURRENDER: return 2 + error;
            default: return 100; //Если в g_ExitState невалид, засчитываем как обычный краш
        }
    }
    return 0;
}

MATRIXGAMEDLL_API SMGDRobotInterface* __cdecl GetRobotInterface(void)
{
    ZeroMemory(&g_RobotInterface, sizeof(SMGDRobotInterface));
    g_RobotInterface.m_Init = &Init;
    g_RobotInterface.m_Deinit = &Deinit;
    g_RobotInterface.m_Support = &Support;
    g_RobotInterface.m_Run = &Run;
    return &g_RobotInterface;
}