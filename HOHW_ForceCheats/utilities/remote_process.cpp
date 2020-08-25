#include <cwctype>

#include "remote_process.hpp"

namespace utilities::memory {

	remote_process::remote_process()
		: process_(INVALID_HANDLE_VALUE), name_(std::numeric_limits<fnv::hash>::max()), process_id_(std::numeric_limits<fnv::hash>::max()){ }

	remote_process::remote_process(fnv::hash name)
		: name_(name), process_id_(get_process_id(name)) {
		process_ = OpenProcess(PROCESS_ALL_ACCESS, 0, process_id_); 
	}

	remote_process::~remote_process() {
		CloseHandle(process_);
	}

	std::uintptr_t remote_process::get_module_base(const fnv::hash module) {
		MODULEENTRY32 entry;
		entry.dwSize = sizeof(MODULEENTRY32);

		std::uintptr_t base_addr = 0;
		
		if (const auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, process_id_)) {

			if (Module32First(snapshot, &entry))
				do {
					std::wstring name_(entry.szModule);
					std::string name(name_.begin(), name_.end());
					std::transform(name.begin(), name.end(), name.begin(), ::tolower);

					if (fnv::hash_runtime(name.c_str()) == module) {

						base_addr = reinterpret_cast<uintptr_t>(entry.modBaseAddr);
						break;
					}
				} while (Module32Next(snapshot, &entry));

				CloseHandle(snapshot);
		}
		return base_addr;
	}

	std::shared_ptr<remote_module> remote_process::get_module(fnv::hash name) {
		return std::make_shared<remote_module>(this, get_module_base(name));
	}

	std::size_t remote_process::get_process_id(fnv::hash name) {
		PROCESSENTRY32 entry;
		entry.dwSize = sizeof(PROCESSENTRY32);

		std::uint32_t process_id_ = -1;

		if (const auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL)) {

			if (Process32First(snapshot, &entry))
				do {
					std::wstring cur_exe_name = entry.szExeFile;
					std::transform(cur_exe_name.begin(), cur_exe_name.end(), cur_exe_name.begin(), std::towlower);

					if (fnv::hash_runtime(std::string(cur_exe_name.begin(), cur_exe_name.end()).c_str()) == name) {

						process_id_ = entry.th32ProcessID;
						break;
					}

					entry.dwSize = sizeof(PROCESSENTRY32);
				} while (Process32Next(snapshot, &entry));

			CloseHandle(snapshot);
		}
		return process_id_;
	}

	void remote_process::write_memory(std::uintptr_t base, std::uintptr_t buffer, std::size_t size) {
		if(!WriteProcessMemory(process_, reinterpret_cast<LPVOID>(base), reinterpret_cast<LPVOID>(buffer), size, nullptr))
			printf_s("WriteProcessMemory: %d\n", GetLastError());
	}

	void remote_process::read_memory(std::uintptr_t base, std::uintptr_t buffer, std::size_t size) {
		if (!ReadProcessMemory(process_, reinterpret_cast<LPVOID>(base), reinterpret_cast<LPVOID>(buffer), size, nullptr))
			printf_s("ReadProcessMemory: %d\n", GetLastError());
	}
}