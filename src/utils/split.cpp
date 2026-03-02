#include <string>
#include <vector>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <poll.h>


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

// IRC case-insensitive comparison (RFC 2812 - Scandinavian charset)
// { = [, } = ], | = \, ~ = ^
int ircToLower(int c)
{
    if (c >= 'A' && c <= 'Z')
        return c + 32;
    if (c == '[') return '{';
    if (c == ']') return '}';
    if (c == '\\') return '|';
    if (c == '^') return '~';
    return c;
}

bool ircCaseEqual(const std::string& a, const std::string& b)
{
    if (a.size() != b.size())
        return false;
    for (size_t i = 0; i < a.size(); i++)
    {
        if (ircToLower(a[i]) != ircToLower(b[i]))
            return false;
    }
    return true;
}

bool sendAll(int fd, const std::string& msg)
{
    const char* data = msg.c_str();
    size_t total = msg.length();
    size_t sent = 0;
    
    while (sent < total)
    {
        ssize_t n = send(fd, data + sent, total - sent, MSG_NOSIGNAL);
        if (n < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // Wait for socket to be ready (max 5 sec)
                struct pollfd pfd;
                pfd.fd = fd;
                pfd.events = POLLOUT;
                if (poll(&pfd, 1, 5000) <= 0)
                    return false;  // Timeout or error
                continue;
            }
            return false;  // Error (EPIPE, etc.)
        }
        if (n == 0)
            return false;  // Connection closed
        sent += n;
    }
    return true;
}