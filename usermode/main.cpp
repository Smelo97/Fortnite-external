#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>

#include "driver.h"

#include "Menu.h"

int main()
{
	DWORD process_id = get_process_id("FortniteClient-Win64-Shipping.exe");

	if (open_driver_handle("FortniteClient-Win64-Shipping.exe"))
	{
		uintptr_t base = get_process_base();

		std::cout << process_id << "\n";
		std::cout << std::hex << base << std::dec << "\n";
		
		SDK::ProcessID = process_id;
		SDK::ModuleBase = base;

		SetupWindow();
		DirectXInit(MyWnd);

		MainLoop();
	}
	
	system("pause");
}