#include "sha3algo.hpp"
#include <fstream>
#include <vector>
#include <string>


void printhex(const unsigned char input[], const size_t size) {
    for (size_t i = 0; i < size; ++i) {
        printf("%02x", input[i]);
    }
}

std::vector<std::string> read_input(const char *filename) {
    std::vector<std::string> lines;
    std::ifstream fin(filename);
    std::string line;
    while (std::getline(fin, line))
        lines.push_back(line);
    fin.close();
    return lines;
}

std::vector<std::string> read_input_hex(const char *filename) {
    std::vector<std::string> lines;
    std::ifstream fin(filename);
    std::string line;
    std::string line2;
    while (std::getline(fin, line)) {
        char *c_str = (char *)malloc(line.size());
        for (size_t i = 0; i < line.size(); ++i) {
            c_str[i] = line[i];
        }
        
        std::string line2;
        line2.resize(line.size() / 2);
        for (size_t i = 0; i < line.size() / 2; ++i) {
            sscanf(c_str + 2 * i, "%02hhx", &line2[i]);
        }

        lines.push_back(line2);
        free(c_str);
    }
    fin.close();
    return lines;
}

void write_output_bin(const char *filename, std::vector<std::string> lines) {
    FILE* fout = fopen(filename, "w");
    for (std::string line: lines) {
        for (size_t i = 0; i < line.size(); ++i) {
            fprintf(fout, "%c", (const unsigned char)line[i]);
        }
        // fprintf(fout, "\n");
    }
    fclose(fout);
}

void write_output_hex(const char *filename, std::vector<std::string> lines) {
    FILE* fout = fopen(filename, "w");
    for (std::string line: lines) {
        for (size_t i = 0; i < line.size(); ++i) {
            fprintf(fout, "%02x", (const unsigned char)line[i]);
        }
        fprintf(fout, "\n");
    }
    fclose(fout);
}