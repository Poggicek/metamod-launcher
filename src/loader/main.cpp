#include <Windows.h>
#include <safetyhook.hpp>
#include <filesystem>

std::filesystem::path g_metamodPath;
void HookLoadLibrary();

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH) {
		WCHAR moduleFileName[MAX_PATH];
		if (!GetModuleFileNameW(NULL, moduleFileName, std::size(moduleFileName))) {
			MessageBoxW(NULL, L"Failed to find current path", L"Error", MB_OK);
			return 1;
		}

		auto execPath = std::filesystem::path(moduleFileName).parent_path();

		g_metamodPath = (execPath / ".." / ".." / "csgo" / "addons" / "metamod" / "bin" / "win64" / "server.dll").lexically_normal();
		if (!std::filesystem::exists(g_metamodPath)) {
			MessageBoxW(NULL, L"Failed to find metamod server.dll", L"Error", MB_OK);
			return 1;
		}

		HookLoadLibrary();
	}

	return TRUE;
}

SafetyHookInline g_loadlibrary_hook{};

HMODULE Hook_loadLibraryExW(wchar_t* fileName, HANDLE hFile, DWORD dwFlags) {
	if (wcsstr(fileName, L"server.dll") != 0) {
		auto result = g_loadlibrary_hook.call<HMODULE>(g_metamodPath.wstring().c_str(), hFile, dwFlags);

		// remove hook
		g_loadlibrary_hook = {};

		return result;
	}

	return g_loadlibrary_hook.call<HMODULE>(fileName, hFile, dwFlags);
}

void HookLoadLibrary() {
	HMODULE moduleHandle = LoadLibraryA("kernel32.dll");

	using LoadLibraryType = decltype(&Hook_loadLibraryExW);
	LoadLibraryType originalAddr = (LoadLibraryType)GetProcAddress(moduleHandle, "LoadLibraryExW");


	if (originalAddr == NULL) {
		MessageBoxW(NULL, L"Failed to find LoadLibraryA", L"Error", MB_OK);
		return;
	}

	g_loadlibrary_hook = safetyhook::create_inline(originalAddr, Hook_loadLibraryExW);
}