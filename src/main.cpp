// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <mutex>
#include <condition_variable>
#include "CLoader.h"
#include "CLogger.h"
#include "CBotManager.h"
#include "CDatabase.h"

std::mutex g_shutdownMutex;
std::condition_variable g_shutdownCV;
bool g_bRunning = true;

CBotManager* g_pBotManager = nullptr;
CDatabase* g_pDatabase = nullptr;
CLoader* g_pLoader = nullptr;
bool g_bLoaded = false;


void SignalHandler(int signal)
{
	if (signal == SIGINT
		|| signal == SIGTERM)
	{
		std::cout << "\n[Signal] Shutdown signal received (" << signal << ")...\n";

		// get lock, and tell main thread to wait
		{
			std::lock_guard<std::mutex> lock(g_shutdownMutex);
			g_bRunning = false;
		}
		// g_shutdownCV.wait() wake main thread
		g_shutdownCV.notify_one();
	}
}

int main()
{
	// Ctrl + C
	std::signal(SIGINT, SignalHandler);

	// terminate signal from sys
	std::signal(SIGTERM, SignalHandler);

	g_pBotManager = new CBotManager();
	g_pDatabase = new CDatabase();
	g_pLoader = new CLoader();

	if (!g_pLoader->Init())
	{
		CLogger::Error("[Main] : g_pLoader->Init() failed!\n");
		return 1;
	}

	CLogger::Info("[Main] : server started successfully.Press Ctrl+C to exit.\n");

	// thread for bot handlers.
	std::thread botThread(&CBotManager::Run, g_pBotManager);
	// --- main thread wait mode ---
	// SignalHandler triggers wake.
	{
		std::unique_lock<std::mutex> lock(g_shutdownMutex);
		g_shutdownCV.wait(lock, [] { return !g_bRunning; });
	}

	// --- safe shutdown ---
	CLogger::Info("[Main] : server shutting down, please wait...\n");

	// Bot thread'inin güvenli bir şekilde mevcut döngüsünü bitirmesini bekliyoruz.
	// (Tabii Run() fonksiyonu içerisindeki döngü g_bRunning durumuna bakmalı)
	if (botThread.joinable())
	{
		botThread.join();
		CLogger::Info("[Main] : bot thread ended by success.\n");
	}

	g_pLoader->ShutDown();

	delete g_pLoader;
	delete g_pDatabase;
	delete g_pBotManager;
	// TODO: save user info and free memory if needed
	CLogger::Info("[Main] : server shutdown complete. Exiting program.\n");
	return 0;
}

