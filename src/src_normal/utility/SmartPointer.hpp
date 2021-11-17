#pragma once

template <typename T>
class SmartPointer
{
	private:
		typedef T value_type;
		typedef T* pointer;
		typedef T& reference;

	public:
		explicit SmartPointer(pointer p = NULL)
		: p(p) {}

		~SmartPointer() {
			delete p;
		}

		reference operator*() {
			return *p;
		}

		pointer operator->() {
			return p;
		}

		reference operator[](size_type n) {
			return p[n];
		}

	private:
		pointer p;
};
