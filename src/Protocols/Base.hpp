#pragma once

#include <cstdint>

enum : uint32_t
{
    Client_All = UINT32_MAX,
};

namespace Protocols
{

struct Event;

class Base
{
public:
    virtual ~Base() = default;

    virtual bool supportsPost() const { return false; }
    virtual void post(uint32_t aClient = Client_All) { }
    virtual void update() = 0;
};

}
