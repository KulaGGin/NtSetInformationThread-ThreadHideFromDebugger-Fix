#include <filesystem>
#include <iostream>
#include <Windows.h>
#include "MinHook.h"
#include "NtSetInformationThreadFix.h"

using DirectInput8CreateFunction = HRESULT(
	HINSTANCE hinst,
	DWORD dwVersion,
	REFIID riidltf,
	LPVOID* ppvOut,
	LPUNKNOWN punkOuter);

DirectInput8CreateFunction* GetOriginalDirectInput8CreatePointer() {
	std::string System32Path(0x1000, 0);
	GetSystemDirectoryA(System32Path.data(), System32Path.size());
	std::filesystem::path DInputDLLPath = System32Path.data() + std::string(R"(\dinput8.dll)");
	auto OriginalDll = LoadLibraryA(DInputDLLPath.string().c_str());
	return reinterpret_cast<DirectInput8CreateFunction*>(GetProcAddress(OriginalDll, "DirectInput8Create"));
}

extern "C" __declspec(dllexport) HRESULT WINAPI DirectInput8Create(
	HINSTANCE hinst,
	DWORD dwVersion,
	REFIID riidltf,
	LPVOID * ppvOut,
	LPUNKNOWN punkOuter) {
	return GetOriginalDirectInput8CreatePointer()(hinst, dwVersion, riidltf, ppvOut, punkOuter);
}

BOOL APIENTRY DllMain(HMODULE ModuleHandle,
                      DWORD ReasonForCall,
                      LPVOID Reserved) {
	if(ReasonForCall != DLL_PROCESS_ATTACH) return TRUE;

	static NtSetInformationThreadFix NtSetInformationThreadFix{};

	return TRUE;
}
