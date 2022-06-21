#include "HttpsClient.h"
#include "AppVersion.h"

#include <boost/foreach.hpp>

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <shellapi.h>


HANDLE StartProgram(std::string file, std::string params) {
	if (file.empty()) return NULL;

	SHELLEXECUTEINFO ShExecInfo = { 0 };
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = NULL;
	ShExecInfo.lpFile = file.c_str();
	ShExecInfo.lpParameters = params.c_str();
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_SHOW;
	ShExecInfo.hInstApp = NULL;

	if (ShellExecuteEx(&ShExecInfo))
		return ShExecInfo.hProcess;

	return NULL;
}

std::string CurrentDir() {
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	std::string::size_type pos = std::string(buffer).find_last_of("\\/");
	return std::string(buffer).substr(0, pos);
}

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {

	HANDLE main = StartProgram(CurrentDir() + "\\putty.exe", "");

	HttpsClient client;

	client.open("the.earth.li", "/~sgtatham/putty/latest/w64/");

	std::string redir_download = client.get_header("Location");
	size_t begin = redir_download.find("/~sgtatham/putty/");
	size_t end = redir_download.find("/w64");
	std::string current_version = redir_download.substr(begin + 17, end - begin - 17);

	AppVersion curr_version;
	curr_version.parse_version(current_version);

	AppVersion local_verion;
	local_verion.parse_from_file(CurrentDir() + "\\putty.exe");

	switch (curr_version.compare(local_verion))
	{
	case UpdateStatus::LATEST_VERSION:
		//MessageBoxW(0, L"Latest version", L"", 0);
		break;
	case UpdateStatus::UPDATE_AVAILABLE:
		if (MessageBox(0, "Putty new version available. Do you want update it?", "PuttyUpdater", MB_ICONINFORMATION | MB_OKCANCEL) == IDOK)
		{
			client.open("the.earth.li", redir_download.substr(redir_download.find_first_of('/~')-1) + "putty-64bit-" + current_version + "-installer.msi");

			if (client.get_status_code() != 200) {
				MessageBox(0, client.get_status_message().c_str(), "Error", 0);
				break;
			}
			std::string file_path = CurrentDir() + "//putty" + current_version + ".msi";
			std::ofstream out_file(file_path, std::ios::binary);
			out_file << client.get_body();
			out_file.close();

			TerminateProcess(main, 0);

			HANDLE handle = StartProgram(file_path, "INSTALLDIR=" + CurrentDir());

			if (handle != NULL) {
				WaitForSingleObject(handle, INFINITE);
				std::remove(file_path.c_str());
				StartProgram(CurrentDir()+"\\putty.exe", "");
			}
		}
		break;
	case UpdateStatus::ERROR_COMPARABLE_LEFT:
		MessageBoxW(0, L"Error compare current version", L"PuttyUpdater", 0);
		break;
	case UpdateStatus::ERROR_COMPARABLE_RIGHT:
		MessageBoxW(0, L"Error compare local version", L"PuttyUpdater", 0);
		break;
	case UpdateStatus::UNKNOWN_ERROR:
		MessageBoxW(0, L"Error", L"PuttyUpdater", 0);
		break;
	default:
		break;
	}
}