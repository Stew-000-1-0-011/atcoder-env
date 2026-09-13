#include <iostream>
#include <string>
#include <print>

int main() {
	std::string line;
	while (std::getline(std::cin, line)) {
		std::println("{}", line);
	}
	return 0;
}