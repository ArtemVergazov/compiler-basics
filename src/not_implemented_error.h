#pragma once

#include <stdexcept>

class NotImplementedError : public std::logic_error
{
public:
    NotImplementedError() : std::logic_error{ "Not implemented" } {}
};
