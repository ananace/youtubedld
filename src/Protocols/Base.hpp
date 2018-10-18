#pragma once

#include "Base/Event.hpp"
#include <cstdint>

enum : uint32_t
{
    Client_None = 0,
    Client_All  = UINT32_MAX,
};

namespace Protocols
{

struct Event;

class Base
{
public:
    virtual ~Base() = default;

    virtual bool supportsPost() const { return false; }

    virtual bool init() { return true; }
    virtual void update() = 0;

    virtual bool poll(Event& /* aEv */) { return false; }
    virtual void post(uint32_t /* aClient */ = Client_All) { }
};

}
