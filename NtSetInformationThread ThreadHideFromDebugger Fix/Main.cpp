#include <filesystem>
#include <iostream>
#include <Windows.h>
#include "MinHook.h"

using NtSetInformationThreadFunction = NTSTATUS(HANDLE, ULONG, PULONG, ULONG);

NtSetInformationThreadFunction* NtSetInformationThread;
NtSetInformationThreadFunction* OriginalNtSetInformationThread;

ULONG ThreadHideFromDebugger = 0x11;

using DirectInput8CreateFunction = HRESULT(
	HINSTANCE hinst,
	DWORD dwVersion,
	REFIID riidltf,
	LPVOID* ppvOut,
	LPUNKNOWN punkOuter);

DirectInput8CreateFunction* OriginalDirectInput8CreatePointer = nullptr;

NTSTATUS NtSetInformationThread_Detour(HANDLE ThreadHandle,
	ULONG ThreadInformationClass,
	PULONG ThreadInformation,
	ULONG ThreadInformationLength) {

	if(ThreadInformationClass == ThreadHideFromDebugger)
		return 0;
	
	return OriginalNtSetInformationThread(ThreadHandle, ThreadInformationClass, ThreadInformation, ThreadInformationLength);
}

void LoadOriginalDInput8DLL() {
	std::string System32Path(0x1000, 0);
	GetSystemDirectoryA(System32Path.data(), System32Path.size());
	std::filesystem::path DInputDLLPath = System32Path.data() + std::string(R"(\dinput8.dll)");
	auto OriginalDll = LoadLibraryA(DInputDLLPath.string().c_str());
	OriginalDirectInput8CreatePointer = reinterpret_cast<decltype(OriginalDirectInput8CreatePointer)>(GetProcAddress(OriginalDll, "DirectInput8Create"));
}

extern "C" __declspec(dllexport) HRESULT WINAPI DirectInput8Create(
	HINSTANCE hinst,
	DWORD dwVersion,
	REFIID riidltf,
	LPVOID * ppvOut,
	LPUNKNOWN punkOuter) {
	return OriginalDirectInput8CreatePointer(hinst, dwVersion, riidltf, ppvOut, punkOuter);
}

void HookNtSetInformationThread() {
	MH_Initialize();

	MH_CreateHook(NtSetInformationThread, NtSetInformationThread_Detour, reinterpret_cast<LPVOID*>(&OriginalNtSetInformationThread));
	MH_EnableHook(NtSetInformationThread);
}

void LoadNtDLLAndFunctions() {
	HMODULE NtDLL = LoadLibrary("ntdll.dll");
	NtSetInformationThread = reinterpret_cast<NtSetInformationThreadFunction*>(GetProcAddress(NtDLL, "NtSetInformationThread"));
}


void Start() {
	LoadOriginalDInput8DLL();
	LoadNtDLLAndFunctions();
	HookNtSetInformationThread();
}

BOOL APIENTRY DllMain(HMODULE ModuleHandle,
                      DWORD ReasonForCall,
                      LPVOID Reserved) {
	if(ReasonForCall != DLL_PROCESS_ATTACH) return TRUE;

	Start();

	return TRUE;
}
