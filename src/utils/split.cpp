#include <string>
#include <vector>
#include <sstream>


std::vector<std::string> split(const std::string& str, char delimiter)
{
    std::vector<std::string> tokens;
    std::size_t start = 0;
   
    //Skip leading delimiters
    while (start < str.size() && str[start] == delimiter)
        start++;
    std::size_t end = str.find(delimiter);
    while (end != std::string::npos)
    {
        if (end > start)
            tokens.push_back(str.substr(start, end - start));
        start = end + 1;
        // Skip consecutive delimiters
        while (start < str.size() && str[start] == delimiter)
            start++;
        end = str.find(delimiter, start);
    }
    // Don't forget the last token
    if (start < str.size())
        tokens.push_back(str.substr(start));
    
    return tokens;
}

std::string intToString(int value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}