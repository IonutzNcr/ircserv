#include <vector>
#include <string>

std::vector<std::string> split(const std::string& str, char delimiter);
std::string intToString(int value);

// IRC case-insensitive comparison (Scandinavian charset)
int ircToLower(int c);
bool ircCaseEqual(const std::string& a, const std::string& b);