#include "framework.h"
#include "Lab1.h"

#define MAX_LOADSTRING 100

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

ATOM MyRegisterClass(HINSTANCE); 
BOOL InitInstance(HINSTANCE, int); 
ATOM ClassBall(HINSTANCE);
BOOL InitBall(HINSTANCE, int);
ATOM PaddleClass(HINSTANCE);
BOOL InitPaddle(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
void BouncingBall(SHORT*, SHORT*);
DWORD WINAPI BallMovement(__in LPVOID);
DWORD WINAPI PaddleMovement(__in LPVOID);
void FileOpen(HBITMAP*);

SHORT SpeedX = 2, SpeedY = 1;
HWND hWndMain, hWndBall, hWndPaddle;
int counterHit = 0, counterMiss = 0;
HBITMAP bitmap;


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	LoadStringW(hInstance, IDS_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_LAB1, szWindowClass, MAX_LOADSTRING);

	MyRegisterClass(hInstance);
	ClassBall(hInstance);
	PaddleClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	if (!InitBall(hInstance, nCmdShow))
	{
		return FALSE;
	}

	if (!InitPaddle(hInstance, nCmdShow))
	{
		return FALSE;
	}

	DWORD BallThreadID, PaddleThreadID;
	HANDLE BallThreadHandle = CreateThread(0, 0, BallMovement, nullptr, 0, &BallThreadID);
	HANDLE PaddleThreadHandle = CreateThread(0, 0, PaddleMovement, nullptr, 0, &PaddleThreadID);

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LAB1));

	MSG msg;

	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return static_cast <int>(msg.wParam);
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wcex.hCursor = nullptr;

	HBRUSH hb = CreateSolidBrush(RGB(100, 255, 10));
	wcex.hbrBackground = hb;

	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_LAB1);

	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));

	return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_EX_LAYERED | WS_MAXIMIZEBOX,
		CW_USEDEFAULT, 0, 500, 350, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd )
	{
		return FALSE;
	}

	hWndMain = hWnd;
	PAINTSTRUCT paint;
	HDC hdc = BeginPaint(hWnd, &paint);

	SetWindowLong(hWnd, GWL_EXSTYLE,
		GetWindowLong(hWnd, GWL_EXSTYLE)| WS_EX_LAYERED);
	SetLayeredWindowAttributes(hWnd, RGB(0,255,0), (255 * 80) / 100, LWA_ALPHA);
	SetTextColor(hdc, RGB(255, 0, 255));

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	EnableMenuItem(GetMenu(hWndMain), IDM_TILE, MF_GRAYED);
	EnableMenuItem(GetMenu(hWndMain), IDM_STRECH, MF_GRAYED);

	return TRUE;
}

ATOM ClassBall(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = nullptr;
	wcex.hCursor = nullptr;

	HBRUSH hb = ::CreateSolidBrush(RGB(255, 0, 0));
	wcex.hbrBackground = hb;

	wcex.lpszMenuName = nullptr;

	wcex.lpszClassName = L"BallClass";
	wcex.hIconSm = nullptr;

	return RegisterClassExW(&wcex);
}

BOOL InitBall(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;

	HWND hWnd = CreateWindowW(L"BallClass", nullptr, WS_CHILD | WS_VISIBLE,
		CW_USEDEFAULT, 0, 20, 20, hWndMain, nullptr, hInstance, nullptr);


	if (!hWnd)
	{
		return FALSE;
	}

	hWndBall = hWnd;

	HRGN region = CreateEllipticRgn(0, 0, 10, 10);
	SetWindowRgn(hWnd, region, true);

	SetWindowLong(hWnd, GWL_EXSTYLE,
		GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(hWnd, RGB(0, 255, 0), (255 * 100) / 100, LWA_ALPHA);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

void BouncingBall(SHORT *SpeedX, SHORT *SpeedY)
{
	RECT BallPosition, WinPosition, PaddlePosition;
	GetWindowRect(hWndBall, &BallPosition);
	GetWindowRect(hWndMain, &WinPosition);
	GetWindowRect(hWndPaddle, &PaddlePosition);
	POINT lowLeftBall;
	if ((BallPosition.top == PaddlePosition.top || BallPosition.bottom == PaddlePosition.bottom) && BallPosition.right >= PaddlePosition.left + 10)
	{
		*SpeedY = -*SpeedY;
	}
	if (BallPosition.top >= PaddlePosition.top && BallPosition.bottom <= PaddlePosition.bottom && BallPosition.right >= PaddlePosition.left + 10)
	{
		*SpeedX = -*SpeedX;
		counterHit++;
		UpdateWindow(hWndMain);
		RedrawWindow(hWndMain, NULL, NULL, RDW_INTERNALPAINT | WM_ERASEBKGND | RDW_ERASE | RDW_INVALIDATE);
	}

	if (BallPosition.left <= WinPosition.left + 5)
	{
		*SpeedX = -*SpeedX;
	}
	if (BallPosition.right >= WinPosition.right)
	{
		*SpeedX = -*SpeedX;
		counterMiss++;
		
		UpdateWindow(hWndMain);
		RedrawWindow(hWndMain, NULL, NULL, RDW_INTERNALPAINT | WM_ERASEBKGND | RDW_ERASE | RDW_INVALIDATE);
	}
	if (BallPosition.top <= WinPosition.top + 50 || BallPosition.bottom >= WinPosition.bottom) 
	{
		*SpeedY = -*SpeedY;
	}
	lowLeftBall.x = BallPosition.left + *SpeedX;
	lowLeftBall.y = BallPosition.top + *SpeedY;

	ScreenToClient(hWndMain, &lowLeftBall);

	SetWindowPos(hWndBall, nullptr, lowLeftBall.x, lowLeftBall.y, 20, 20, SWP_NOOWNERZORDER);
}

DWORD WINAPI BallMovement(__in LPVOID lpParameter)
{

	for (int i = 0;;)
	{
		BouncingBall(&SpeedX, &SpeedY);
		Sleep(2);
	}
	return 0;
}

ATOM PaddleClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = nullptr;
	wcex.hCursor = nullptr;
	wcex.hbrBackground = reinterpret_cast <HBRUSH>(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;

	wcex.lpszClassName = L"PaddleClass";
	wcex.hIconSm = nullptr;

	return RegisterClassExW(&wcex);
}

BOOL InitPaddle(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;

	HWND hWnd = CreateWindowW(L"PaddleClass", nullptr, WS_CHILD,
		CW_USEDEFAULT, 0, 25, 80, hWndMain, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	hWndPaddle = hWnd;

	RECT WinPosition;
	GetWindowRect(hWndMain, &WinPosition);
	SetWindowPos(hWnd, nullptr, 460, 210, 25, 80, SWP_NOOWNERZORDER);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
} 

DWORD WINAPI PaddleMovement(__in LPVOID lpParameter)
{
	RECT PaddlePosition, WinPosition;
	POINT PaddleXY, MouseXY;
	for (;;)
	{
		GetWindowRect(hWndPaddle, &PaddlePosition);
		GetWindowRect(hWndMain, &WinPosition);
		PaddleXY.y = PaddlePosition.top;
		PaddleXY.x = PaddlePosition.left;
		ScreenToClient(hWndMain, &PaddleXY);
		GetCursorPos(&MouseXY);
		if (MouseXY.y < WinPosition.top + 40 || MouseXY.y > WinPosition.bottom - 80)
			continue;
		ScreenToClient(hWndMain, &MouseXY);
		SetWindowPos(hWndPaddle, nullptr, PaddleXY.x, MouseXY.y, 25, 80, SWP_NOOWNERZORDER);
	}
	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUT), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case IDM_NEW_GAME:
		{
			SetWindowPos(hWndBall, nullptr, 0, 0, 20, 20, SWP_NOOWNERZORDER);
			counterHit = 0;
			counterMiss = 0;
		}
			break;
		case IDM_COLOR:
		{
			CHOOSECOLOR color;        

			static COLORREF acrCustClr[16];
			HBRUSH hbrush;         
			static DWORD rgbCurrent;  
 
			ZeroMemory(&color, sizeof(color));
			color.lStructSize = sizeof(color);
			color.hwndOwner = hWndMain;
			color.lpCustColors = (LPDWORD)acrCustClr;
			color.rgbResult = rgbCurrent;
			color.Flags = CC_FULLOPEN | CC_RGBINIT;

			if (ChooseColor(&color) == TRUE)
			{
				hbrush = CreateSolidBrush(color.rgbResult);
				rgbCurrent = color.rgbResult;
				SetClassLongPtr(hWndMain, GCLP_HBRBACKGROUND, (LONG_PTR)hbrush);
			}

			UpdateWindow(hWndMain);
			RedrawWindow(hWndMain, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);
		}
			break;
		case IDM_BITMAP:
		{
			//HBITMAP bitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP));
			FileOpen(&bitmap);
			HDC hdc = GetDC(hWndMain);
			HDC memDC = CreateCompatibleDC(hdc);
			HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, bitmap);
			
			if(!BitBlt(hdc, 0, 0, 48, 48, memDC, 0, 0, SRCCOPY))
				MessageBox(hWndMain, L"BitBlt failed", L"", MB_OK);
			HBRUSH hbrush = CreatePatternBrush(oldBitmap);
			SetClassLongPtr(hWndMain, GCLP_HBRBACKGROUND, (LONG_PTR)hbrush);/*

			if (!StretchBlt(hdc, 0, 0, 350, 500, memDC, 0, 0, 48, 48, SRCCOPY))
				MessageBox(hWndMain, L"streching failed", L"", MB_OK);
			else
				MessageBox(hWndMain, L"streching success", L"", MB_OK);
			*/
			UpdateWindow(hWndMain);
			RedrawWindow(hWndMain, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);
			SelectObject(memDC, oldBitmap);
			DeleteObject(oldBitmap);
			DeleteDC(memDC);
			ReleaseDC(hWndMain, hdc);

			EnableMenuItem(GetMenu(hWndMain), IDM_TILE, MF_ENABLED);
			EnableMenuItem(GetMenu(hWndMain), IDM_STRECH, MF_ENABLED);
			CheckMenuItem(GetMenu(hWndMain), IDM_STRECH, MF_UNCHECKED);
			CheckMenuItem(GetMenu(hWndMain), IDM_TILE, MF_CHECKED);
		}
		break;
		case IDM_STRECH:
		{
			bitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP));
			HDC hdc = GetDC(hWndMain);
			HDC memDC = CreateCompatibleDC(hdc);
			HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, bitmap);
			if (!StretchBlt(hdc, 0, 0, 350, 500, memDC, 0, 0, 48, 48, SRCCOPY))
				MessageBox(hWndMain, L"streching failed", L"", MB_OK);
			HBRUSH hbrush = CreatePatternBrush(bitmap);
			SetClassLongPtr(hWndMain, GCLP_HBRBACKGROUND, (LONG_PTR)hbrush);
			SetStretchBltMode(hdc, COLORONCOLOR);
			UpdateWindow(hWndMain);
			RedrawWindow(hWndMain, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);
			ReleaseDC(hWndMain, hdc);
			CheckMenuItem(GetMenu(hWndMain), IDM_STRECH, MF_CHECKED);
			CheckMenuItem(GetMenu(hWndMain), IDM_TILE, MF_UNCHECKED);
		}
			break;
		case IDM_TILE:
		{ 
			bitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP));
			HDC hdc = GetDC(hWndMain);
			HDC memDC = CreateCompatibleDC(hdc);
			HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, bitmap);
			HBRUSH hbrush = CreatePatternBrush(bitmap);
			SetClassLongPtr(hWndMain, GCLP_HBRBACKGROUND, (LONG_PTR)hbrush);
			UpdateWindow(hWndMain);
			RedrawWindow(hWndMain, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);
			ReleaseDC(hWndMain, hdc);
			CheckMenuItem(GetMenu(hWndMain), IDM_STRECH, MF_UNCHECKED);
			CheckMenuItem(GetMenu(hWndMain), IDM_TILE, MF_CHECKED);
		}
		break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	
	case WM_PAINT:
	{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			TCHAR sHit[2], sMiss[2];
			_stprintf_s(sHit, TEXT("%d"), counterHit);
			_stprintf_s(sMiss, TEXT("%d"), counterMiss);
			HFONT font = CreateFont(
				- MulDiv(24, GetDeviceCaps(hdc, LOGPIXELSY), 72), 0, 0, 0, FW_BOLD, false, FALSE, 0, EASTEUROPE_CHARSET,
				OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T(" Verdana "));
			HFONT oldFont = (HFONT)SelectObject(hdc, font);
			SetBkMode(hdc, TRANSPARENT);
			RECT rc;
			GetClientRect(hWnd, &rc);
			rc.left = 100;
			rc.right = 150;
			rc.top = 40;
			rc.bottom = 100;
			DrawText(hdc, sHit, (int)_tcslen(sHit), &rc,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			rc.left = 350;
			rc.right = 400;
			rc.top = 40;
			rc.bottom = 100;
			DrawText(hdc, sMiss, (int)_tcslen(sHit), &rc,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			SelectObject(hdc, oldFont);
			DeleteObject(font);
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
		
	case WM_INITDIALOG:
		return static_cast<INT_PTR>(TRUE);
		
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return static_cast<INT_PTR>(TRUE);
		}
		break;
	}
	return static_cast<INT_PTR>(FALSE);
}


void FileOpen(HBITMAP *hbm) //taken from: https://stackoverflow.com/questions/8555953/getopenfilename-and-system-function-call-run-time-errors-c-win32-api
{
	OPENFILENAME ofn;          
	TCHAR szFile[MAX_PATH + 1];   

	ZeroMemory(szFile, sizeof(szFile));

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWndMain;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = TEXT("All\0*.*\0Text\0*.TXT\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

if (GetOpenFileName(&ofn) == TRUE)
{
	*hbm = LoadBitmapW(hInst, ofn.lpstrFile);
}
}