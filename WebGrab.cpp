#include <iostream>
#include <string>
#include <sstream>

int main()
{
    std::cout << "WebGrab started.\n";

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
            break;
        }

    }
	std::cout << "WebGrab terminated." << std::endl;
    return 0;
}
