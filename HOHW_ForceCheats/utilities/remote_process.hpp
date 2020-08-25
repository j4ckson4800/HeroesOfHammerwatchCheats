#pragma once
#include "remote_module.hpp"

namespace utilities::memory {
	class remote_process {
	protected:
		fnv::hash name_;
		_handle process_;
		std::size_t process_id_;
	public:
		remote_process();
		remote_process(fnv::hash name);
	public:
		~remote_process();
	public:
		std::shared_ptr<remote_module> get_module(fnv::hash name);
	public:
		void read_memory(std::uintptr_t base, std::uintptr_t buffer, std::size_t size);
		void write_memory(std::uintptr_t base, std::uintptr_t buffer, std::size_t size);
	protected:
		std::size_t get_process_id(fnv::hash name);
		std::uintptr_t get_module_base(const fnv::hash module);
	public:
		inline auto& handle() noexcept { return process_; }
		inline auto is_valid() const noexcept { return process_ != INVALID_HANDLE_VALUE; }
	};
}