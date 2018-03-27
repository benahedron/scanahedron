#pragma once

#include <memory>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <cassert>

#define SHARED_PTR(name)                           \
    typedef std::shared_ptr<class name> name##Ptr; \
    typedef std::shared_ptr<const class name> name##ConstPtr

#define SHARED_STRUCT_PTR(name)                     \
    typedef std::shared_ptr<struct name> name##Ptr; \
    typedef std::shared_ptr<const struct name> name##ConstPtr
