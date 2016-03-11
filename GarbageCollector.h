/**
 * File: GarbageCollector.h
 *
 * @date 27.11.2009
 *
 * Team 5
 */
#ifndef _GARBAGE_COLLECTOR_H_
#define _GARBAGE_COLLECTOR_H_

#include <iostream>
#include <list>
#include <cstdlib>
#include <typeinfo>
using namespace std;

#define DEBUG_

#ifdef DEBUG
#define LOG(msg) cout << "GC: " << msg << endl
#else
#define LOG(msg)
#endif

/**
 * Namespace for GarbageCollector
 */
namespace GarbageCollector {

template<typename T> class Iterator {

		// points to the current entry
		T *pointer;
		// points to the position after the last entry
		T *end;
		// points to the first entry
		T *start;
		// size of the list
		unsigned length;

	public:
		/**
		 * Default (empty) constructor
		 */
		Iterator() {
			pointer = end = start = NULL;
			length = 0;
		}
		/**
		 * Constructor for an array
		 */
		Iterator(T *p, T * first, T *last) {
			pointer = p;
			end = last;
			start = first;
			length = last - first;
		}
		/**
		 * Getter for the size
		 */
		unsigned size() {
			return length;
		}
		/**
		 * Getter for the content of the pointer.
		 * To not break STL behavior, we do not check for an illegal access.
		 */
		T &operator*() {
			return *pointer;
		}
		/**
		 * Getter for the content of the pointer for method calls.
		 * To not break STL behavior, we do not check for an illegal access.
		 */
		T *operator->() {
			return pointer;
		}
		/**
		 * Prefix increment operator.
		 */
		Iterator operator++() {
			pointer++;
			return *this;
		}
		/**
		 * Prefix decrement operator.
		 */
		Iterator operator--() {
			pointer--;
			return *this;
		}
		/**
		 * Postfix increment operator.
		 */
		Iterator operator++(int lhs) {
			T *temp = pointer;
			pointer++;
			return Iterator<T> (temp, start, end);
		}
		/**
		 * Postfix decrement operator.
		 */
		Iterator operator--(int lhs) {
			T *temp = pointer;
			pointer--;
			return Iterator<T> (temp, start, end);
		}
		/**
		 * Return a reference to a specific index in the array.
		 * To not break STL behavior, we do not check for an illegal access.
		 */
		T & operator[](int index) {
			return pointer[index];
		}
		/**
		 * Compare equal operator.
		 */
		bool operator==(Iterator other) {
			return pointer == other.pointer;
		}
		/**
		 * Compare not equal operator.
		 */
		bool operator!=(Iterator other) {
			return pointer != other.pointer;
		}
		/**
		 * Compare less than operator.
		 */
		bool operator<(Iterator other) {
			return pointer < other.pointer;
		}
		/**
		 * Compare less than or equal operator.
		 */
		bool operator<=(Iterator other) {
			return pointer <= other.pointer;
		}
		/**
		 * Compare greater than operator.
		 */
		bool operator>(Iterator other) {
			return pointer > other.pointer;
		}
		/**
		 * Compare greater than or equal operator.
		 */
		bool operator>=(Iterator other) {
			return pointer >= other.pointer;
		}
		/**
		 * Subtract operator.
		 */
		Iterator operator-(int n) {
			pointer -= n;
			return *this;
		}
		/**
		 * Addition operator.
		 */
		Iterator operator+(int n) {
			pointer += n;
			return *this;
		}
		/**
		 * Values between operator
		 */
		int operator-(Iterator<T> & other) {
			return pointer - other.pointer;
		}
}; //end of class Iterator

/**
 * Class PointerData
 *
 * To store information according to an pointer for use with the garbage
 * collector.
 */
template<typename T> class PointerData {
	public:
		// reference counter
		unsigned ref_count;
		// raw pointer
		T *raw_pointer;
		// is_array
		bool is_array;
		// size of the array. if is_array = true
		unsigned array_size;
		/**
		 * Constructor with the pointer to the allocated memory
		 */
		PointerData(T * pointer, unsigned size = 0) {
			ref_count = 1;
			raw_pointer = pointer;
			(size != 0) ? is_array = true : is_array = false;
			array_size = size;
		}
}; //end of class PointerData

/**
 * The overloaded compare equal than operator for the usage of the STL implementation
 * of list, vector etc.
 */
template<typename T> bool operator==(const PointerData<T> &first,
        const PointerData<T> &second) {
	return (first.raw_pointer == second.raw_pointer);
}

/**
 * Class Pointer
 *
 * A smart pointer implementation which is used for dynamically allocated
 * memory. If this is smart pointer points to an array, the size has to be
 * specified to indicate this pointer points to an array.
 */
template<typename T, int size = 0> class Pointer {
		// gc_list holding all the PointerData objects for garbace collection
		static list<PointerData<T> > gc_list;
		// pointer to the address of the "real" object in memory
		T * addr;
		// if is_array = true, we are holding the size of the array
		unsigned array_size;
		// true when first pointer is created
		static bool first;
		// iterator pointer to the garbage collection list
		typename list<PointerData<T> >::iterator findPointerData(T * pointer) {
			typename list<PointerData<T> >::iterator p;
			// Find pointer in gc_list
			for (p = gc_list.begin(); p != gc_list.end(); p++) {
				if (p->raw_pointer == pointer)
					return p;
			}
			return p;
		}
	public:
		/**
		 * set the collect cycle for this type of pointers
		 * however, on shutdown, we collect anyway everything
		 * WARNING: use this carefully! default = 0
		 */
		static unsigned collect_cycle;
		// Iterator for the specific pointer type
		typedef Iterator<T> GarbageCollectorIterator;
		// Construct both initialized and uninitialized objects.
		Pointer(T * obj = NULL) {
			typename list<PointerData<T> >::iterator pointer;

			pointer = findPointerData(obj);
			/* If this pointer is already in the garbage collector, just increase
			 * the reference counter. Else we create a new entry.
			 */
			if (pointer != gc_list.end()) {
				pointer->ref_count++;
			} else {
				PointerData<T> data(obj, size);
				gc_list.push_front(data);
			}
			addr = obj;
			array_size = size;

			LOG("New GarbageCollector pointer.");
			if (array_size > 0)
				LOG("Size is " << array_size);

			//register shutdown() as an exit function.
			if (first)
				atexit(shutdown);
			first = false;
		}
		/**
		 * Copy constructor.
		 */
		Pointer(const Pointer &other) {
			typename list<PointerData<T> >::iterator p;
			p = findPointerData(other.addr);
			p->ref_count++; //increment ref count
			addr = other.addr;
			array_size = other.array_size;

			LOG("Constructing GarbageCollector pointer copy.");
			if (array_size > 0)
				LOG("Size is " << array_size);
		}

		/**
		 * Destructor
		 */
		~Pointer() {
			typename list<PointerData<T> >::iterator p = findPointerData(addr);
			if (p->ref_count)
				p->ref_count--;
			LOG("GarbageCollector pointer going out of scope.");
			if (p->ref_count == 0)
				collect();
		}
		/**
		 * Runs the garbage collector cycle and collect freed memory
		 */
		static void collect() {
			static unsigned counter = 0;
			// just run the collect() method every set collect cycle
			if (counter < Pointer<T>::collect_cycle) {
				counter++;
				return;
			}

			// reset the counter
			counter = 0;

			LOG("Before garbage collection");
#ifdef DEBUG
			printList();
#endif

			typename list<PointerData<T> >::iterator p = gc_list.begin();
			// Scan gc_list looking for unreferenced pointers
			while (p != gc_list.end()) {
				// If in use, skip.
				if (p->ref_count == 0) {
					// Free memory unless the Pointer is null.
					if (p->raw_pointer) {
						if (p->array_size > 0) {
							LOG("Deleting array of size " << p->array_size);
							delete[] p->raw_pointer; // delete array
						} else {
							LOG("Deleting: " << *(T*) p->raw_pointer);
							delete p->raw_pointer; // delete single element
						}
					}
					// after 'delete' remove info. obj. from the list
					p = gc_list.erase(p);
				} else {
					p++; // in use, next element
				}
			}

			LOG("After garbage collection");
#ifdef DEBUG
			printList();
#endif

		}
		/**
		 * Assign operator.
		 */
		T *operator=(T* t) {
			typename list<PointerData<T> >::iterator p;
			// decrement the reference count for the memory
			// currently being pointed to.
			p = findPointerData(addr);
			p->ref_count--;
			// if the new address already existent in the system,
			// increment its count. Otherwise, create a new entry for gc_list.
			p = findPointerData(t);
			if (p != gc_list.end())
				p->ref_count++;
			else {
				// create and stores the entry
				PointerData<T> GarbageCollectorObj(t, size);
				gc_list.push_front(GarbageCollectorObj);
			}
			addr = t;
			return t;
		}
		/**
		 * Address operator.
		 */
		Pointer &operator=(Pointer &other) {
			typename list<PointerData<T> >::iterator pointer;
			pointer = findPointerData(addr);
			pointer->ref_count--;
			// Next, increment the reference count of the new address
			pointer = findPointerData(other, addr);
			pointer->ref_count++; // increment ref count
			addr = other.addr; // store the address
			return other;
		}
		/**
		 * Reference operator.
		 */
		T &operator*() {
			return *addr;
		}
		/**
		 * Pointer operator.
		 */
		T *operator->() {
			return addr;
		}
		/**
		 * Reference to a specific index operator
		 */
		T &operator[](int index) {
			return addr[index];
		}
		/**
		 * Content operator.
		 */
		operator T *() {
			return addr;
		}
		/**
		 * Returns an iterator pointing to the begin of an array.
		 */
		Iterator<T> begin() {
			return Iterator<T> (addr, addr, addr
			        + ((array_size > 0) ? array_size : 1));
		}
		/**
		 * Returns an iterator pointing to the end of an array
		 */
		Iterator<T> end() {
			return Iterator<T> (addr + ((array_size > 0) ? array_size : 1), addr,
			        addr + ((array_size > 0) ? array_size : 1));
		}
		/**
		 * Return the size of the garbage collector list.
		 */
		static int GarbageCollectorListSize() {
			return gc_list.size();
		}
		/**
		 * Print the garbage collector list.
		 */
		static void printList() {
			typename list<PointerData<T> >::iterator p;
			cout << "GarbageCollectorlist<" << typeid(T).name() << ", " << size
			        << ">;\n";
			cout << "raw_pointer       ref_count     value\n";
			if (gc_list.begin() == gc_list.end()) {
				cout << "           -- Empty  -- \n\n";
				return;
			}

			for (p = gc_list.begin(); p != gc_list.end(); p++) {
				cout << "[" << (void *) p->raw_pointer << "]" << "       "
				        << p->ref_count << "     ";
				if (p->raw_pointer)
					cout << " " << *p->raw_pointer;
				else
					cout << " ---";
				cout << endl;
			}
			cout << endl;
		}
		/**
		 * Shutdown function to erase all the data hold in the garbage collector
		 * list.
		 */
		static void shutdown() {
			LOG("Quit function called");

			if (gc_list.size() == 0)
				return; // the list is empty
			typename list<PointerData<T> >::iterator p;
			for (p = gc_list.begin(); p != gc_list.end(); p++) {
				// Set all reference counts to zero
				p->ref_count = 0;
			}

			// set collect cycle to 0 to be sure that we free all the memory
			Pointer<T>::collect_cycle = 0;
#ifdef DEBUG
			LOG("Before collecting for shutdown() for " << typeid(T).name());
#endif
			collect();
#ifdef DEBUG
			LOG("After collecting for shutdown() for " << typeid(T).name());
#endif
		}
};//end class Pointer

// Initialize all the static variables
template<typename T, int size>
list<PointerData<T> > Pointer<T, size>::gc_list;
template<typename T, int size>
bool Pointer<T, size>::first = true;
template<typename T, int size>
unsigned Pointer<T, size>::collect_cycle = 0;

} // namespace GarbageCollector

#endif // _GARBAGE_COLLECTOR_H_
