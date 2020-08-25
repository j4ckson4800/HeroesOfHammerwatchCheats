#pragma once
#include "remote_process.hpp"

namespace utilities::memory {
	class address {
		template<class T>
		friend address operator+(const address& l, T offset);
		template<class T>
		friend address operator-(const address& l, T offset);
	protected:
		remote_module* module_;
		std::uintptr_t rva_;
	public:
		address(remote_module* mod, std::uintptr_t offset);
	public:
		void set(std::uintptr_t buf, std::size_t size);
		address operator*();
	public:
		inline operator std::uintptr_t() {
			return rva_ + module_->base();
		}
		inline address& operator+=(std::uintptr_t offset) {
			rva_ += offset;
			return *this;
		}
		inline address& operator-=(std::uintptr_t offset) {
			rva_ -= offset;
			return *this;
		}
	};
	template<class T>
	static inline address operator+(const address& l, T offset) {
		return address(l.module_, offset + l.rva_);
	}
	template<class T>
	static inline address operator-(const address& l, T offset) {
		return address(l.module_, offset - l.rva_);
	}
}