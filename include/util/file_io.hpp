#pragma once

#include <string>

/// read entire file into string, false if error
bool read_file(const std::string &path, std::string &out);
