#pragma once

#include "Base.hpp"

namespace Outputs
{

class Dummy : public Base
{
public:
    const char* getName() const override { return "Dummy"; }

    float getVolume() const override { return 0.f; }
    void setVolume(float /* aVolume */) override { }
};

}
