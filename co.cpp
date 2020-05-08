#include "co.hpp"

namespace co{
    Generator* Generator::current_ = nullptr;
    Generator Generator::root_;
}