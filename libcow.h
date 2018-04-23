#ifndef LIB_COW_H
#define LIB_COW_H
//#define _SCL_SECURE_NO_WARNINGS

#include <iostream>
#include <string.h>
#include <thread>
#include <semaphore.h>
#define INPUT_SIZE 10
#define DEFAULT_S 4096

namespace libcow {
	
	class memory {
	private:
		int *bitmap, *modified, *protection;
		char **buffer, **copies;
		int size;
		sem_t mutex;

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
				buffer[i] = new char[INPUT_SIZE];

			copies = new char*[DEFAULT_S];
			bitmap = new int[DEFAULT_S]();
			modified = new int[DEFAULT_S]();
			protection = new int[DEFAULT_S]();
			size = DEFAULT_S;
			sem_init(&mutex, 0, 1);
		}
		memory(int n) {
			buffer = new char*[n];
			for (int i = 0; i < n; i++)
				buffer[i] = new char[INPUT_SIZE];

			copies = new char*[n];
			bitmap = new int[n]();
			modified = new int[n]();
			protection = new int[n]();
			size = n;
			sem_init(&mutex, 0, 1);
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
			if ((protection[p] == 1) && (modified[p] == 1)) {
				std::cerr << "Insufficient memory avaiable to allocate a new copy, clear some data to continue." << std::endl;
				return 0;
			}
			
			if (protection[p] == 0) {
				// buffer position is not protected

				std::cout << std::endl << "========WRITE OPERATION========" << std::endl;
				std::cout << "Trying to allocate at position " << p << "..." << std::endl;
				x.copy(buffer[p], x.size(), os);
				bitmap[p] = 1;
				std::cout << "Sucessfully allocated at position " << p << "!" << std::endl;
				return 1;
			}
			else {
				// buffer position is protected and dont has a copy being used

				std::cout << std::endl << "========WRITE OPERATION========" << std::endl;
				std::cout << "Memory Protection Fault, creating a copy of the content of position " << p << "..." << std::endl;
				if (modified[p] == 0)
					copies[p] = new char[INPUT_SIZE];
				if (bitmap[p] == 1)
					// 'copies[p] = buffer[p]' is not the right operation
					strcpy(copies[p], buffer[p]);

				x.copy(buffer[p], x.size(), os);
				modified[p] = 1;
				std::cout << "Sucessfully created a copy of position " << p << "!" << std::endl;
				//disable protection flag
				protection[p] = 0;
				return 1;
			}
		}
		int read(int p, int len, int offset) {
			int i;

			std::cout << std::endl << "========READ OPERATION========" << std::endl;
			if ((offset + len) > INPUT_SIZE) {
				std::cerr << "To read more than one row at a time, use the 'showBuffer()' method." << std::endl;
				return 0;
			}
			// Threating the read operation as an example of checkpoint copy to stable storage, because it enables the modify protection to the memory page
			protection[p] = 1;
			std::cout << "BUFFER[" << p << "]: " << std::endl;
			for (i = offset; i < len + offset; i++) {
				std::cout << buffer[p][i] << " ";
			}
			std::cout << std::endl;
			return 1;
		}

		/*void operation(char x, std::string input, int p, int len) {
			
			if (x == 'w') {
			
				sem_wait(&mutex);
				std::thread write_b(input, p);
				sem_post(&mutex);
			
			}
			else if (x == 'r') {
				
				sem_wait(&mutex);
				std::thread read_b(p, len, 0);
				sem_post(&mutex);
			}
		}*/

		int* getMap(int len, int offset) {
			int* aux = new int[len];

			for (int i = offset; i < len + offset; i++) {
				aux[i] = bitmap[i];
			}
			return aux;
		}
		void showBuffer() {
			int i, j;

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
				if (modified[i] == 0)
					std::cout << "[    ****    ]" << std::endl;
				else {
					std::cout << "[ ";
					for (j = 0; j < INPUT_SIZE; j++) {
						std::cout << copies[i][j];
					}
					std::cout << " ]" << std::endl;
				}
				
			} // for
			return;
		} // void
	}; //class
} // namespace

#endif