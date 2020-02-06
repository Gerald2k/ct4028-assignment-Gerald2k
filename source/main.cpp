#include <stdafx.h>

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

	// Create a timer, with current time and previous time to get delta time between frames.
	auto currentTime = std::chrono::high_resolution_clock::now();
	auto previousTime = currentTime;
	

	// Delta time variable
	std::chrono::duration <double> elapsedTime;

	unsigned int frame = 0;

	while (true) 
	{
		previousTime = currentTime;
		currentTime = std::chrono::high_resolution_clock::now();
		elapsedTime = currentTime - previousTime;

		double fElapsedTime = elapsedTime.count();
		if (frame % 30 == 0) 
		{
			swprintf_s(screen, 16, L"FPS=%4.2f ", 1.0f / fElapsedTime);
			screen[(CONSOLE_WIDTH * CONSOLE_HEIGHT) - 1] = '\0';
			DWORD dwBytesWritten = 0;
			WriteConsoleOutputCharacter(hConsole, (LPCSTR)screen, 22, { 0,0 }, &dwBytesWritten);
		}
		++frame;
	}

	return 0;
}

