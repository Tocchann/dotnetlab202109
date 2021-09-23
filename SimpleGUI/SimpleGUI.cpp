// SimpleGUI.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "pch.h"
#include "SimpleGUI.h"

#pragma comment(lib,"comctl32.lib")
#pragma comment(lib, "shlwapi.lib")

#ifdef _DEBUG
# define new DEBUG_NEW
#endif

#define MAX_LOADSTRING 100

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM                MyRegisterClass( HINSTANCE hInstance, LPCWSTR );
BOOL                InitInstance( HINSTANCE, int, LPCWSTR, LPCWSTR );
LRESULT CALLBACK    WndProc( HWND, UINT, WPARAM, LPARAM );
INT_PTR CALLBACK    About( HWND, UINT, WPARAM, LPARAM );

// --追加--
std::map<UINT, std::function<void( HWND, int, HWND, UINT )>> g_commandHandler;
bool g_dispQueueStatus = false;

void APIENTRY RegisterCommandMessage( HINSTANCE hInstance );
int APIENTRY MessageLoop( HACCEL hAccelTable );
void APIENTRY DumpQueueStatus( LPCWSTR prefix );
void APIENTRY TestPostThreadMessage( LPCWSTR prefix );
void TestSubThreadAction( HWND hwnd );
void APIENTRY DispVersion( HWND hwnd, int cmdId );
void APIENTRY TestWaitInputIdle( HWND hwnd, LPCWSTR execFileName );
// --追加--

int APIENTRY wWinMain( _In_ HINSTANCE hInstance,
					   _In_opt_ HINSTANCE hPrevInstance,
					   _In_ LPWSTR    lpCmdLine,
					   _In_ int       nCmdShow )
{
	_CrtSetDbgFlag( _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG )| _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	lstrcpy( new wchar_t[32], L"First Leak" );
	UNREFERENCED_PARAMETER( hPrevInstance );
	UNREFERENCED_PARAMETER( lpCmdLine );

	DumpQueueStatus( L"First\n" );

#if 0
	TestPostThreadMessage( L"MainThread" );
#endif

	// TODO: ここにコードを挿入してください。

	//	COMCTl32.DLL の V6 をリンクしておかないとビジュアルスタイルが有効にならない
	INITCOMMONCONTROLSEX iccx ={ sizeof( INITCOMMONCONTROLSEX ), ICC_WIN95_CLASSES };
	InitCommonControlsEx( &iccx );

	// グローバル文字列を初期化する
	wchar_t title[MAX_LOADSTRING];
	wchar_t wndclsName[MAX_LOADSTRING];
	LoadStringW( hInstance, IDS_APP_TITLE, title, MAX_LOADSTRING );
	LoadStringW( hInstance, IDC_SIMPLEGUI, wndclsName, MAX_LOADSTRING );

	MyRegisterClass( hInstance, wndclsName );
	RegisterCommandMessage( hInstance );

	if( __argc > 1 )
	{
		std::wstring str;
#if 1
		MSG msg;
		auto result = PeekMessage( &msg, nullptr, 0, 0, PM_NOREMOVE );
		if( result )
		{
			str = std::format( L"メッセージあり:%d", msg.message );
		}
		else
		{
			str = L"メッセージなし";
		}
#else
		auto result = MsgWaitForMultipleObjects( 0, nullptr, FALSE, 1000, 0 );
		switch( result )
		{
		case WAIT_TIMEOUT:	str = L"WAIT_TIMEOUT"; break;
		case WAIT_FAILED:	str = L"WAIT_FAILED"; break;
		case WAIT_OBJECT_0:	str = L"WAIT_OBJECT_0"; break;
		default:			str = std::format( L"other{0}", result ); break;
		}
#endif
		MessageBox( nullptr, str.data(), title, MB_OK );
	}
	// アプリケーション初期化の実行:
	if( !InitInstance( hInstance, nCmdShow, title, wndclsName ) )
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators( hInstance, MAKEINTRESOURCE( IDC_SIMPLEGUI ) );

	//	２回目呼び出しは、ウィンドウを作成したら１秒待つ
	if( __argc > 1 )
	{
		Sleep( 1000 );
	}
	return MessageLoop( hAccelTable );
}
//	モードレスダイアログがないので比較的シンプルな実装
int APIENTRY MessageLoop( HACCEL hAccelTable )
{
	DumpQueueStatus( L"Pre GetMessage()\n" );
	// メイン メッセージ ループ:
	MSG msg;
	while( GetMessage( &msg, nullptr, 0, 0 ) )
	{
		DumpQueueStatus( L"Post GetMessage()\n" );
		if( !TranslateAccelerator( msg.hwnd, hAccelTable, &msg ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		DumpQueueStatus( L"Post DispatchMessage()\n" );
	}
	DumpQueueStatus( L"Loopout\n" );
	return static_cast<int>(msg.wParam);
}
// 最小のメッセージループ
int MinimumMessageLoop()
{
  MSG msg;
  // メッセージキューからメッセージを取得
  while( GetMessage( &msg, nullptr, 0, 0 ) )
  {
    // (お呪い)入力メッセージを変換
    TranslateMessage( &msg );
    // msg.hWnd のプロシージャにメッセージを処理させる
    DispatchMessage( &msg );
  }
  return static_cast<int>(msg.wParam);
}


//
//  関数: MyRegisterClass()
//
//  目的: ウィンドウ クラスを登録します。
//
ATOM MyRegisterClass( HINSTANCE hInstance, LPCWSTR wndclsName )
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof( WNDCLASSEX );

	wcex.style          = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc    = WndProc;
	wcex.cbClsExtra     = 0;
	wcex.cbWndExtra     = 0;
	wcex.hInstance      = hInstance;
	wcex.hIcon          = LoadIcon( hInstance, MAKEINTRESOURCE( IDI_SIMPLEGUI ) );
	wcex.hCursor        = LoadCursor( nullptr, IDC_ARROW );
	wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName   = MAKEINTRESOURCEW( IDC_SIMPLEGUI );
	wcex.lpszClassName  = wndclsName;
	wcex.hIconSm        = LoadIcon( wcex.hInstance, MAKEINTRESOURCE( IDI_SMALL ) );

	return RegisterClassExW( &wcex );
}
void APIENTRY RegisterCommandMessage( HINSTANCE hInstance )
{
	// コマンドハンドラを登録
	g_commandHandler[IDM_ABOUT] = [hInstance]( HWND hWnd, int, HWND, UINT )
	{
		DialogBox( hInstance, MAKEINTRESOURCE( IDD_ABOUTBOX ), hWnd, About );
	};
	g_commandHandler[ID_CREATE_THREAD] = []( HWND hWnd, int, HWND, UINT )
	{
		TestSubThreadAction( hWnd );
	};
	auto handler = []( HWND hWnd, int cmdID, HWND, UINT )
	{
		DispVersion( hWnd, cmdID );
	};
	g_commandHandler[ID_GET_VERSION_EX] = handler;
	g_commandHandler[ID_RTL_GET_VERSION] = handler;
	g_commandHandler[ID_VERIFY_VERSION_INFO] = handler;
	g_commandHandler[ID_WAIT_CUI] = []( HWND hWnd, int, HWND, UINT )
	{
		TestWaitInputIdle( hWnd, L"SimpleCLI.exe" );
	};
	g_commandHandler[ID_WAIT_GUI] = []( HWND hWnd, int, HWND, UINT )
	{
		TestWaitInputIdle( hWnd, L"SimpleGUI.exe" );
	};
	g_commandHandler[ID_WAIT_NET_RUNTIME] = []( HWND hWnd, int, HWND, UINT )
	{
		TestWaitInputIdle( hWnd, L"windowsdesktop-runtime-5.0.9-win-x64.exe" );
	};
	g_commandHandler[IDM_EXIT] = []( HWND hWnd, int, HWND, UINT )
	{
		DestroyWindow( hWnd );
	};
	g_commandHandler[ID_DISP_QUEUE_STATUS] =[]( HWND , int, HWND, UINT )
	{
		g_dispQueueStatus = !g_dispQueueStatus;
	};
}
//
//   関数: InitInstance(HINSTANCE, int)
//
//   目的: インスタンス ハンドルを保存して、メイン ウィンドウを作成します
//
//   コメント:
//
//        この関数で、グローバル変数でインスタンス ハンドルを保存し、
//        メイン プログラム ウィンドウを作成および表示します。
//
static BOOL CALLBACK MonitorProc( HMONITOR hMonitor, HDC hDC, LPRECT lpRect, LPARAM lParam )
{
	std::list<MONITORINFO>& monitorInfos = *reinterpret_cast<std::list<MONITORINFO>*>(lParam);
	MONITORINFO info ={ 0 };
	info.cbSize = sizeof( info );
	if( GetMonitorInfo( hMonitor, &info ) )
	{
		monitorInfos.push_back( info );
	}
	return TRUE;
}
BOOL InitInstance( HINSTANCE hInstance, int nCmdShow, LPCWSTR title, LPCWSTR wndclsName )
{
	bool isFirstInstance = __argc < 2;
	//	モニター１のセンターに来るようにセットアップする
	std::list<MONITORINFO> monitorInfos;
	EnumDisplayMonitors( nullptr, nullptr, MonitorProc, reinterpret_cast<LPARAM>(&monitorInfos) );
	int x, y;
	int cx, cy;
	//	Enumが失敗した場合
	if( monitorInfos.empty() )
	{
		x = y = 0;
		cx = GetSystemMetrics( SM_CXSCREEN );
		cy = GetSystemMetrics( SM_CYSCREEN );
	}
	//	１つ目のモニターに配置する
	else
	{
		const auto& info = monitorInfos.front();
		//	ワークエリア(タスクバーを除いた領域)の中央に位置するように配置する
		x = info.rcWork.left;
		y = info.rcWork.top;
		cx = info.rcWork.right-info.rcWork.left;
		cy = info.rcWork.bottom-info.rcWork.top;
	}
	//	デバッグ時に画面を見やすくするための位置ずらし
	x += (cx-640)/2 - 320;	//	中央より左側に設置
	y += (cy-480);			//	下に寄せる
	//	メッセージボックスが隠れてしまわないようにウィンドウサイズ分右にずらして表示(前のインスタンスの位置は考えない)
	if( !isFirstInstance )
	{
		x += 640;
	}
	HWND hWnd = CreateWindowW( wndclsName, title, WS_OVERLAPPEDWINDOW,
							   x, y, 640, 480, nullptr, nullptr, hInstance, nullptr );

	if( !hWnd )
	{
		return FALSE;
	}
	DumpQueueStatus( L"After CreateWindow()\n" );

	ShowWindow( hWnd, nCmdShow );
	DumpQueueStatus( L"After ShowWindow()\n" );

	if( isFirstInstance )
	{
		UpdateWindow( hWnd );
		DumpQueueStatus( L"After UpdateWindow()\n" );
	}

	return TRUE;
}

//
//  関数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: メイン ウィンドウのメッセージを処理します。
//
//  WM_COMMAND  - アプリケーション メニューの処理
//  WM_PAINT    - メイン ウィンドウを描画する
//  WM_DESTROY  - 中止メッセージを表示して戻る
//
//
/* void Cls_OnInitMenuPopup(HWND hwnd, HMENU hMenu, UINT item, BOOL fSystemMenu) */
void OnInitMenuPopup( HWND hwnd, HMENU hMenu, UINT item, BOOL fSystemMenu )
{
	OutputDebugString( std::format( L"OnInitMenuPopup( item={0}, fSysMenu={1} )\n", item, fSystemMenu ).data() );
}
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		HANDLE_MSG( hWnd, WM_PAINT, []( HWND hwnd )
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint( hwnd, &ps );
			// TODO: HDC を使用する描画コードをここに追加してください...
			EndPaint( hwnd, &ps );
		} );
		HANDLE_MSG( hWnd, WM_COMMAND, []( HWND hwnd, int cmdID, HWND hwndCtl, UINT codeNotify )
		{
			auto itr = g_commandHandler.find( cmdID );
			if( itr != g_commandHandler.end() )
			{
				itr->second( hwnd, cmdID, hwndCtl, codeNotify );
			}
			else
			{
				FORWARD_WM_COMMAND( hwnd, cmdID, hwndCtl, codeNotify, DefWindowProc );
			}
		} );
		HANDLE_MSG( hWnd, WM_DESTROY, []( HWND ) { PostQuitMessage( ERROR_SUCCESS ); } );
		HANDLE_MSG( hWnd, WM_INITMENUPOPUP, []( HWND hwnd, HMENU hMenu, UINT item, BOOL fSystemMenu )
		{
			//	いろいろ実験のメニュー
			if( item == 1 )
			{
				CheckMenuItem( hMenu, ID_DISP_QUEUE_STATUS, (g_dispQueueStatus) ? MF_CHECKED : MF_UNCHECKED );
			}
		} );
		default: return DefWindowProc( hWnd, message, wParam, lParam );
	}
	return 0;
}

// バージョン情報ボックスのメッセージ ハンドラーです。
INT_PTR CALLBACK About( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		HANDLE_MSG( hDlg, WM_INITDIALOG, []( HWND, HWND, LPARAM ) { return TRUE; } );
		HANDLE_MSG( hDlg, WM_COMMAND, []( HWND hwnd, int cmdID, HWND hwndCtl, UINT codeNotify )
		{
			switch( cmdID )
			{
			case IDOK:
			case IDCANCEL:
				EndDialog( hwnd, cmdID );
				break;
			}
		} );
	}
	return FALSE;
}
//-- 追加
inline std::wstring QueueStatusText( UINT status )
{
	std::wstring text;
	auto AddText = []( std::wstring& text, LPCWSTR addValue, UINT status, UINT flag )
	{
		if( !text.empty() )
		{
			text += L"|";
		}
		text += addValue;
		return status &~flag;
	};
	if( status & QS_KEY ) status = AddText( text, std::format( L"QS_KEY(0x{0:04x})", QS_KEY ).data(), status, QS_KEY );
	if( status & QS_MOUSEMOVE ) status = AddText( text, std::format( L"QS_MOUSEMOVE(0x{0:04x})", QS_MOUSEMOVE ).data(), status, QS_MOUSEMOVE );
	if( status & QS_MOUSEBUTTON ) status = AddText( text, std::format( L"QS_MOUSEBUTTON(0x{0:04x})", QS_MOUSEBUTTON ).data(), status, QS_MOUSEBUTTON );
	if( status & QS_POSTMESSAGE ) status = AddText( text, std::format( L"QS_POSTMESSAGE(0x{0:04x})", QS_POSTMESSAGE ).data(), status, QS_POSTMESSAGE );
	if( status & QS_TIMER ) status = AddText( text, std::format( L"QS_TIMER(0x{0:04x})", QS_TIMER ).data(), status, QS_TIMER );
	if( status & QS_PAINT ) status = AddText( text, std::format( L"QS_PAINT(0x{0:04x})", QS_PAINT ).data(), status, QS_PAINT );
	if( status & QS_SENDMESSAGE ) status = AddText( text, std::format( L"QS_SENDMESSAGE(0x{0:04x})", QS_SENDMESSAGE ).data(), status, QS_SENDMESSAGE );
	if( status & QS_HOTKEY ) status = AddText( text, std::format( L"QS_HOTKEY(0x{0:04x})", QS_HOTKEY ).data(), status, QS_HOTKEY );
	if( status & QS_ALLPOSTMESSAGE ) status = AddText( text, std::format( L"QS_ALLPOSTMESSAGE(0x{0:04x})", QS_ALLPOSTMESSAGE ).data(), status, QS_ALLPOSTMESSAGE );
	if( status & QS_RAWINPUT ) status = AddText( text, std::format( L"QS_RAWINPUT(0x{0:04x})", QS_RAWINPUT ).data(), status, QS_RAWINPUT );
	if( status & QS_TOUCH ) status = AddText( text, std::format( L"QS_TOUCH(0x{0:04x})", QS_TOUCH ).data(), status, QS_TOUCH );
	if( status & QS_POINTER ) status = AddText( text, std::format( L"QS_POINTER(0x{0:04x})", QS_POINTER ).data(), status, QS_POINTER );
	return text + std::format( L"(0x{0:04x})", status );
}
void APIENTRY DumpQueueStatus( LPCWSTR prefix )
{
	if( !g_dispQueueStatus )
	{
		return;
	}
	auto status = GetQueueStatus( QS_ALLINPUT );
	DWORD lastError = ERROR_SUCCESS;
	if( status == 0 )
	{
		lastError = GetLastError();
	}
	std::wstring msg = prefix;
	msg += L"GetQueueStatus( QS_ALLINPUT ):\n";
	if( status == 0 )
	{
		msg += L"  0x0000|0x0000\n";
		msg += std::format( L"  LastError={0}\n", lastError );
	}
	else
	{
		msg += L"  HI:" + QueueStatusText( HIWORD( status ) ) + L"\n";
		msg += L"  LO:" + QueueStatusText( LOWORD( status ) ) + L"\n";
	}
	OutputDebugString( msg.data() );
}
void APIENTRY TestPostThreadMessage( LPCWSTR prefix )
{
	auto threadID = GetCurrentThreadId();
	// スレッドにキューが割り当てられていないとエラーになるのでキューの存在チェックに利用可能
	auto result = PostThreadMessage( threadID, WM_NULL, 0, 0 );
	auto lastError = GetLastError();
	OutputDebugString( std::format( L"{0}:PostThreadMessage({1})={2}:LastError={3}\n", prefix, threadID, result, lastError ).data() );
}
static DWORD APIENTRY ThreadProc( LPVOID param )
{
	TestPostThreadMessage(L"SubThread");
	HWND hwnd = static_cast<HWND>(param);
	OutputDebugString( L"Call SendMessage()\n" );
	FORWARD_WM_COMMAND( hwnd, IDM_ABOUT, nullptr, 0, SendMessage );
	OutputDebugString( L"Return SendMessage()\n" );
	PostMessage( hwnd, WM_NULL, 0, 0 );
	return 0;
}
void TestSubThreadAction( HWND hwnd )
{
	OutputDebugString( L"Start --- TestSubThreadAction()\n" );
	auto currThreadID = GetCurrentThreadId();
	DWORD threadID;
	HANDLE hThread = CreateThread( nullptr, 0, ThreadProc, hwnd, 0, &threadID );
	if( hThread != nullptr )
	{
		CloseHandle( hThread );	//	使わないのでそのままクローズ
	}
	OutputDebugString( L"End --- TestSubThreadAction()\n" );
}
void APIENTRY DispVersion( HWND hwnd, int cmdId )
{
	OSVERSIONINFOEX info={ 0 };
	info.dwOSVersionInfoSize = sizeof( info );
	bool succeeded = false;
	std::wstring funcName;
	switch( cmdId )
	{
	case ID_GET_VERSION_EX:
		{
			funcName = L"GetVersionEx()";
			BOOL( WINAPI* pGetVersionEx )(_Out_  LPOSVERSIONINFOEXW lpVersionInformation);
			(FARPROC&)pGetVersionEx = GetProcAddress( GetModuleHandle( L"kernel32.dll" ), "GetVersionExW" );
			succeeded = pGetVersionEx != nullptr && pGetVersionEx( &info );
		}
		break;
	case ID_RTL_GET_VERSION:
		{
			funcName = L"RtlGetVersion()";
			HRESULT( NTAPI* pRtlGetVersion )(_Out_  LPOSVERSIONINFOEXW lpVersionInformation);
			(FARPROC&)pRtlGetVersion = GetProcAddress( GetModuleHandle( L"ntdll.dll" ), "RtlGetVersion" );
			succeeded = pRtlGetVersion != nullptr && SUCCEEDED( pRtlGetVersion( &info ) );
		}
		break;
	case ID_VERIFY_VERSION_INFO:
		{
			funcName = L"VerSetConditionMask()";
			OSVERSIONINFOEX chk ={ 0 };
			chk.dwOSVersionInfoSize = sizeof( chk );
			chk.dwMajorVersion = 1;

			auto conditionMask = VerSetConditionMask( 0, VER_MAJORVERSION, VER_GREATER_EQUAL );
			DWORD typeMask = VER_MAJORVERSION;
			while( VerifyVersionInfo( &chk, typeMask, conditionMask ) )
			{
				info.dwMajorVersion = chk.dwMajorVersion;
				chk.dwMajorVersion++;
			}
			chk.dwMajorVersion = info.dwMajorVersion;
			typeMask |= VER_MINORVERSION;
			conditionMask = VerSetConditionMask( conditionMask, VER_MINORVERSION, VER_GREATER_EQUAL );
			while( VerifyVersionInfo( &chk, typeMask, conditionMask ) )
			{
				info.dwMinorVersion = chk.dwMinorVersion;
				chk.dwMinorVersion++;
			}
			chk.dwMinorVersion = info.dwMinorVersion;

			conditionMask = VerSetConditionMask( conditionMask, VER_BUILDNUMBER, VER_GREATER_EQUAL );
			typeMask |= VER_BUILDNUMBER;
			while( VerifyVersionInfo( &chk, typeMask, conditionMask ) )
			{
				info.dwBuildNumber = chk.dwBuildNumber;
				chk.dwBuildNumber++;
			}
			chk.dwBuildNumber = info.dwBuildNumber;

			
			conditionMask = VerSetConditionMask( conditionMask, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL );
			typeMask |= VER_SERVICEPACKMAJOR;
			while( VerifyVersionInfo( &chk, typeMask, conditionMask ) )
			{
				info.wServicePackMajor = chk.wServicePackMajor;
				chk.wServicePackMajor++;
			}
			chk.wServicePackMajor = info.wServicePackMajor;

			conditionMask = VerSetConditionMask( conditionMask, VER_SERVICEPACKMINOR, VER_GREATER_EQUAL );
			typeMask |= VER_SERVICEPACKMINOR;
			while( VerifyVersionInfo( &chk, typeMask, conditionMask ) )
			{
				info.wServicePackMinor = chk.wServicePackMinor;
				chk.wServicePackMinor++;
			}
			chk.wServicePackMinor = info.wServicePackMinor;
		}
		break;
	}
	if( !funcName.empty() )
	{
		std::wstring msg =
			std::format( L"dwMajorVersion={0}", info.dwMajorVersion ) + L"\n" +
			std::format( L"dwMinorVersion={0}", info.dwMinorVersion ) + L"\n" +
			std::format( L"dwBuildNumber={0}", info.dwBuildNumber ) + L"\n" +
			std::format( L"dwPlatformId={0}", info.dwPlatformId ) + L"\n" +
			std::format( L"szCSDVersion={0}", info.szCSDVersion ) + L"\n" +

			std::format( L"wServicePackMajor={0}", info.wServicePackMajor ) + L"\n" +
			std::format( L"wServicePackMinor={0}", info.wServicePackMinor ) + L"\n" +
			std::format( L"wSuiteMask={0}", info.wSuiteMask ) + L"\n" +
			std::format( L"wProductType={0}", info.wProductType ) + L"\n" +
			std::format( L"wReserved={0}", info.wReserved ) + L"\n";
			MessageBox( hwnd, msg.data(), funcName.data(), MB_ICONINFORMATION|MB_OK );
	}
}
void APIENTRY TestWaitInputIdle( HWND hwnd, LPCWSTR execFileName )
{
	wchar_t modulePath[MAX_PATH];
	GetModuleFileName( nullptr, modulePath, MAX_PATH );
	lstrcpy( PathFindFileName( modulePath ), execFileName );
	STARTUPINFO si ={ 0 };
	PROCESS_INFORMATION pi ={ 0 };
	si.cb = sizeof( si );
	si.wShowWindow = SW_SHOWNOACTIVATE;
	MessageBox( hwnd, L"Call CreateProcess()", execFileName, MB_ICONINFORMATION|MB_OK );
	std::unique_ptr<wchar_t> ptr( _wcsdup( std::format( L"{0} SecondProcess", modulePath ).data() ) );
	OutputDebugString( L"Call CreateProcess()\n" );
	auto result = CreateProcess( nullptr, ptr.get(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi );
	if( result )
	{
		OutputDebugString( L"Call WaitForInputIdle()\n" );
		auto waitResult = WaitForInputIdle( pi.hProcess, INFINITE );
		OutputDebugString( L"Return WaitForInputIdle()\n" );
		CloseHandle( pi.hThread );
		CloseHandle( pi.hProcess );
		MessageBox( hwnd, L"Return WaitForInputIdle()", execFileName, MB_ICONINFORMATION|MB_OK );
	}
	else
	{
		auto lastError = GetLastError();
		MessageBox( hwnd, std::format( L"Fail CreateProcess( {0} )\nLastError={1}", execFileName, lastError ).data(), nullptr, MB_ICONEXCLAMATION|MB_OK);
	}
}
//---- おまけ -- MsgWaitForMultipleObjects を使った実装例(まともに動かないので要注意)
#include <map>
#include <functional>
#include <ppl.h>
//	この辺まとめてクラスにするのが良い(追加処理とかあるし)
class MainLoopWithWaitObjects
{
private:
	std::map<HANDLE, std::function<bool( bool, bool& )>> waitActions;
public:
	// 注意！！！待機ハンドルは最大でMAXIMUM_WAIT_OBJECTS-1(1はメッセージの分)なので、それ以上になる場合は何か工夫が必要
	DWORD APIENTRY SetupWaitHandles( HANDLE* waitHandles, DWORD capacityCount )
	{
		DWORD waitCount = 0;
		if( capacityCount > 0 )
		{
			for( const auto& pair : waitActions )
			{
				waitHandles[waitCount++] = pair.first;
			}
		}
		return waitCount;
	}
	bool APIENTRY SignalWaitAction( HANDLE wakeHandle, bool isSucceeded )
	{
		bool isContinue = true;
		auto itr = waitActions.find( wakeHandle );
		if( itr != waitActions.end() )
		{
			if( !itr->second( isSucceeded, isContinue ) )
			{
				waitActions.erase( itr );
			}
		}
		return isContinue;
	}
	bool APIENTRY PreTranslateMessage( const MSG* pMsg )
	{
		//	スレッドあてメッセージなので何らかの処理...
		if( pMsg->hwnd == nullptr )
		{
			// TODO
			//	メッセージを使ってwaitActions にオブジェクトを登録するというやり方もあるがオブジェクトの寿命が面倒なのでお勧めしない
		}
		return pMsg->hwnd != nullptr;
	}

	int RunMessageLoop()
	{
		HANDLE waitHandles[MAXIMUM_WAIT_OBJECTS-1] ={ nullptr };
		// 待機ハンドルをセットアップする。待機ハンドルのセットアップは、この関数を持つクラスで行うことを想定
		DWORD waitCount = SetupWaitHandles( waitHandles, MAXIMUM_WAIT_OBJECTS-1 );
		bool isContinue = true;
		DWORD timeoutTime = 60*1000;  //  デフォルトは１分ごとにタイムアウト
		int resultCode = ERROR_SUCCESS;
		do
		{
			auto result = ::MsgWaitForMultipleObjects( waitCount, waitHandles, FALSE, timeoutTime, QS_ALLINPUT );
			// MsgWaitForMultipleObjects のエラー(パラメータが間違っているとかが多い)
			if( result == WAIT_FAILED )
			{
				// エラーコードを見て処理の必要があればそれ相応に処理。ここではプログラム的なバグを想定してそのままリターン
				resultCode = ::GetLastError();
				isContinue = false;
			}
			// タイムアウト
			else if( result == WAIT_TIMEOUT )
			{
				// Win32 なので COMを使わないかもしれないけどサンプルとして用意
				CoFreeUnusedLibraries();
			}
			// メッセージが何か来たのでループを回す
			else if( result == WAIT_OBJECT_0+waitCount )
			{
				MSG msg;
				while( ::PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ) )
				{
					if( msg.message == WM_QUIT )
					{
						resultCode = static_cast<decltype(resultCode)>(msg.wParam);
						isContinue = false;
						break;
					}
					//  メッセージの事前処理
					//  msg.hWnd == nullptr の場合やモードレスダイアログなら IsDialogMessage を呼び出すなどを行う
					if( PreTranslateMessage( &msg ) )
					{
						::TranslateMessage( &msg );
						::DispatchMessage( &msg );
					}
				}
			}
			// 待機ハンドルがシグナル状態になった(==アクションを起こせ！)
			else if( WAIT_OBJECT_0 <= result && result < WAIT_OBJECT_0+waitCount )
			{
				isContinue = SignalWaitAction( waitHandles[result-WAIT_OBJECT_0], true );
				//  継続する場合は毎回整理
				if( isContinue )
				{
					waitCount = SetupWaitHandles( waitHandles, MAXIMUM_WAIT_OBJECTS-1 );
				}
			}
			// 待機に失敗した(Mutexの場合のみ)
			else if( WAIT_ABANDONED_0 <= result && result < WAIT_ABANDONED_0+waitCount )
			{
				isContinue = SignalWaitAction( waitHandles[result-WAIT_ABANDONED_0], false );
				//  継続する場合は毎回整理
				if( isContinue )
				{
					waitCount = SetupWaitHandles( waitHandles, MAXIMUM_WAIT_OBJECTS-1 );
				}
			}
		} while( isContinue );
		return resultCode;
	}
};

std::list<RECT> tabAreas;
