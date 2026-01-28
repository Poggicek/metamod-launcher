#include <Windows.h>
#include <filesystem>
#include <safetyhook.hpp>

void HookLoadLibrary();
std::filesystem::path metamodPath;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow)
{
	WCHAR moduleFileName[MAX_PATH];
	if (!GetModuleFileNameW(NULL, moduleFileName, std::size(moduleFileName))) {
		MessageBoxW(NULL, L"Failed to find current executable", L"Error", MB_OK);
		return 1;
	}

	auto execPath = std::filesystem::path(moduleFileName).parent_path();
	auto enginePath = execPath / "engine2.dll";

	if (!std::filesystem::exists(enginePath)) {
		MessageBoxW(NULL, L"Failed to find engine2.dll", L"Error", MB_OK);
		return 1;
	}

	HMODULE engineModule = LoadLibraryW(enginePath.c_str());
	if (!engineModule) {
		MessageBoxW(NULL, L"Failed to load engine2.dll", L"Error", MB_OK);
		return 1;
	}

	metamodPath = (execPath / ".." / ".." / "csgo" / "addons" / "metamod" / "bin" / "win64" / "server.dll").lexically_normal();
	if (!std::filesystem::exists(metamodPath)) {
		MessageBoxW(NULL, L"Failed to find metamod server.dll", L"Error", MB_OK);
		return 1;
	}

	using Source2Main = int(*)(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nShowCmd, PSTR path, const char* modName);
	auto source2Main = reinterpret_cast<Source2Main>(GetProcAddress(engineModule, "Source2Main"));
	if (!source2Main) {
		MessageBoxW(NULL, L"Failed to find Source2Main in engine2.dll", L"Error", MB_OK);
		FreeLibrary(engineModule);
		return 1;
	}

	char multibyteDir[260];
	if (!WideCharToMultiByte(CP_UTF8, 0, execPath.wstring().c_str(), -1, multibyteDir, MAX_PATH, NULL, NULL))
	{
			MessageBoxW(NULL, L"Failed to convert path", L"Error", MB_OK);
	}

	HookLoadLibrary();

	source2Main(hInstance, hPrevInstance, lpCmdLine, nCmdShow, multibyteDir, "csgo");
	return 0;
}

SafetyHookInline g_loadlibrary_hook{};

HMODULE Hook_loadLibraryExW(wchar_t* fileName, HANDLE hFile, DWORD dwFlags) {
	if (wcsstr(fileName, L"server.dll") != 0) {

		auto result = g_loadlibrary_hook.call<HMODULE>(metamodPath.wstring().c_str(), hFile, dwFlags);

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