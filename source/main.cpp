#include <stdafx.h>

#define CONSOLE_WIDTH 120
#define CONSOLE_HEIGHT 40
//\Lazily declare some global variables and initialise them
HWND g_windowHandle		= nullptr;
HDC g_windowDC			= nullptr;
void* g_bitBuffer		= nullptr;
BITMAPINFO* g_bmpInfo	= nullptr;
HBITMAP g_bufferBmp		= nullptr;
HDC g_bufDevContext		= nullptr;
HBITMAP g_defBmp		= nullptr;

// Static windows message handle callback function
static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

int main(int argv, char* argc[])
{
	UNREFERENCED_PARAMETER(argv);
	UNREFERENCED_PARAMETER(argc);
	// Create a console buffer
	wchar_t* screen = new wchar_t[CONSOLE_WIDTH * CONSOLE_HEIGHT];
	memset(screen, ' ', CONSOLE_WIDTH * CONSOLE_HEIGHT);
	// Windows code to create console screen buffer
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);

	HWND consoleWindow = GetConsoleWindow();
	SetWindowPos(consoleWindow, 0, 20, 180, 0,0, SWP_NOSIZE | SWP_NOZORDER);
	// Create a render context window - or bitmap render window
	unsigned int windowWidth = 640;
	unsigned int windowHeight = 480;

	//Register a windows class with this console application to get a device context
	WNDCLASSEX wndClass = { 0 };
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WndProc;
	wndClass.hInstance = (HINSTANCE)GetModuleHandle(NULL);
	wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
	wndClass.lpszMenuName = nullptr;
	wndClass.lpszClassName = "RayCaster Framework";

	if (!RegisterClassExA(&wndClass))
	{
		return 1;
	}

	// Set up window for this application
	// This window will make use of device independent of rendering
	LONG x = 0; LONG y = 0;
	RECT consoleRect = { NULL };
	if (GetWindowRect(consoleWindow, &consoleRect))
	{
		x = consoleRect.right;
		y = consoleRect.top;
	}

	RECT windowRect = { x, y, x + (LONG)windowWidth, y + (LONG)windowHeight };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);
	// Create window based of window description
	g_windowHandle = CreateWindowA("Raycaster Framework", "Main Scene",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.bottom,
		nullptr, nullptr, (HINSTANCE)GetModuleHandle(NULL), nullptr);
	ShowWindow(g_windowHandle, SW_SHOW);
	MoveWindow(g_windowHandle, x, y, windowWidth, windowHeight, true);

	// Create a back buffer render target
	// We need device context for this
	g_windowDC = GetDC(g_windowHandle);
	if (g_windowDC == nullptr)
	{
		return 1;
	}

	// Create a bitesize array that will be large enough to hold bitmapinforheader
	char* data = (char*)malloc(sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));
	if (data != nullptr)
	{
		g_bmpInfo = (BITMAPINFO*)data;
		g_bmpInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		g_bmpInfo->bmiHeader.biWidth = (LONG)windowWidth;
		g_bmpInfo->bmiHeader.biHeight = (LONG)windowHeight;
		g_bmpInfo->bmiHeader.biBitCount = 32;
		g_bmpInfo->bmiHeader.biPlanes = 1;
		g_bmpInfo->bmiHeader.biCompression = BI_RGB;
		g_bmpInfo->bmiHeader.biXPelsPerMeter = 0;
		g_bmpInfo->bmiHeader.biYPelsPerMeter = 0;
		g_bmpInfo->bmiHeader.biClrUsed = 256;
		g_bmpInfo->bmiHeader.biClrImportant = 256;

		g_bufferBmp = CreateDIBSection(g_windowDC, g_bmpInfo, DIB_RGB_COLORS, &g_bitBuffer, NULL, 0);
		if (g_bufferBmp == nullptr)
		{
			free(data);
			return 0;
		}

		// Get buffer device context
		g_bufDevContext = CreateCompatibleDC(g_windowDC);
		if (g_bufDevContext == nullptr)
		{
			free(data);
			return 0;
		}

		g_defBmp = (HBITMAP)SelectObject(g_bufDevContext, g_bufferBmp);
		if (g_defBmp == nullptr)
		{
			free(data);
			return 0;
		}
		free(data);
	}

	// Seed random
	srand((unsigned int)time(nullptr));

	MSG msg = { 0 };


	// Create a timer, with current time and previous time to get delta time between frames.
	auto currentTime = std::chrono::high_resolution_clock::now();
	auto previousTime = currentTime;
	

	// Delta time variable
	std::chrono::duration <double> elapsedTime;

	unsigned int frame = 0;

	while (msg.message != WM_QUIT) 
	{
		RECT clrect;
		GetClientRect(g_windowHandle, &clrect);

		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else 
		{
			previousTime = currentTime;
			currentTime = std::chrono::high_resolution_clock::now();
			elapsedTime = currentTime - previousTime;
			double fElapsedTime = elapsedTime.count();

			// Clear our bitmap window background
			FillRect (g_bufDevContext, &clrect, (HBRUSH)(0x0000) + 2);

			for (int i = 0; i < 5000; ++i)
			{
				unsigned int xPos = rand() % windowWidth;
				unsigned int yPos = rand() % windowHeight;
				// Calculate the position in the screen buffer
				unsigned int index = (xPos + (yPos * windowWidth)) * 4;

				if (xPos + (yPos * windowWidth) > windowWidth* windowHeight)
				{

				}
				else
				{
					//0x00_BB_GG_RR
					unsigned int colour = (rand() % 256) << 16 | (rand() % 256) << 8 | (rand() % 256);
					//Copy the colour value into bitmap buffer memory
					memcpy(&(((char*)(g_bitBuffer))[index]), &colour, 3);
				}
			}
			RedrawWindow(g_windowHandle, nullptr, nullptr, RDW_INVALIDATE);


			if (frame % 30 == 0)
			{
				swprintf_s(screen, 16, L"FPS=%4.2f ", 1.0f / fElapsedTime);
				screen[(CONSOLE_WIDTH * CONSOLE_HEIGHT) - 1] = '\0';
				DWORD dwBytesWritten = 0;
				WriteConsoleOutputCharacter(hConsole, (LPCSTR)screen, 22, { 0,0 }, &dwBytesWritten);
			}
			++frame;
		}
		
	}

	return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) 
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	PAINTSTRUCT paintStruct;
	HDC hDC;
	if (hwnd == g_windowHandle)
	{
		switch (message)
		{
			case WM_PAINT: // Called when the window content is invalidated
			{
				hDC = BeginPaint(hwnd, &paintStruct);

				RECT clRect;
				GetClientRect(hwnd, &clRect);
				BitBlt(hDC, clRect.left, clRect.top, (clRect.right - clRect.left) + 1,
					(clRect.bottom - clRect.top) + 1, g_bufDevContext, 0, 0, SRCCOPY);

				EndPaint(hwnd, &paintStruct);
				break;
			}
			case WM_DESTROY: // Called wjem tje window needs to close
			{
				PostQuitMessage(0);
				break;
			}
			default:
				return DefWindowProc(hwnd, message, wParam, lParam);
		};
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

