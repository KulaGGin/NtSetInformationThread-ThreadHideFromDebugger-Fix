#pragma once
#include <filesystem>
#include <iostream>
#include <Windows.h>
#include "MinHook.h"

class NtSetInformationThreadFix {
public:
	using NtSetInformationThreadFunction = NTSTATUS(HANDLE, ULONG, PULONG, ULONG);

	NtSetInformationThreadFix() {
		Start();
	}

	static inline NtSetInformationThreadFunction* NtSetInformationThread;
	static inline NtSetInformationThreadFunction* OriginalNtSetInformationThread;


	inline static ULONG ThreadHideFromDebugger = 0x11;

	static NTSTATUS NtSetInformationThread_Detour(HANDLE ThreadHandle,
		ULONG ThreadInformationClass,
		PULONG ThreadInformation,
		ULONG ThreadInformationLength) {

		if(ThreadInformationClass == ThreadHideFromDebugger)
			return 0;

		return OriginalNtSetInformationThread(ThreadHandle, ThreadInformationClass, ThreadInformation, ThreadInformationLength);
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
		LoadNtDLLAndFunctions();
		HookNtSetInformationThread();
	}
};