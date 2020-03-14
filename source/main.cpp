#include <stdafx.h>
#include "rc_ImageLoader.h"
#include "Renderer.h"

#define CONSOLE_WIDTH 120
#define CONSOLE_HEIGHT 40

#define PI 3.1415926535

#define FOV 60
#define HALF_FOV (FOV>>1)

int main(int argv, char* argc[])
{
	// Set up our player characters position and facing
	float playerPosX = 2.0;	// X = 2
	float playerPosY = 3.f;	// Y = 3
	float playerDirX = 1.f; // X Direction = right
	float playerDirY = 0.f;	// Y direction = up

	// FOV calculateions and getting near plane
	float nearPlaneDist = 1.f;
	float nearPlaneLength = tanf(HALF_FOV * (float)(PI / 180.f)) * nearPlaneDist;	//float nearPlaneLength = tan(FOV / 2) * adjacentLength

	UNREFERENCED_PARAMETER(argv);
	UNREFERENCED_PARAMETER(argc);

	// Create a console buffer
	wchar_t* screen = new wchar_t[CONSOLE_WIDTH * CONSOLE_HEIGHT];
	memset(screen, ' ', CONSOLE_WIDTH * CONSOLE_HEIGHT);

	u8 levelMap[64] = {
		1, 1, 1, 1, 1, 1, 1, 1,
		1, 0, 1, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 1, 1, 0, 1,
		1, 0, 0, 0, 0, 1, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 1, 1, 1, 1, 1,
		1, 1, 0, 0, 0, 0, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 1,
	};

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
		void* imageData = ImageLoader::LoadFromFile("../resources/images/TITLEH.pcx" , RC_ImageType::IM_PCX, imWidth, imHeight, imBBP, imPalette);

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

				// Clear render buffer
				mainWindow.ClearRenderBuffer();
				// Draw Background
				drawBackground(mainWindow);
				// Camera plane is perpendicular to player direction
				// Swap components and negate y
				// Multiply by near plane length to get vector to right length
				float camPlaneX = -playerDirY * nearPlaneLength;
				float camPlaneY = playerDirX * nearPlaneLength;

				for (u32 c = 0; c < windowWidth; c++) // c represents whuch screen column we are examining
				{
					// get the x position as a value between -1 & 1
					float cameraX = ((2.f * c) / (float)windowWidth) - 1.f;
					float rayDirX = playerDirX + (camPlaneX * cameraX);
					float rayDirY = playerDirY + (camPlaneY * cameraX); //POSSIBLY ERROR HE MENT Y HERE??

					// Length of ray from one x/y side to next x/y side
					float deltaDistX = (rayDirX != 0.f) ? std::abs(1.f / rayDirX) : 0;
					float deltaDistY = (rayDirY != 0.f) ? std::abs(1.f / rayDirY) : 0;

					// Players current grid position
					int mapX = (int)(playerPosX);
					int mapY = (int)(playerPosY);

					// Length of ray from current position to next x/y side
					float sideDistX = 0.f;
					float sideDistY = 0.f;

					float perpWallDist = 0.f;

					// Direction to step in x/y +-1
					int stepX = 0;
					int stepY = 0;

					int collision = 0;
					if (rayDirX < 0)
					{
						stepX = -1;
						sideDistX = (playerPosX - mapX) * deltaDistX;
					}
					else
					{
						stepX = 1;
						sideDistX = (mapX + 1.f - playerPosX) * deltaDistX;
					}
					if (rayDirY < 0)
					{
						stepY = -1;
						sideDistY = (playerPosY - mapY) * deltaDistY;
					}
					else
					{
						stepY = 1;
						sideDistY = (mapY + 1.f - playerPosY) * deltaDistY;
					}

					int yIntersection = 0;
					while (collision == 0)
					{
						if (sideDistX < sideDistY)
						{
							sideDistX += deltaDistX;
							mapX += stepX;
							yIntersection = 0;
						}
						else {
							sideDistY += deltaDistY;
							mapY += stepY;
							yIntersection = 1;
						}
						collision = levelMap[(mapX + (mapY * mapWidth))];
					}

					// We have collided our raycast with a wall
					perpWallDist = (!yIntersection) ?
						(mapX - playerPosX + (1 - stepX) / 2.f) / rayDirX:
						(mapY - playerPosY + (1 - stepY) / 2.f) / rayDirY;

					// Calculate the height of line to draw
					s32 lineHeight = (int)(windowHeight / perpWallDist);
					// Calculate upper and lower points to draw
					s32 yPos = (windowHeight / perpWallDist);
					mainWindow.FillRenderBuffer(c, yPos, 1, lineHeight, (!yIntersection) ? 0x00FF0FF0 : 0X00880880);
				}

				
				//mainWindow.FillRenderBuffer(0, 0, imWidth, imHeight, imageData);

				/*for (int i = 0; i < 5000; ++i)
				{
					unsigned int xPos = rand() % 640;
					unsigned int yPos = rand() % 480;
					// Colour is in the foramt AA__BB__GG__RR
					unsigned int colour = (rand() % 256 << 16 | rand() % 256 << 8 | rand() % 256);
					mainWindow.FillRenderBuffer(xPos, yPos, 1, 1, &colour);
				}*/

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

void drawBackground(Renderer& a_renderer)
{
	unsigned int ceilingColour = 0x00505050;
	unsigned int floorColour = 0x00B2B2B2;
	a_renderer.FillRenderBuffer(0, 0, 640, 320, ceilingColour);
	a_renderer.FillRenderBuffer(0, 320, 640, 320, floorColour);
}

