#include <string>
#include <Windows.h>

enum UpdateStatus
{
	UPDATE_AVAILABLE,
	LATEST_VERSION,
	ERROR_COMPARABLE_LEFT,
	ERROR_COMPARABLE_RIGHT,
	UNKNOWN_ERROR
};


class AppVersion
{
public:
	AppVersion();

	int get_major();
	int get_minor();
	int get_revision();
	int get_build();

	void parse_version(int major, int minor, int revision, int build);
	void parse_version(std::string version);
	void parse_version(DWORD VersionMS, DWORD VersionLS);

	void parse_from_file(std::string path);

	UpdateStatus compare(AppVersion version);

	bool equal(AppVersion version);

private:
	int major;
	int minor;
	int revision;
	int build;
};
