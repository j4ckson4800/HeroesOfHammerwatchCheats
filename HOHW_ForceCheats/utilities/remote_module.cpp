#include <algorithm>

#include "address.hpp"
#include "remote_process.hpp"

namespace utilities::memory {

	remote_module::remote_module()
		: module_base_(std::numeric_limits<std::uintptr_t>::max()), parent_(nullptr) { }

	remote_module::remote_module(remote_process* parent, std::uintptr_t base)
		: parent_(parent), module_base_(base) { }

	remote_module::~remote_module() { }

	address remote_module::find_pattern(std::string_view pattern) {
		const auto nt_hd = get_nt_headers();

		std::uint64_t size_of_image = 0;

		if (std::reinterpret_pointer_cast<IMAGE_NT_HEADERS32>(nt_hd)->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
			size_of_image = std::reinterpret_pointer_cast<IMAGE_NT_HEADERS32>(nt_hd)->OptionalHeader.SizeOfImage;
		else 
			size_of_image = nt_hd->OptionalHeader.SizeOfImage;
		
		_mbi mbi = { 0 };

		for (std::uintptr_t i{ module_base_ }; i < module_base_ + size_of_image; i += mbi.RegionSize) {

			if (!VirtualQueryEx(parent_->handle(), reinterpret_cast<LPCVOID>(i), &mbi, sizeof(mbi)))
				return address(this, 0);

			if (mbi.State != MEM_COMMIT || mbi.Protect == PAGE_NOACCESS)
				continue;

			auto buffer = new std::uint8_t[mbi.RegionSize];
			
			parent_->read_memory(reinterpret_cast<std::uintptr_t>(mbi.BaseAddress), reinterpret_cast<std::uintptr_t>(buffer), mbi.RegionSize);
			const auto offset = find_pattern(buffer, mbi.RegionSize, pattern);

			delete[] buffer;

			if (offset != std::numeric_limits<std::uintptr_t>::max())
				return address(this, reinterpret_cast<std::uintptr_t>(mbi.BaseAddress) + offset - module_base_);
		}

		return address(this, 0);
	}

	std::shared_ptr<_dos_headers> remote_module::get_dos_headers() {
		auto dos_hd = std::make_shared<_dos_headers>();
		parent_->read_memory(module_base_, reinterpret_cast<std::uintptr_t>(dos_hd.get()), sizeof(_dos_headers));

		return dos_hd;
	}

	std::shared_ptr<_nt_headers> remote_module::get_nt_headers() {

		const auto dos_hd = get_dos_headers();
		const auto nt_hd = std::make_shared<_nt_headers>();
		parent_->read_memory(module_base_ + dos_hd->e_lfanew, reinterpret_cast<std::uintptr_t>(nt_hd.get()), sizeof(_nt_headers));

		return nt_hd;
	}

	std::uintptr_t remote_module::find_pattern(std::uint8_t* buffer, std::size_t size, std::string_view pattern) {
		const auto pattern_bytes = pattern_to_byte(pattern.data());

		const auto s = pattern_bytes.size();
		const auto d = pattern_bytes.data();

		std::size_t current_byte = 0;
		const auto it = std::find_if(buffer, reinterpret_cast<std::uint8_t*>(buffer + size - s), [&](auto&& b) {
			if (d[current_byte] == b || d[current_byte] == std::numeric_limits<std::uintptr_t>::max()) current_byte++;
			else current_byte = 0;

			return (current_byte == s);
		});

		if (it == buffer + size - s)
			return std::numeric_limits<std::uintptr_t>::max();

		return reinterpret_cast<std::uintptr_t>(it) - s + 1 - reinterpret_cast<std::uintptr_t>(buffer);
	}

	std::vector<int> remote_module::pattern_to_byte(std::string_view pattern) {
		auto bytes = std::vector<int>{};
		auto start = const_cast<char*>(pattern.data());
		auto len = pattern.length();
		auto end = const_cast<char*>(start) + len;
		bytes.reserve(len / 3 + 5);

		for (auto current = start; current < end; ++current) {
			if (*current == '?') {
				++current;

				if (*current == '?')
					++current;

				bytes.emplace_back(-1);
			}
			else
				bytes.emplace_back(strtoul(current, &current, 16));
		}
		return bytes;
	}
}