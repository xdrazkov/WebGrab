#include <iostream>
#include <string>
#include <sstream>
#include <queue>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <chrono>

std::queue<std::string> task_queue;
std::atomic<bool> shutdown(false);
std::mutex queue_mutex;
std::condition_variable cv;

const int WORKER_THREADS = 3;

void process_url(const std::string& url) {
    std::cout << "Processing URL: " << url << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(20));
	std::cout << "Finished processing URL: " << url << std::endl;
}

void worker_function(int id) {
	std::cout << "Worker " << id << " started.\n";
    while (true) {
        std::string url;

        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            cv.wait(lock, [] { return !task_queue.empty() || shutdown; });

            if (shutdown) {
                break;
            }

            url = task_queue.front();
			task_queue.pop();
        }

        if (!url.empty()) {
			process_url(url);
        }
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
			cv.notify_all();
            break;
        }
        else if (command == "download" || command == "dl") {
            std::string url;
            if (iss >> url) {
                {
                    std::lock_guard<std::mutex> lock(queue_mutex);
                    task_queue.push(url);
                    std::cout << "Added download task for URL: " << url << std::endl;
                }
                cv.notify_one();
            }
            else {
				std::cout << "Usage: download <url>" << std::endl;
            }
        }
        else if (command == "queue") {
            std::lock_guard<std::mutex> lock(queue_mutex);
            if (task_queue.empty()) {
                std::cout << "No tasks in the queue." << std::endl;
            } else {
				std::cout << "Length of queue: " << task_queue.size() << std::endl;
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
