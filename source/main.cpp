#include <stdafx.h>
#include "rc_ImageLoader.h"
#include "Renderer.h"

#define CONSOLE_WIDTH 120
#define CONSOLE_HEIGHT 40

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
	Renderer mainWindow;
	if (!mainWindow.Initialise(consoleWindow, windowWidth, windowHeight))
	{
		u32 imWidth = 0, imHeight = 0;
		u8 imBBP = 0;
		void* imPalette = nullptr;
		ImageLoader::LoadFromFile("../resources/images/TITLEH.pcx" , RC_ImageType::IM_PCX, imWidth, imHeight, imBBP, imPalette);

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

				mainWindow.ClearRenderBuffer();

				for (int i = 0; i < 5000; ++i)
				{
					unsigned int xPos = rand() % 640;
					unsigned int yPos = rand() % 480;
					// Colour is in the foramt AA__BB__GG__RR
					unsigned int colour = (rand() % 256 << 16 | rand() % 256 << 8 | rand() % 256);
					mainWindow.FillRenderBuffer(xPos, yPos, 1, 1, &colour);
				}

				mainWindow.Draw();

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
	}

	
	return 0;
}


