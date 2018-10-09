#pragma once

namespace Protocols
{

struct Event;

class Base
{
public:
    virtual ~Base() = default;

    virtual bool supportsPost() const { return false; }
    virtual void post() { }
    virtual void update() = 0;
};

}
