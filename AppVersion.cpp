#include "AppVersion.h"

std::string GetLastErrorAsString(std::string prefix = "")
{
	//Get the error message, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
		return std::string(); //No error message has been recorded

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	std::string message(messageBuffer, size);

	//Free the buffer.
	LocalFree(messageBuffer);

	return prefix + message;
}

AppVersion::AppVersion()
{
	this->major = 0;
	this->minor = 0;
	this->revision = 0;
	this->build = 0;
}

int AppVersion::get_major()
{
	return this->major;
}

int AppVersion::get_minor()
{
	return this->minor;
}

int AppVersion::get_revision()
{
	return this->revision;
}

int AppVersion::get_build()
{
	return this->build;
}

void AppVersion::parse_version(int major, int minor, int revision, int build)
{
	this->major = major;
	this->minor = minor;
	this->revision = revision;
	this->build = build;
}

void AppVersion::parse_version(std::string version)
{
	int cur_part = 0;
	char* context;
	char* version_part = strtok_s((char*)version.c_str(), ".", &context);
	while (version_part != NULL)
	{
		switch (cur_part++)
		{
		case 0:
			this->major = atoi(version_part);
			break;
		case 1:
			this->minor = atoi(version_part);
			break;
		case 2:
			this->revision = atoi(version_part);
			break;
		case 3:
			this->build = atoi(version_part);
			break;
		default:
			break;
		}

		version_part = strtok_s(NULL, ".", &context);
	}
}

void AppVersion::parse_version(DWORD VersionMS, DWORD VersionLS)
{
	this->major = HIWORD(VersionMS);
	this->minor = LOWORD(VersionMS);
	this->revision = HIWORD(VersionLS);
	this->build = LOWORD(VersionLS);
}

void AppVersion::parse_from_file(std::string path)
{
	DWORD  verHandle = 0;
	DWORD  verSize = GetFileVersionInfoSizeA(path.c_str(), &verHandle);

	if (verSize != NULL)
	{
		LPSTR verData = new char[verSize];
		if (GetFileVersionInfoA(path.c_str(), verHandle, verSize, verData))
		{
			UINT   size = 0;
			LPBYTE lpBuffer = NULL;

			if (VerQueryValueA(verData, "\\", (VOID FAR * FAR*) & lpBuffer, &size))
			{
				if (size)
				{
					VS_FIXEDFILEINFO* verInfo = (VS_FIXEDFILEINFO*)lpBuffer;

					if (verInfo->dwSignature == 0xfeef04bd) {
						parse_version(verInfo->dwFileVersionMS, verInfo->dwFileVersionLS);
						return;
					}
					else
					{
						MessageBox(0, GetLastErrorAsString("File info signature failed ").c_str(), "PuttyUpdater", MB_ICONERROR | MB_OK);
					}
				}
				else
				{
					MessageBox(0, GetLastErrorAsString("File info empty ").c_str(), "PuttyUpdater", MB_ICONERROR | MB_OK);
				}
			}
			else
			{
				MessageBox(0, GetLastErrorAsString("Error in VerQueryValue: ").c_str(), "PuttyUpdater", MB_ICONERROR | MB_OK);
			}
		}
		else
		{
			MessageBox(0, GetLastErrorAsString("Error in GetFileVersionInfo: ").c_str(), "PuttyUpdater", MB_ICONERROR | MB_OK);
		}
		delete[] verData;
	}
	else
	{
		MessageBox(0, GetLastErrorAsString("Error in GetFileVersionInfoSize: ").c_str(), "PuttyUpdater", MB_ICONERROR | MB_OK);
	}

	parse_version(0, 0, 0, 0);
}

UpdateStatus AppVersion::compare(AppVersion version)
{
	if ((this->major == 0) && (this->minor == 0) &&
		(this->revision == 0) && (this->build == 0)) return UpdateStatus::ERROR_COMPARABLE_LEFT;

	if ((version.get_major() == 0) && (version.get_minor() == 0) &&
		(version.get_revision() == 0) && (version.get_build() == 0)) return UpdateStatus::ERROR_COMPARABLE_RIGHT;

	if (equal(version)) return UpdateStatus::LATEST_VERSION;

	if ((this->major >= version.get_major()) && (this->minor >= version.get_minor()) &&
		(this->revision >= version.get_revision()) && (this->build >= version.get_build())) return UpdateStatus::UPDATE_AVAILABLE;

	return UpdateStatus::UNKNOWN_ERROR;
}

bool AppVersion::equal(AppVersion version)
{
	return (this->major == version.get_major()) && (this->minor == version.get_minor()) &&
		(this->revision == version.get_revision()) && (this->build == version.get_build());
}