#pragma once

# include <map>
# include <cstddef>

namespace _SmartPointerDetail_
{

typedef std::map<void *, unsigned int> SmartPointerMapType;
extern SmartPointerMapType _reference_tracker;

}


template <typename T>
class SmartPointer
{
	private:
		typedef T value_type;
		typedef const T const_value_type;
		typedef T* pointer;
		typedef const T* const_pointer;
		typedef T& reference;
		typedef const T& const_reference;
		typedef std::size_t size_type;

		typedef SmartPointer<const_value_type> const_smart_pointer;

	public:
		SmartPointer()
		: p(NULL) {}
		SmartPointer(pointer p)
		: p(p) {
			incrementReference();
		}

		SmartPointer(const SmartPointer& rhs)
		: p(rhs.p) {
			incrementReference();
		}

		SmartPointer& operator=(const SmartPointer& rhs) {
			if (this == &rhs) {
				return *this;
			}
			decrementReference();
			p = rhs.p;
			incrementReference();
			return *this;
		}

		~SmartPointer() {
			if (p == NULL) {
				return;
			}
			decrementReference();
			if (getCount() == 0) {
				delete p;
			}
		}

		reference operator*() {
			return *p;
		}

		const_reference operator*() const {
			return *p;
		}

		pointer operator->() {
			return p;
		}

		const_pointer operator->() const {
			return p;
		}

		reference operator[](size_type n) {
			return p[n];
		}

		const_reference operator[](size_type n) const {
			return p[n];
		}

		operator const_smart_pointer() const {
			return const_smart_pointer(p);
		}

	private:

		void incrementReference() const {
			if (p == NULL) {
				return;
			}
			_SmartPointerDetail_::_reference_tracker[(void*)(p)] += 1;
		}

		void decrementReference() const {
			if (p == NULL) {
				return;
			}
			_SmartPointerDetail_::_reference_tracker[(void*)(p)] -= 1;
		}

		unsigned int getCount() const {
			return _SmartPointerDetail_::_reference_tracker[(void*)(p)];
		}

	private:
		pointer p;
};
