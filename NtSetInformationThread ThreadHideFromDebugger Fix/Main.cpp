#include <iostream>
#include <Windows.h>
#include <string>
#include <filesystem>


using DirectInput8CreateFunction = HRESULT(
	HINSTANCE hinst,
	DWORD dwVersion,
	REFIID riidltf,
	LPVOID* ppvOut,
	LPUNKNOWN punkOuter);

DirectInput8CreateFunction* OriginalDirectInput8CreatePointer = nullptr;

extern "C" __declspec(dllexport) HRESULT WINAPI DirectInput8Create(
	HINSTANCE hinst,
	DWORD dwVersion,
	REFIID riidltf,
	LPVOID * ppvOut,
	LPUNKNOWN punkOuter) {
	return OriginalDirectInput8CreatePointer(hinst, dwVersion, riidltf, ppvOut, punkOuter);
}


DWORD WINAPI Start() {
	std::string System32Path(0x1000, 0);
	GetSystemDirectoryA(System32Path.data(), System32Path.size());
	std::filesystem::path DInputDLLPath{};
	DInputDLLPath = System32Path + R"(\dinput8.dll)";
	auto OriginalDll = LoadLibraryA(DInputDLLPath.string().c_str());
	OriginalDirectInput8CreatePointer = reinterpret_cast<decltype(OriginalDirectInput8CreatePointer)>(GetProcAddress(OriginalDll, "DirectInput8Create"));
	return 0;
}

BOOL APIENTRY DllMain(HMODULE ModuleHandle,
	DWORD ReasonForCall,
	LPVOID Reserved) {
	if(ReasonForCall != DLL_PROCESS_ATTACH) return TRUE;
	Start();
	return TRUE;
}