// SimpleCLI.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include "pch.h"

#pragma comment(lib, "shlwapi.lib")

int main( int argc, char** argv )
{
	if( argc > 1 )
	{
		auto threadID = GetCurrentThreadId();
		auto result = PostThreadMessage( threadID, WM_USER, 0, 0 );
		auto lastError = GetLastError();
		std::cout << "PostThreadMessage()=" << result << ":LastError=" << lastError << std::endl;
		Sleep( 1000 );
		std::cout << "Ending " << argv[1] << std::endl;
	}
	else
	{
		wchar_t modulePath[MAX_PATH];
		GetModuleFileName( nullptr, modulePath, MAX_PATH );
		lstrcpy( PathFindFileName( modulePath ), L"SimpleGUI.exe" );
		STARTUPINFO si ={ 0 };
		PROCESS_INFORMATION pi ={ 0 };
		si.cb = sizeof( si );
		std::unique_ptr<wchar_t> ptr( _wcsdup( std::format( L"{0} SecondProcess", modulePath ).data() ) );
		auto result = CreateProcess( nullptr, ptr.get(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi );
		if( result )
		{
			std::cout << "Call WaitForInputIdle()" << std::endl;
			auto waitResult = WaitForInputIdle( pi.hProcess, INFINITE );
			std::cout << "Return WaitForInputIdle()" << std::endl;
			CloseHandle( pi.hThread );
			CloseHandle( pi.hProcess );
		}
	}
}
