

#include "../../includes/Data.hpp"

Data::Data(){}

Data::Data(int _fd, std::string _data, bool _ready)
: fd(_fd), data(_data), ready(_ready)
{}

Data::~Data(){}

