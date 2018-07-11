#ifndef LIB_COW_H
#define LIB_COW_H

#include <iostream>
#include <string>
#include <string.h>
#include <thread>
#include <semaphore.h>
#include <unistd.h>
#define INPUT_SIZE 10
#define DEFAULT_S 4096

namespace libcow {
	
	class memory {
	private:
		int *bitmap, *modified, *protection;
		char **buffer, **copies;
		int size, log;
		sem_t acess_data;
		sem_t data;

		int page_faults, mem_usage, num_alloc_copy, num_dealloc_copy;

		char** getBuffer(int len, int offset) {
			// returns a char array inside the major buffer
			int i, j;

			char ** aux = new char*[len];
			for (i = 0; i < len; i++)
				aux[i] = new char[INPUT_SIZE];

			j = 0;
			for (i = offset; i < len+offset; i++) {
				aux[j] = buffer[i];
				j++;
			}
			return aux;
		}

	public:
		memory() {
			buffer = new char*[DEFAULT_S];
			for (int i = 0; i < DEFAULT_S; i++)
				buffer[i] = new char[INPUT_SIZE]();

			copies = new char*[DEFAULT_S];
			for (int i = 0; i < DEFAULT_S; i++)
				copies[i] = nullptr;

			bitmap = new int[DEFAULT_S]();
			modified = new int[DEFAULT_S]();
			protection = new int[DEFAULT_S]();
			size = DEFAULT_S;
			log = 0;
			sem_init(&acess_data, 0, 1);
			sem_init(&data, 0, 1);

			page_faults = 0;
			mem_usage = 0;
			num_alloc_copy = 0;
			num_dealloc_copy = 0;
		}
		memory(int n, int log_flag) {
			buffer = new char*[n];
			for (int i = 0; i < n; i++)
				buffer[i] = new char[INPUT_SIZE]();

			copies = new char*[n];
			for (int i = 0; i < n; i++)
				copies[i] = nullptr;

			bitmap = new int[n]();
			modified = new int[n]();
			protection = new int[n]();
			size = n;
			log = log_flag;
			sem_init(&acess_data, 0, 1);
			sem_init(&data, 0, 1);

			page_faults = 0;
			mem_usage = 0;
			num_alloc_copy = 0;
			num_dealloc_copy = 0;
		}
		~memory() {
			int i;
			for (i = 0; i < size; i++)
				delete[] buffer[i];
			delete[] buffer;
			for (i = 0; i < size; i++)
				delete[] copies[i];
			delete[] copies;
			delete[] bitmap;
			delete[] modified;
			delete[] protection;
			sem_destroy(&acess_data);
			sem_destroy(&data);
		}

		int write(std::string x, int p) {
			//inner offset for each row
			int os = 0;

			if ((os + x.size()) > INPUT_SIZE) {
				std::cerr << "Not enough space in input cell." << std::endl;
				return 0;
			}
			if ((p < 0) || (p >= size)) {
				std::cerr << "Insert a valid position." << std::endl;
				return 0;
			}
			sem_wait(&acess_data);

			if ((protection[p] == 1) && (copies[p] != nullptr)) {
				std::cerr << "Insufficient memory avaiable to allocate a new copy at " << p << ", clear some data to continue." << std::endl;
				sem_post(&acess_data);
				return 0;
			}
			
			if (protection[p] == 0) {
				// buffer position is not protected
				sem_post(&acess_data);

				sem_wait(&data);
				if (log) {
					std::cout << std::endl << "========WRITE OPERATION========" << std::endl;
					std::cout << "Trying to allocate at position " << p << "..." << std::endl;
				}

				x.append(1, '\0');
				x.copy(buffer[p], x.size(), os);
				bitmap[p] = 1;
				sem_post(&data);

				if (log)
					std::cout << "Sucessfully allocated at position " << p << "!" << std::endl;
				return 1;
			}
			else {
				// buffer position is protected and dont has a copy being used

				sem_wait(&data);
				if (log) {
					std::cout << std::endl << "========WRITE OPERATION========" << std::endl;
					std::cout << "Memory Protection Fault, creating a copy of the content of position " << p << "..." << std::endl;
					page_faults++;
				}
				x.append(1, '\0');

				if ((copies[p] == nullptr) && (modified[p] <= 0)){
					copies[p] = new char[INPUT_SIZE];
					strcpy(copies[p], buffer[p]);
					num_alloc_copy++;
				}
				x.copy(buffer[p], x.size(), os);
				bitmap[p] = 1;

				if (log)
					std::cout << "Sucessfully created a copy of position " << p << "!" << std::endl;
				sem_post(&data);

				protection[p] = 0;
				sem_post(&acess_data);
				
				return 1;
			}
		}
		int read(int p, int last_p) {
			// Threating the read operation as an example of checkpoint copy to stable storage, because it enables the modify protection to the memory page
			int i, j;

			if ((p < 0) || (last_p > (size - 1))) {
				std::cerr << "Insert a valid read interval." << std::endl;
				return 0;
			}
			if (last_p < p) {
				std::cerr << "Final position must be bigger than initial." << std::endl;
				return 0;
			}

			if (log)
				std::cout << std::endl << "===> READ OPERATION " << p << " to " << last_p << " INIT <===" << std::endl;
			
			sem_wait(&acess_data);
			for (i = p; i <= last_p; i++) {
				protection[i] = 1;
				if (copies[i] != nullptr)
					modified[i]++;
			}
			sem_post(&acess_data);
			
			for (i = p; i <= last_p; i++) {
				sem_wait(&acess_data);
				sem_wait(&data);
				
				if (log)
					std::cout << "---> READ OPERATION CONTINUE <---" << std::endl;
				
				if (copies[i] == nullptr) {

					if (log) {
						std::cout << "BUFFER [" << i << "]: " << std::endl;
						for (j = 0; buffer[i][j] != '\0'; j++) {
							std::cout << buffer[i][j] << " ";
						}
						std::cout << std::endl;
					}

					protection[i] = 0;		
					sem_post(&acess_data);
					sem_post(&data);
				}
				else {

					if (log) {
						std::cout << "COPY [" << i << "]: " << std::endl;
						for (j = 0; copies[i][j] != '\0'; j++) {
							std::cout << copies[i][j] << " ";
						}
						std::cout << std::endl;
					}

					modified[i]--;
					if (modified[i] <= 0) {
						delete[] copies[i];
						copies[i] = nullptr;
						protection[i] = 0;
						num_dealloc_copy++;
					}
					sem_post(&acess_data);
					sem_post(&data);
				}
			}
			return 1;
		}

		int* getMap(int len, int offset) {
			int* aux = new int[len];

			for (int i = offset; i < (len + offset); i++) {
				aux[i] = bitmap[i];
			}
			return aux;
		}
		void showBuffer() {
			int i, j;

			sem_wait(&data);
			for (i = 0; i < size; i++) {

				if (i < 10)
					std::cout << "000" << i << "|  ";
				else if (i < 100)
					std::cout << "00" << i << "|  ";
				else if (i < 1000)
					std::cout << "0" << i << "|  ";

				if (bitmap[i] == 0)
					std::cout << "[    ****    ]    [ " << protection[i] << " ]    ";
				else {
					std::cout << "[ ";
					for (j = 0; j < INPUT_SIZE; j++) {
						std::cout << buffer[i][j];
					}
					std::cout << " ]    [ " << protection[i] << " ]    ";
				}
				if (copies[i] == nullptr)
					std::cout << "[    ****    ]" << std::endl;
				else {
					std::cout << "[ ";
					for (j = 0; j < INPUT_SIZE; j++) {
						std::cout << copies[i][j];
					}
					std::cout << " ]" << std::endl;
				}
				
			} // for
			sem_post(&data);
			return;
		}
		void showInfo() {

			for (int i = 0; i < size; i++) {
				if (bitmap[i] == 1)
					mem_usage++;
			}

			std::cout << "=====BUFFER STATUS=====" << std::endl;
			std::cout << "page_faults: " << page_faults << std::endl;
			std::cout << "mem_usage: " << ((float) mem_usage / size) * 100 << "%" << std::endl;
			std::cout << "num_alloc_copy: " << num_alloc_copy << std::endl;
			std::cout << "num_dealloc_copy: " << num_dealloc_copy << std::endl;
		}

	}; // class
} // namespace

#endif