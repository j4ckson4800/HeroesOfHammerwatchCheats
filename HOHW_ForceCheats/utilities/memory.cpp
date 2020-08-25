#include "memory.hpp"

namespace utilities::memory {

	bool is_code_ptr(void* ptr) {
		constexpr const DWORD protect_flags = PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY | PAGE_READWRITE | PAGE_READONLY;

		MEMORY_BASIC_INFORMATION out;
		VirtualQuery(ptr, &out, sizeof out);

		return out.Type 
			&& out.Type != MEM_PRIVATE 
			&& out.State != MEM_FREE
			&& !(out.Protect & (PAGE_GUARD | PAGE_NOACCESS))
			&& out.Protect & protect_flags;
	}
}