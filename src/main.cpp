// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

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

	// Bot thread'inin güvenli bir şekilde mevcut döngüsünü bitirmesini bekliyoruz.
	// (Tabii Run() fonksiyonu içerisindeki döngü g_bRunning durumuna bakmalı)
	
	g_pApp = nullptr;
	// TODO: save user info and free memory if needed
	CLogger::Info("[Main] : server shutdown complete. Exiting program.\n");
	return 0;
}

