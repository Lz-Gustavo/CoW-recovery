//#define _CRT_SECURE_NO_WARNINGS
//#define _SCL_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <thread>
#include "libcow.h"

int main() {

	// creates the data buffer and shows it initial state
	libcow::memory* m1 = new libcow::memory(20);
	//m1->showBuffer();

	// launch 2 write operation threads
	std::thread t1(&libcow::memory::write, m1, "entrada", 2);
	std::thread t2(&libcow::memory::write, m1, "teste", 3);

	t1.join();
	t2.join();

	// shows the buffer state after the input of 2 new variables
	m1->showBuffer();

	// launch a read op. thread to trigger that page protection flag, resulting in a memory protection warning for the upnext write operation
	std::thread t3(&libcow::memory::read, m1, 2, 3);
	std::thread t4(&libcow::memory::write, m1, "opamaoi", 2);
	std::thread t5(&libcow::memory::read, m1, 2, 2);

	t3.join();
	t4.join();
	t5.join();

	//m1->showBuffer();

	std::thread t6(&libcow::memory::read, m1, 2, 2);
	
	t6.join();

	//m1->showBuffer();

	return 0;
}