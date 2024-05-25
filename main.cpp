#include <winsock2.h>
#include <Windows.h>
#include <print>
#include "consoleHelper.hpp"
#include "MinHook.h"
#include <string>

static_assert(sizeof(int*) == 8, "Only x64 build is supported!");
#ifndef NDEBUG
#error "Only Release build is supported!"
#endif

typedef int (WSAAPI* PTR_WSASENDTO)(
	SOCKET s,
	LPWSABUF lpBuffers,
	DWORD dwBufferCount,
	LPDWORD lpNumberOfBytesSent,
	DWORD dwFlags,
	const struct sockaddr* lpTo,
	int iToLen,
	LPWSAOVERLAPPED lpOverlapped,
	LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

// Pointer for calling original WsaSendTo.
PTR_WSASENDTO fpWsaSendTo = NULL;

// Detour function which overrides WsaSendTo.
int WINAPI HookFunctionWsaSendTo(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, const struct sockaddr* lpTo, int iToLen, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
	std::println("[+][+][+] HELLO FROM HOOK! [+][+][+]");

	return fpWsaSendTo(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpTo, iToLen, lpOverlapped, lpCompletionRoutine);
}

DWORD WINAPI MainThread(const LPVOID hDllModule);
void CleanupAndExit(LPVOID hDllModule);

BOOL WINAPI DllMain(
	HINSTANCE hinstDLL,  // handle to DLL module
	DWORD fdwReason,     // reason for calling function
	[[maybe_unused]] LPVOID lpReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hinstDLL);
		CreateThread(NULL, NULL, MainThread, hinstDLL, NULL, NULL);
	}
	return TRUE;
}

DWORD WINAPI MainThread(const LPVOID hDllModule) {

	console::ConsoleHelper::Attach();
	std::println("[+] Press + to apply hook.");
	std::println("[+] Press - to unhook.");
	std::println("[+] Press END to exit.\n");

	if (MH_Initialize() != MH_OK)
	{
		std::println("[-] Failed MH_Initialize");
		CleanupAndExit(hDllModule);
	}
	std::println("[+] MH_Initialize successful.");

	HMODULE hModuleWs2 = GetModuleHandle("ws2_32.dll");
	PTR_WSASENDTO pFunctionToHook = (PTR_WSASENDTO)GetProcAddress(hModuleWs2, "WSASendTo");

	std::println("[+] ws2_32.dll found loaded at {:016X}", (uint64_t)hModuleWs2);
	std::println("[+] WSASendTo found at {:016X}", (uint64_t)pFunctionToHook);

	MH_STATUS status;
	if ((status = MH_CreateHook(pFunctionToHook, &HookFunctionWsaSendTo, (LPVOID*)&fpWsaSendTo)) != MH_OK)
	{
		MessageBox(NULL, "Failed MH_CreateHook", "Error", MB_OK);

		const std::string statusStr = std::to_string(status);
		std::println("[-] Failed MH_CreateHook: {}", statusStr);
		std::println("[-] Exiting, you can close the console window.");

		CleanupAndExit(hDllModule);
	}

	std::println("[+] Hook created");

	while (true)
	{
		if (GetAsyncKeyState(VK_END) & 0x1)
		{
			break;
		}

		if (GetAsyncKeyState(VK_ADD) & 0x1)
		{
			std::println("[+] Hooking...");
			MH_EnableHook(MH_ALL_HOOKS);
		}

		if (GetAsyncKeyState(VK_SUBTRACT) & 0x1)
		{
			std::println("[+] Unhooking...");
			MH_DisableHook(MH_ALL_HOOKS);
		}

		Sleep(50);
	}

	std::println("[+] Exiting, you can close the console window.");

	CleanupAndExit(hDllModule);
}

void CleanupAndExit(LPVOID hDllModule)
{
	if (!console::ConsoleHelper::Detach())
	{
		std::println("[-] Failed to free console!");
	}

	MH_Uninitialize();
	FreeLibraryAndExitThread((HMODULE)hDllModule, 0);
}