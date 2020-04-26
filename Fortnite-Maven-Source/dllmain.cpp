#include "stdafx.h"

void RunThis()
{
	Util::CreateConsole();
	MH_Initialize();
	SettingsHelper::Initialize();

	if (!Util::Initialize()) {
		return;
	}

	if (!Offsets::Initialize()) {
		return;
	}

	if (!Core::Initialize()) {
		return;
	}

	if (!Render::Initialize()) {
		return;
	}
}
unsigned long main_thread ( void* )
{

	RunThis();
	return 0;
}

bool DllMain(HMODULE module_instance, DWORD call_reason, void*)
{
	if (call_reason != DLL_PROCESS_ATTACH)
		return false;

	wchar_t file_name[MAX_PATH] = L"";
	GetModuleFileNameW(module_instance, file_name, _countof(file_name));
	LoadLibraryW(file_name);

	return true;
}

extern "C" __declspec(dllexport)
LRESULT wnd_hk(int32_t code, WPARAM wparam, LPARAM lparam)
{
	// handle race condition from calling hook multiple times
	static auto done_once = false;

	const auto pmsg = reinterpret_cast<MSG*>(lparam);

	if (!done_once && pmsg->message == 0x5b0)
	{
		UnhookWindowsHookEx(reinterpret_cast<HHOOK>(lparam));

		// you can just one line this since CloseHandle doesn't throw unless it's under debug mode
		if (const auto handle = CreateThread(nullptr, 0, &main_thread, nullptr, 0, nullptr); handle != nullptr)
			CloseHandle(handle);

		done_once = true;
	}

	// call next hook in queue
	return CallNextHookEx(nullptr, code, wparam, lparam);
}
