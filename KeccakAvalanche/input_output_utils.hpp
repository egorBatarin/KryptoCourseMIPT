#include <vector>
#include <string>

void printhex(const unsigned char input[], const size_t size);

std::vector<std::string> read_input(const char *filename);

std::vector<std::string> read_input_hex(const char *filename);

void write_output_bin(const char *filename, std::vector<std::string> lines);

void write_output_hex(const char *filename, std::vector<std::string> lines);