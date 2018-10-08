#pragma once

namespace Protocols
{

struct Event;

class Base
{
public:
    virtual ~Base() = default;

    virtual void Update() = 0;
};

}
