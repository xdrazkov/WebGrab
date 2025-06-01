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
#include <curl/curl.h>
#include <fstream>
#include <filesystem>

std::queue<std::string> task_queue;  
std::atomic<bool> shutdown_flag(false);  
std::mutex queue_mutex;  
std::condition_variable cv;  

const int WORKER_THREADS = 3;  
const std::string DOWNLOADS_DIRECTORY = "downloads";

void sync_print(const std::string& message) {
    std::osyncstream out(std::cout);
    out << message << std::flush;
}

void sync_println(const std::string& message) {
    std::osyncstream out(std::cout);
    out << message << std::endl;
}

void sync_error(const std::string& message) {
    std::osyncstream err(std::cerr);
    err << message << std::endl;
}

size_t write_data(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

void process_url(const std::string& url) {  
    sync_println("Processing URL: " + url);  
    //std::this_thread::sleep_for(std::chrono::seconds(20));  
    CURL* curl;
    CURLcode res;
    FILE* fp;

    std::string filename = url.substr(url.find_last_of("/\\") + 1);
    if (filename.empty()) {
        filename = "downloaded_file";
    }

    // Create downloads directory if it doesn't exist
    std::filesystem::path downloads_path(DOWNLOADS_DIRECTORY);
    if (!std::filesystem::exists(downloads_path)) {
        std::filesystem::create_directory(downloads_path);
    }

    std::string filepath = (downloads_path / filename).string();
	// If file already exists, append a number to the filename
    std::filesystem::path filepath_obj(filepath);
    std::string stem = filepath_obj.stem().string();
    std::string ext = filepath_obj.extension().string();
	int count = 1;
    while (std::filesystem::exists(filepath)) {
		filepath = (downloads_path / (stem + "_" + std::to_string(count) + ext)).string();
		count++;
    }

    curl = curl_easy_init();
    if (curl) {
		fopen_s(&fp, filepath.c_str(), "wb");
        if (fp) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                sync_error("CURL error: " + std::string(curl_easy_strerror(res)));
            }
            curl_easy_cleanup(curl);
            fclose(fp);
        }
        else {
            sync_error("Failed to open file for writing: " + filename);
        }
    }
    else {
        sync_error("Failed to initialize CURL for URL: " + url);
    }

    sync_println("Finished processing URL: " + url);  
}  

void worker_function(int id) {
    //sync_println("Worker " + std::to_string(id) + " started.");
    while (true) {  
        std::string url;  

        {  
            std::unique_lock<std::mutex> lock(queue_mutex);  
            cv.wait(lock, [] { return !task_queue.empty() || shutdown_flag; });  

            if (shutdown_flag) {  
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

    curl_global_init(CURL_GLOBAL_ALL);

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
            shutdown_flag = true;  
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
			sync_error("Unknown command: " + command + ". Available commands: download <url>, queue, quit.");
        }  

    }  

    std::cout << "Waiting for tasks to finish..." << std::endl;  
    for (auto& worker : workers) {  
        if (worker.joinable()) {  
            worker.join();  
        }  
    }  

    curl_global_cleanup();

    std::cout << "WebGrab terminated." << std::endl;  
    return 0;  
}
