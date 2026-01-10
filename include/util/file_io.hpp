#pragma once

#include <fstream>
#include <sstream>

bool read_file(const std::string &path, std::string &out); // Read entire file into string. false if error