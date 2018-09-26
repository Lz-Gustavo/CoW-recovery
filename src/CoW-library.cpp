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

void sleep_ms(int milliseconds) {

	struct timespec ts;
	ts.tv_sec = milliseconds / 1000;
	ts.tv_nsec = (milliseconds % 1000) * 1000000;
	nanosleep(&ts, NULL);
}

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
	for (int i = 0; i < 14; i++) {
		all_text >> trash_aux >> aux;
		data.push_back(aux);
	}

	config.close();
	return new memory(data[0], data[1]);
}

void worker_generateReq(memory *&buffer, vector<int> config) {

	// CONFIG NOTE:
	//
	// [0] -> TAM
	// [1] -> LOG
	// [2] -> OPS
	// [3] -> WORKERS
	//
	// [4] -> WRITE(%)
	// [5] -> W_MIN_POS
	// [6] -> W_MAX_POS
	// [7] -> W_WAIT(sec)
	//
	// [8] -> READ(%)
	// [9] -> R_MIN_POS
	// [10] -> R_MAX_POS
	// [11] -> R_WAIT(sec)
	// 
	// [12] -> T_TIME
	// [13] -> SUPERVISOR

	int i;
	srand(time(NULL));
	int pos, final_pos;

	if (config[13] <= 0) {
		for (i = 0; i < config[2]; i++) {

			// thinking time 0 to max specified
			//sleep_ms((int) rand() % config[12]);
			sleep_ms((int) config[12]);

			if ((rand() % 100) < (config[4])) {
				
				pos = rand() % ((config[6] + 1) - config[5]) + config[5];
				//list_threads.push_back(thread(&memory::write, buffer, "conteudo", pos));
				buffer->write("conteudo", pos);
				sleep(config[7]);
			}
			else {

				do {

					pos = rand() % ((config[10] + 1) - config[9]) + config[9];
					final_pos = rand() % ((config[10] + 1) - config[9]) + config[9];
				} while (final_pos < pos);

				buffer->read(pos, final_pos);
				//list_threads.push_back(thread(&memory::read, buffer, pos, final_pos));
				sleep(config[11]);
			}
		}
	}
	else {
		for (;;) {
			
			sleep_ms((int) config[12]);

			if ((rand() % 100) < (config[4])) {
				
				pos = rand() % ((config[6] + 1) - config[5]) + config[5];
				buffer->write("conteudo", pos);
				sleep(config[7]);
			}
			else {

				do {

					pos = rand() % ((config[10] + 1) - config[9]) + config[9];
					final_pos = rand() % ((config[10] + 1) - config[9]) + config[9];
				} while (final_pos < pos);

				buffer->read(pos, final_pos);
				sleep(config[11]);
			}
		}
	}

	return;
}

void generateWorkers(memory *&buffer, vector<int> &config, vector<thread> &list_threads) {
	int i;

	for (i = 0; i < config[3]; i++) {

		list_threads.push_back(thread(worker_generateReq, ref(buffer), config));
	}

	
	if (config[13] > 0) {

		sleep(config[13]);

		for (i = 0; i < list_threads.size(); i++) {
			list_threads[i].detach();
			list_threads[i].~thread();
		}
	}
	else
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

	memory* m1 = config_memory("../config.txt", config_data);

	generateWorkers(m1, config_data, list_threads);

	//m1->showBuffer();

	m1->showInfo();

	delete m1;

	return 0;
}