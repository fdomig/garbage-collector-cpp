#include <iostream>
#include "GarbageCollector.h"
#include <string.h>
#include <sys/timeb.h>
#include <vector>

#define SIZE 500000

using namespace std;
using namespace GarbageCollector;

void test();
void testWithGC();

int get_ms() {
	struct timeb timebuffer;
	ftime(&timebuffer);
	return (timebuffer.time * 1000) + timebuffer.millitm;
}

int main(int argc, char * argv[]) {

	cout << "Garbage Collector Test" << endl;
	int start, stop;

	cout << "*** Test without GC ***" << endl;
	start = get_ms();
	test();
	stop = get_ms();
	cout << "--- Test done: Insert " << SIZE << " elements with delete in ";
	cout << (float) (stop - start) / 1000.0 << " s. ---" << endl;

	cout << "*** Test with GC ***" << endl;
	start = get_ms();
	testWithGC();
	stop = get_ms();
	cout << "--- Test done: Insert " << SIZE << " elements with delete in ";
	cout << (float) (stop - start) / 1000.0 << " s. ---" << endl;

	return 0;
}

void testWithGC() {
    for (int i = 0; i < SIZE; i++) {
    	Pointer<string> s = new string("Test");
	}
}

void test() {
	for (int i = 0; i < SIZE; i++) {
		string* s = new string("Test");
		delete s;
	}
}
