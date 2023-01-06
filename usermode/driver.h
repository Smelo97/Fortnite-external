enum request {
	getbase = 0,
	reading = 1,
	writing = 2
};

struct communication {
	request request;
	DWORD process_id;
	DWORD key;
	ULONG64 process_base;

	void* output;
	void* buffer_address;
	uintptr_t address;
	size_t size;
};

inline void(__fastcall* hooked_function)(void* a1, void* a2);
inline DWORD process_id = 0;
inline uint64_t process_base = 0;

template <class T>
inline T read(uint64_t address, size_t size = sizeof(T))
{
	T response{};

	communication request = {};
	SecureZeroMemory(&request, sizeof(communication));

	request.key = 0xDEADBEEF;
	request.request = request::reading;
	request.process_id = process_id;
	request.address = address;
	request.output = &response;
	request.size = size;
	hooked_function(&request, 0);

	return response;
}

template <typename T>
inline bool write(uintptr_t address, T value, size_t size = sizeof(T))
{
	communication request = {};
	SecureZeroMemory(&request, sizeof(communication));

	request.key = 0xDEADBEEF;
	request.request = request::writing;
	request.process_id = process_id;
	request.address = address;
	request.size = size;
	request.buffer_address = (void*)&value;
	hooked_function(&request, 0);

	return true;
}

inline DWORD get_process_id(const char* process_name)
{
	PROCESSENTRY32 pt;
	HANDLE hsnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pt.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(hsnap, &pt)) {
		do {
			if (!lstrcmpi(pt.szExeFile, process_name)) {
				CloseHandle(hsnap);
				return pt.th32ProcessID;
			}
		} while (Process32Next(hsnap, &pt));
	}
	CloseHandle(hsnap);
	return 0;
}

inline ULONG64 get_process_base()
{
	communication request = {};
	SecureZeroMemory(&request, sizeof(communication));

	request.key = 0xDEADBEEF;
	request.request = request::getbase;
	request.process_id = process_id;
	hooked_function(&request, 0);

	return request.process_base;
}

inline bool open_driver_handle(const char* process_name)
{
	process_id = get_process_id(process_name);
	if (!process_id)
	{
		return false;
	}
	else
	{
		HMODULE user32 = LoadLibraryA("user32.dll");
		HMODULE win32u = LoadLibraryA("win32u.dll");

		if (!user32 || !win32u)
		{
			return false;
		}
		else
		{

			FARPROC function_addr = GetProcAddress(win32u, "NtGdiCreateDIBBrush");
			if (!function_addr)
			{
				return false;
			}
			else
			{
				*(void**)&hooked_function = function_addr;

				process_base = get_process_base();
				if (!process_base)
				{
					return false;
				}
			}
		}
	}

	return true;
}