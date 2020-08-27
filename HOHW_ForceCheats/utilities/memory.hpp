#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include "fnv_hash.hpp"

struct datamap_t;

namespace utilities::memory {
	static inline int32_t rva_2_offset(const uint32_t rva, PIMAGE_NT_HEADERS nt_headers, const bool in_memory = false) {
		if (rva == 0 || !in_memory)
			return rva;

		auto sec = IMAGE_FIRST_SECTION(nt_headers);
		for (size_t i = 0; i < nt_headers->FileHeader.NumberOfSections; i++) {
			if (rva >= sec->VirtualAddress && rva < sec->VirtualAddress + sec->Misc.VirtualSize)
				break;
			sec++;
		}

		return rva - sec->VirtualAddress + sec->PointerToRawData;
	}
	static uintptr_t get_proc_address(const uintptr_t module, const fnv::hash function, const bool in_memory = false) {
		const auto dos_headers = reinterpret_cast<IMAGE_DOS_HEADER*>(module);
		if (dos_headers->e_magic != IMAGE_DOS_SIGNATURE)
			return 0;

		const auto nt_headers = reinterpret_cast<IMAGE_NT_HEADERS*>(dos_headers->e_lfanew + module);
		if (nt_headers->Signature != IMAGE_NT_SIGNATURE)
			return 0;

		const auto exports = reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(
			rva_2_offset(nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress,
				nt_headers, in_memory) + module);

		const auto names = reinterpret_cast<uint32_t*>(rva_2_offset(exports->AddressOfNames, nt_headers, in_memory) + module);

		auto ordinal_index = static_cast<uint32_t>(-1);
		for (uint32_t i = 0; i < exports->NumberOfFunctions; i++) {
			const auto function_name = reinterpret_cast<const char*>(rva_2_offset(names[i], nt_headers, in_memory) + module);

			if (fnv::hash_runtime(function_name) == function) {
				ordinal_index = i;
				break;
			}
		}

		if (ordinal_index > exports->NumberOfFunctions)
			return 0;

		const auto ordinals = reinterpret_cast<uint16_t*>(rva_2_offset(exports->AddressOfNameOrdinals, nt_headers, in_memory) + module);
		const auto addresses = reinterpret_cast<uint32_t*>(rva_2_offset(exports->AddressOfFunctions, nt_headers, in_memory) + module);
		return rva_2_offset(addresses[ordinals[ordinal_index]], nt_headers, in_memory) + module;
	}
	template <std::size_t index, typename return_type, typename... function_args>
	inline static auto get_virtual(void* instance, function_args&& ... args) -> return_type {
		return ((*reinterpret_cast<return_type(__thiscall***)(void*, function_args...)>(instance))[index])(instance, std::forward<function_args>(args)...);
	}
	template<typename function_type>
	inline static auto get_virtual(void* class_ptr, int index) -> function_type {
		return static_cast<function_type>((*static_cast<void***>(class_ptr))[index]);
	}
	extern bool is_code_ptr(void* ptr);
}
