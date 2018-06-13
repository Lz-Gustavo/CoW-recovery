#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <time.h>
#include "libcow.h"

using libcow::memory;
using namespace std;

memory* config_memory(string file_path, vector<int> &data) {
	
	ifstream config;
	string line;
	stringstream all_text;
	string trash_aux; 
	int aux;

	config.open(file_path, ios::in);

	while (getline(config, line)) {
		all_text << line << " ";
	}

	all_text.seekg(0);
	for (int i = 0; i < 11; i++) {
		all_text >> trash_aux >> aux;
		data.push_back(aux);
	}

	config.close();
	return new memory(data[0], data[1]);
}

void generateReq(memory *&buffer, vector<int> &config, vector<thread> &list_threads) {

	// CONFIG NOTE:
	//
	// [0] -> TAM
	// [1] -> LOG
	// [2] -> OPS
	//
	// [3] -> WRITE(%)
	// [4] -> W_MIN_POS
	// [5] -> W_MAX_POS
	// [6] -> W_WAIT(sec)
	//
	// [7] -> READ(%)
	// [8] -> R_MIN_POS
	// [9] -> R_MAX_POS
	// [10] -> R_WAIT(sec)

	int i;
	srand(time(NULL));
	int pos, final_pos;

	for (i = 0; i < config[2]; i++) {

		if ((rand() % 100) < (config[3])) {
			
			sleep(config[6]);
			pos = rand() % (config[5] - config[4]) + config[4];
			list_threads.push_back(thread(&memory::write, buffer, "conteudo", pos));
		}
		else {

			sleep(config[10]);
			pos = rand() % (config[9] - config[8]) + config[8];
			final_pos = rand() % (config[9] - config[8]) + config[8];

			while (final_pos < pos) {
				pos = rand() % (config[9] - config[8]) + config[8];
				final_pos = rand() % (config[9] - config[8]) + config[8];
			}

			list_threads.push_back(thread(&memory::read, buffer, pos, final_pos));
		}
	}

	for (i = 0; i < list_threads.size(); i++)
		list_threads[i].join();

	return;
}

int main() {

	int i;
	int* data;
	string line, ext;
	vector<thread> list_threads;
	vector<int> config_data;

	memory* m1 = config_memory("config.txt", config_data);

	generateReq(m1, config_data, list_threads);

	m1->showBuffer();

	m1->showInfo();

	return 0;
}