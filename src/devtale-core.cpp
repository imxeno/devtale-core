// devtale-core.cpp : Defines the entry point for the application.
//
#include "devtale-core.h"
#include "jsonrpc.hpp"
#include "utils.h"
#include "memory.h"
#include "protocol.h"
#include "websocket_handler.h"

using namespace devtale;

#define DEVTALE_INSECURE false

bool running = true;
HINSTANCE h_instance;

void CreateDebugWindow()
{
	FILE* n_in;
	FILE* n_out;
	FILE* n_err;
	AllocConsole();
	freopen_s(&n_in, "CONIN$", "r", stdin);
	freopen_s(&n_out, "CONOUT$", "w", stdout);
	freopen_s(&n_err, "CONOUT$", "w", stderr);
	std::cout.clear();
}


int main()
{
#ifdef _DEBUG
	CreateDebugWindow();
#endif
	boost::asio::io_context ioc;
	websocket_handler pepega_h(ioc);
	packet_handler packet_h(&pepega_h);
	pepega_h.connect();
	protocol::get()->set_packet_handler(&packet_h);
	ioc.run();
	std::cout << "Run!" << std::endl;
}

// DLL Injection entry point

// ReSharper disable once CppInconsistentNaming
bool WINAPI DllMain(_In_ HINSTANCE instance, _In_ DWORD call_reason, _In_ LPVOID reserved)
{
	UNUSED(instance);
	UNUSED(reserved);
	
	switch (call_reason)
	{
	case DLL_PROCESS_ATTACH:
		{
		h_instance = instance;
		std::thread main_thread(main);
		main_thread.detach();
		}
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
	default:
		break;
	}
	return TRUE;
}
