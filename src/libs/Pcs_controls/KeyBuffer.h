#pragma once

#include "../Animation/ActionInfo.h"
#include "controls.h"
#include "vmodule_api.h"

#include <vector>

struct ControlKeyBuffer
{
  public:
    ControlKeyBuffer();
    ~ControlKeyBuffer();

    void Reset();
    void AddKey(char *u8_str, int u8_size, bool bSystem);
    void AddKey(const KeyDescr &key);

    long GetBufferLength()
    {
        return pcBuffer_.size();
    }

    // FIXME: Not good
    const KeyDescr *c_str()
    {
        return pcBuffer_.data();
    }

    std::vector<KeyDescr> pcBuffer_;
};