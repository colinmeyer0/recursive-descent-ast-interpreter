#include <fstream>
#include <sstream>

#include "util/file_io.hpp"

bool read_file(const std::string &path, std::string &out) {
    // Open file path
    std::ifstream in(path);
    if (!in) return false; // failed to open

    // copy into string
    std::ostringstream buf;
    buf << in.rdbuf(); // Stream file into buffer
    out = buf.str();   // copy to output
    return true;
}
