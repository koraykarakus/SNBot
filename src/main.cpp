// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <mutex>
#include <condition_variable>
#include "Loader.h"
#include "logger.h"

Loader* g_pLoader;
std::mutex g_shutdownMutex;
std::condition_variable g_shutdownCV;
bool g_bRunning = true;

void SignalHandler(int signal)
{
	if (signal == SIGINT
		|| signal == SIGTERM)
	{
		std::cout << "\n[Signal] Shutdown signal received (" << signal << ")...\n";

		// Kilidi al ve ana thread'e durması gerektiğini bildir
		{
			std::lock_guard<std::mutex> lock(g_shutdownMutex);
			g_bRunning = false;
		}
		// g_shutdownCV.wait() kullanan ana thread'i uyandır
		g_shutdownCV.notify_one();
	}
}

int main()
{
	// Ctrl + C
	std::signal(SIGINT, SignalHandler);

	// terminate signal from sys
	std::signal(SIGTERM, SignalHandler);

	g_pLoader = new Loader();

	if (!g_pLoader->Init())
	{
		Logger::Error("Initialization failed!\n");
		delete g_pLoader;
		g_pLoader = nullptr;
		return 1;
	}

	Logger::Info("Sunucu baglantilari dinlemeye basladi...");

	std::cout << "\nTest server started successfully. "
		"Press Ctrl+C to exit.\n";

	// --- ANA THREAD BEKLEME MODU ---
	// Bu kısım işlemciyi %0 yükle bekletir. 
	// Sadece SignalHandler tetiklendiğinde uyanır.
	{
		std::unique_lock<std::mutex> lock(g_shutdownMutex);
		g_shutdownCV.wait(lock, [] { return !g_bRunning; });
	}

	// --- GÜVENLİ KAPATMA (SAFE SHUTDOWN) ---
	std::cout << "Server shutting down, please wait...\n";

	// save user info and free memory
	g_pLoader->ShutDown();
	delete g_pLoader;

	std::cout << "Server shutdown complete. Exiting program.\n";
	return 0;
}