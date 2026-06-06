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

std::mutex g_shutdownMutex;
std::condition_variable g_shutdownCV;
bool g_bRunning = true;

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

	if (!g_Loader.Init())
	{
		CLogger::Error("Initialization failed!\n");
		return 1;
	}

	CLogger::Info("server started successfully.\nPress Ctrl+C to exit.\n");

	// thread for bot handlers.
	std::thread botThread(&CBotManager::Run, &g_BotManager);

	// --- main thread wait mode ---
	// SignalHandler triggers wake.
	{
		std::unique_lock<std::mutex> lock(g_shutdownMutex);
		g_shutdownCV.wait(lock, [] { return !g_bRunning; });
	}

	// --- GÜVENLİ KAPATMA (SAFE SHUTDOWN) ---
	CLogger::Info("Server shutting down, please wait...\n");

	// Bot thread'inin güvenli bir şekilde mevcut döngüsünü bitirmesini bekliyoruz.
	// (Tabii Run() fonksiyonu içerisindeki döngü g_bRunning durumuna bakmalı)
	if (botThread.joinable())
	{
		botThread.join();
		CLogger::Info("Bot thread ended by success.");
	}

	// TODO: save user info and free memory if needed
	g_Loader.ShutDown();
	std::cout << "Server shutdown complete. Exiting program.\n";
	return 0;
}

