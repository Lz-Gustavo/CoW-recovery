//#define _CRT_SECURE_NO_WARNINGS
//#define _SCL_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <vector>
#include "libcow.h"

using libcow::memory;
using namespace std;

int* extract(string line) {
	//receives the input line from the line and returns an int array containing both param
	
	int i, pos_comma;
	int *aux = new int[3];
	string ext;

	if (line.compare(0, 5, "write") == 0) {

		aux[0] = 0;
		pos_comma = (int) line.find(',', 6);
		for (i = (pos_comma + 1); line[i] != ')'; i++ )
			ext.push_back(line[i]);

		aux[1] = stoi(ext, nullptr, 10);
	}
	else {

		aux[0] = 1;
		
		for (i = (int) (line.find('(') + 1); line[i] != ','; i++)
			ext.push_back(line[i]);

		aux[1] = stoi(ext, nullptr, 10);
		ext.clear();

		for (i += 1; line[i] != ')'; i++)
			ext.push_back(line[i]);

		aux[2] = stoi(ext, nullptr, 10);
	}
	return aux;
}

string extract_string(string line) {
	
	return (line.substr(line.find('\"') + 1, (line.find(',') - line.find('\"')) - 2));
}

int main() {

	int i;
	int* data;
	ifstream input;
	string line, ext;
	vector<thread> list_threads;

	// creates the data buffer and shows it initial state
	memory* m1 = new memory(20);
	
	input.open("input.txt", ios::in);

	while (getline(input, line)) {

		data = extract(line);
		if (data[0] == 0) {

			ext = extract_string(line);
			//cout << "extraiu: " << ext << endl;

			list_threads.push_back(thread(&memory::write, m1, ext, data[1]));
		}
		else if (data[0] == 1) {
			list_threads.push_back(thread(&memory::read, m1, data[1], data[2]));
		}
	}

	for (i = 0; i < list_threads.size(); i++)
		list_threads[i].join();

	return 0;
}