#pragma once
#include <vector>
#include <memory>
#include <string_view>

#include <Windows.h>
#include <TlHelp32.h>

#include "fnv_hash.hpp"

typedef HANDLE _handle;
typedef IMAGE_DOS_HEADER _dos_headers;
typedef IMAGE_NT_HEADERS64 _nt_headers;
typedef MEMORY_BASIC_INFORMATION _mbi;

namespace utilities::memory {
	class remote_process;
	class address;

	class remote_module {
	protected:
		remote_process* parent_;
		std::uintptr_t module_base_;
	public:
		remote_module();
		remote_module(remote_process* parent, std::uintptr_t handle);
	public:
		~remote_module();
	public:
		address find_pattern(std::string_view pattern);
	protected:
		std::shared_ptr<_dos_headers> get_dos_headers();
		std::shared_ptr<_nt_headers> get_nt_headers();
	protected:
		std::uintptr_t find_pattern(std::uint8_t* buffer, std::size_t size, std::string_view pattern);
		std::vector<int> pattern_to_byte(std::string_view pattern);
	public:
		inline auto is_valid() const noexcept { return module_base_ != 0; }
		inline auto base() const noexcept { return module_base_; }
		inline auto parent() noexcept { return parent_; }
	};
}