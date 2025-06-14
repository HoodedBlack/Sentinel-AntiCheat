#include <Windows.h>
#include <iostream>
#include "../Shared/ioctl.h"


int main(int argc, char* argv[]) {
	HANDLE hDev = CreateFileW(SYMLINK_USERMODE, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);

	if (hDev == INVALID_HANDLE_VALUE) {
		std::cerr << "Failed to open device: " << GetLastError() << std::endl;
		return 1;
	}

	while (true) {
		DWORD junk = 0;
		BOOL ok = DeviceIoControl(hDev, IOCTL_AC_HEARTBEAT, nullptr, 0, nullptr, 0, &junk, nullptr);
		if (!ok) {
			std::cerr << "DeviceIoControl failed: " << GetLastError() << std::endl;
		} else {
			std::cout << "Heartbeat sent." << std::endl;
		}
		Sleep(1000);
	}

	CloseHandle(hDev);
	return 0;
}
