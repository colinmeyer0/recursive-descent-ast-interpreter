#pragma once

#include <sstream>
#include <fstream>

bool read_file(const std::string &path, std::string &out); // Read entire file into string. false if error