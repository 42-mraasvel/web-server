#pragma once

# include <map>

template <typename T>
class SmartPointer
{
	private:
		typedef T value_type;
		typedef T* pointer;
		typedef T& reference;
		typedef std::size_t size_type;

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
			if (reference_tracker[p] == 0) {
				delete p;
			}
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

		void incrementReference() const {
			if (p == NULL) {
				return;
			}
			reference_tracker[p] += 1;
		}

		void decrementReference() const {
			if (p == NULL) {
				return;
			}
			reference_tracker[p] -= 1;
		}

	private:
		typedef std::map<pointer, unsigned int> MapType;
		static MapType reference_tracker;

		pointer p;
};

template <typename T>
typename SmartPointer<T>::MapType SmartPointer<T>::reference_tracker;
