#pragma once

#include <vector>
#include <string>

std::vector<std::string> split(const std::string& str, char delimiter);
std::string intToString(int value);

// for Scandinavian charset
int ircToLower(int c);
bool ircCaseEqual(const std::string& a, const std::string& b);

bool sendAll(int fd, const std::string& msg);