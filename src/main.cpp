// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#endif

#include <iostream>
#include "CApplication.h"


CApplication* g_pApp;

void SignalHandler(int signal)
{
	if (signal == SIGINT
		|| signal == SIGTERM)
	{
		std::cout << "\n[Signal] Shutdown signal received (" << signal << ")...\n";
		if (g_pApp != nullptr)
		{
			g_pApp->Shutdown();
		}
	}
}

int main()
{
#ifdef _WIN32
	// console output
	SetConsoleOutputCP(CP_UTF8);
	// console input
	SetConsoleCP(CP_UTF8);
#endif
	// Ctrl + C
	std::signal(SIGINT, SignalHandler);

	// terminate signal from sys
	std::signal(SIGTERM, SignalHandler);

	CApplication App;
	g_pApp = &App;

	if (!App.Init())
	{
		CLogger::Error("[Main] : App.Init() failed! press enter to close.\n");
		std::cin.get();
		return 1;
	}

	CLogger::Info("[Main] : server started successfully.Press Ctrl+C to exit.\n");

	App.Run();

	// --- safe shutdown ---
	CLogger::Info("[Main] : server shutting down, please wait...\n");

	g_pApp = nullptr;
	// TODO: save user info and free memory if needed
	CLogger::Info("[Main] : server shutdown complete. Exiting program.\n");
	return 0;
}
