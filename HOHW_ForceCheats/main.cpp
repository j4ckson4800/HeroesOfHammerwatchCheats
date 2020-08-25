#include <iostream>
#include <algorithm>
#include <string_view>

#include "utilities/address.hpp"
#include "utilities/remote_process.hpp"

int main() {
	
	//80 7D ? 00 74 ? C6 05 ? ? ? ? 01 + 8
	//80 7D ? 00 74 ? C6 05 ? ? ? ? 01 + 15
	//FF 52 34 C6 05 ? ? ? ? 00 + 5
	const auto process = std::make_shared<utilities::memory::remote_process>(FNV("hwr.exe"));

	if (!process->is_valid()) {

		std::wcout << L"Couldn't locate hwr.exe" << std::endl;
		return -1;
	}

	const auto module_handle = process->get_module(FNV("hwr.exe"));

	if (!module_handle->is_valid()) {

		std::wcout << L"Internal musor error" << std::endl;
		return -1;
	}

	std::byte e_cheats[] = { std::byte(1), std::byte(0), std::byte(0) };

	auto e_cheats_addr = *(module_handle->find_pattern("80 7D ? 00 74 ? C6 05 ? ? ? ? 01") + 8);
	auto report_warnings_addr = *(module_handle->find_pattern("E9 ? ? ? ? 80 3D ? ? ? ? 00 74 3D") + 7);

	const auto prev_e_cheats = std::uintptr_t(*e_cheats_addr) & 0xff;
	const auto prev_report_warnings = std::uintptr_t(*report_warnings_addr) & 0xff;

	std::wcout << L"e_cheats == " << std::hex << prev_e_cheats << std::dec << L",  forcing it to true" << std::endl;
	std::wcout << L"report_warnings == " << std::hex << prev_report_warnings << std::dec << L",  forcing it to false" << std::endl;

	report_warnings_addr.set(reinterpret_cast<std::uintptr_t>(e_cheats + 1), sizeof(std::byte));
	e_cheats_addr.set(reinterpret_cast<std::uintptr_t>(e_cheats), sizeof(e_cheats));

	std::wcout << L"e_cheats == " << (std::uintptr_t(*e_cheats_addr) & 0xff) << L", press any key to revert it's state" << std::endl;
	std::cin.get();

	e_cheats[0] = std::byte(prev_e_cheats);
	e_cheats_addr.set(reinterpret_cast<std::uintptr_t>(e_cheats), sizeof(e_cheats));

	e_cheats[1] = std::byte(prev_report_warnings);
	report_warnings_addr.set(reinterpret_cast<std::uintptr_t>(e_cheats + 1), sizeof(std::byte));

	return 0;
}
