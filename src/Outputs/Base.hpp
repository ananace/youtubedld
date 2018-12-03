#pragma once

namespace Outputs
{

class Base
{
public:
    virtual const char* getName() const = 0;

    virtual float getVolume() const = 0;
    virtual void setVolume(float aVolume) = 0;

private:
};

}
