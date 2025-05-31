#include <iostream>
#include <string>
#include <sstream>
#include <queue>
#include <thread>
#include <atomic>

std::queue<std::string> task_queue;
std::atomic<bool> shutdown(false);

const int WORKER_THREADS = 3;

void worker_function(int id) {
	std::cout << "Worker " << id << " started.\n";
    while (not shutdown) {

    }
}

int main()
{
    std::cout << "WebGrab started.\n";

    std::vector<std::thread> workers;
    for (int i = 0; i < WORKER_THREADS; ++i) {
        workers.emplace_back(worker_function, i);
	}

    std::string line;
    while (true) {
        std::cout << "> " << std::flush;
        if (!std::getline(std::cin, line)) {
            break;
		}
        
		std::istringstream iss(line);
        std::string command;
		iss >> command;

        if (command == "quit") {
            shutdown = true;
            break;
        }
        else if (command == "download") {
            std::string url;
            if (iss >> url) {
                task_queue.push(url);
                std::cout << "Added download task for URL: " << url << std::endl;
            }
            else {
                std::cout << "Usage: download <url>" << std::endl;
            }
        }
        else {
            std::cout << "Unknown command: " << command << std::endl;
            std::cout << "Available commands: download <url>, quit" << std::endl;
        }

    }

    std::cout << "Waiting for tasks to finish..." << std::endl;
    for (auto& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }

	std::cout << "WebGrab terminated." << std::endl;
    return 0;
}
