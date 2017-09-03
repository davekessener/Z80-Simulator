#ifndef LIB_PROPERTY_H
#define LIB_PROPERTY_H

#include <memory>
#include <vector>
#include <functional>

namespace lib
{
	template<typename T>
	class Property
	{
		public:
		typedef std::function<void(T&)> listener_fn;

		public:
			Property(T *t) : v_(t), listeners_(new std::vector<listener_fn>) { }
			void listen(listener_fn f) { listeners_->push_back(f); }
			void set(const T& v) { *v_ = v; for(listener_fn& f : *listeners_) { f(*v_); } }
			T get( ) const { return *v_; }
		private:
			std::shared_ptr<T> v_;
			std::shared_ptr<std::vector<listener_fn>> listeners_;
	};
}

#endif

