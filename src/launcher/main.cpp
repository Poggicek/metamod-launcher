#include <Windows.h>
#include <filesystem>
#include <commctrl.h>
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

void Error(const wchar_t* title, const wchar_t* expandedInfo, const wchar_t* fmt, ...) {
	va_list pArgs;
	va_start(pArgs, fmt);
	wchar_t buffer[1024];

	_vsnwprintf(buffer, std::size(buffer), fmt, pArgs);

	TASKDIALOGCONFIG config = { 0 };
	config.cbSize = sizeof(config);
	config.pszMainIcon = MAKEINTRESOURCEW(-7);
	config.pszWindowTitle = L"Error";
	config.pszMainInstruction = title;
	config.pszContent = buffer;
	config.dwCommonButtons = TDCBF_CLOSE_BUTTON;
	if (expandedInfo)
		config.pszExpandedInformation = expandedInfo;

	TaskDialogIndirect(&config, nullptr, nullptr, nullptr);
	va_end(pArgs);
}

enum StartOptions {
	OPTION_NONE = 0,
	OPTION_DEDICATED = IDOK,
	OPTION_CLIENT = IDIGNORE,
	OPTION_QUIT = IDCANCEL
};

int ClientIsNotSecure() {
	TASKDIALOG_BUTTON buttons[3] = { 0 };
	buttons[0].nButtonID = OPTION_DEDICATED;
	buttons[0].pszButtonText = L"Yes, run dedicated server\nRuns the server without insecure mode";
	buttons[1].nButtonID = OPTION_CLIENT;
	buttons[1].pszButtonText = L"Yes, run client with -insecure\nRuns the game with metamod in insecure mode";
	buttons[2].nButtonID = OPTION_QUIT;
	buttons[2].pszButtonText = L"No\nQuit the game";

	TASKDIALOGCONFIG config = { 0 };
	config.cbSize = sizeof(config);
	config.pszMainIcon = MAKEINTRESOURCEW(-6);
	config.pszWindowTitle = L"Error";
	config.pszMainInstruction = L"Client is NOT secure";
	config.pszContent = L"You tried to launch the CS2 client without a -dedicated flag AND without the -insecure flag, this would run the game and possibly get you VAC banned, if you wish to run the client append the -insecure command line parameter.\n\nAre you sure you want to continue?";
	config.pButtons = buttons;
	config.cButtons = 3;
	config.cxWidth = 250;
	config.dwFlags = TDF_USE_COMMAND_LINKS | TDF_ALLOW_DIALOG_CANCELLATION;

	int buttonId = OPTION_QUIT;
	TaskDialogIndirect(&config, &buttonId, nullptr, nullptr);


	return buttonId;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, wchar_t* lpCmdLine, int nCmdShow)
{
	int startOption = OPTION_NONE;

	if (wcsstr(lpCmdLine, L"-insecure") == nullptr && wcsstr(lpCmdLine, L"-dedicated") == nullptr) {
		startOption = ClientIsNotSecure();
		if (startOption == OPTION_QUIT)
			return 1;
	}

	WCHAR moduleFileName[MAX_PATH];
	if (!GetModuleFileNameW(NULL, moduleFileName, std::size(moduleFileName))) {
		MessageBoxW(NULL, L"Failed to find current executable", L"Error", MB_OK);
		return 1;
	}

	auto execPath = std::filesystem::path(moduleFileName).parent_path();
	auto launcherPath = execPath / "cs2.exe";
	auto dllPath = execPath / "metamod-loader.dll";

	if (!std::filesystem::exists(dllPath)) {
		Error(L"Error bootstrapping CS2", nullptr, L"Failed to find metamod-loader.dll\n\nMake sure that you put metamod-loader.dll in the same folder as metamod-launcher.exe");
		return 1;
	}


	if (!std::filesystem::exists(launcherPath)) {
		Error(L"Error bootstrapping CS2", nullptr, L"Failed to find cs2.exe\n\nMake sure that you put metamod-launcher.exe in the same folder as cs2.exe (game/bin/win64)");
		return 1;
	}

	std::wstring startCmdLine = L"cs2.exe " + std::wstring(lpCmdLine);

	if (startOption == OPTION_DEDICATED) {
		startCmdLine += L" -dedicated";
	}
	else if (startOption == OPTION_CLIENT) {
		startCmdLine += L" -insecure";
	}

	STARTUPINFOW si = { sizeof(si) };
	PROCESS_INFORMATION pi;
	auto success = CreateProcessW(
		launcherPath.wstring().c_str(),
		startCmdLine.data(),
		NULL,
		NULL,
		FALSE,
		CREATE_SUSPENDED,
		NULL,
		execPath.wstring().c_str(),
		&si,
		&pi
	);

	if (!success) {
		Error(L"Error bootstrapping CS2", nullptr, L"Failed to launch cs2.exe");
		return 1;
	}

	auto dllPathString = dllPath.wstring();
	auto dllPathRemoteAddr = VirtualAllocEx(pi.hProcess, nullptr, (dllPathString.size() + 1) * sizeof(wchar_t), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if(!dllPathRemoteAddr) {
		Error(L"Error bootstrapping CS2", nullptr, L"Failed to allocate memory for dll path");
		return 1;
	}

	success = WriteProcessMemory(pi.hProcess, dllPathRemoteAddr, dllPathString.c_str(), (dllPathString.size() + 1) * sizeof(wchar_t), nullptr);

	if (!success) {
		Error(L"Error bootstrapping CS2", nullptr, L"Failed to write external memory", L"Error");
		return 1;
	}

	auto metamodPath = (execPath / ".." / ".." / "csgo" / "addons" / "metamod" / "bin" / "win64" / "server.dll").lexically_normal();
	if (!std::filesystem::exists(metamodPath)) {
		Error(L"Error bootstrapping CS2", nullptr, L"Failed to find metamod's server.dll", L"Error");
		return 1;
	}

	const auto remoteThread = CreateRemoteThread(
		pi.hProcess, nullptr, 0, reinterpret_cast<::LPTHREAD_START_ROUTINE>(LoadLibraryW), dllPathRemoteAddr, 0, nullptr);

	if (!remoteThread) {
		Error(L"Error bootstrapping CS2", nullptr, L"Failed to create remote thread", L"Error");
		return 1;
	}

	WaitForSingleObject(remoteThread, INFINITE);

	ResumeThread(remoteThread);
	ResumeThread(pi.hThread);

	return 0;
}