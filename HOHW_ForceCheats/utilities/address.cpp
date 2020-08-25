#include "address.hpp"

namespace utilities::memory {
	address::address(remote_module* mod, std::uintptr_t offset)
		: module_(mod), rva_(offset) { }

	address address::operator*() {
		std::uintptr_t v;
		module_->parent()->read_memory(module_->base() + rva_, reinterpret_cast<std::uintptr_t>(&v), sizeof(std::uintptr_t));
		
		return address(module_, v - module_->base());
	}
	void address::set(std::uintptr_t buf, std::size_t size) {
		module_->parent()->write_memory(module_->base() + rva_, buf, size);
	}
}