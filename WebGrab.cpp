#include <iostream>  
#include <string>  
#include <sstream>  
#include <queue>  
#include <thread>  
#include <atomic>  
#include <mutex>  
#include <condition_variable>  
#include <chrono>  
#include <vector>  
#include <syncstream>

std::queue<std::string> task_queue;  
std::atomic<bool> shutdown(false);  
std::mutex queue_mutex;  
std::condition_variable cv;  

const int WORKER_THREADS = 3;  

void sync_print(const std::string& message) {
    std::osyncstream out(std::cout);
    out << message << std::flush;
}

void sync_println(const std::string& message) {
    std::osyncstream out(std::cout);
    out << message << std::endl;
}

void process_url(const std::string& url) {  
    //sync_println("Processing URL: " + url);  
    std::this_thread::sleep_for(std::chrono::seconds(20));  
    sync_println("Finished processing URL: " + url);  
}  

void worker_function(int id) {
    //sync_println("Worker " + std::to_string(id) + " started.");
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
    sync_println("WebGrab started.");

    std::vector<std::thread> workers;  
    for (int i = 0; i < WORKER_THREADS; ++i) {  
        workers.emplace_back(worker_function, i);  
    }  

    std::string line;  
    while (true) {  
        sync_print("> ");
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
                    sync_println("Added download task for URL: " + url);
                }  
                cv.notify_one();  
            }  
            else {  
                sync_println("Usage: download <url>");
            }  
        }  
        else if (command == "queue") {  
            std::lock_guard<std::mutex> lock(queue_mutex);  
            if (task_queue.empty()) {  
                sync_println("No tasks in the queue.");
            } else {  
                sync_println("Length of queue: " + std::to_string(task_queue.size()));
            }  
        }  
        else {  
            sync_println("Unknown command: " + command);
            sync_println("Available commands: download <url>, quit");
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
