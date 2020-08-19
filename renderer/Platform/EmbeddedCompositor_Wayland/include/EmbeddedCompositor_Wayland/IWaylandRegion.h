//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IWAYLANDREGION_H
#define RAMSES_IWAYLANDREGION_H

#include <cstdint>

namespace ramses_internal
{
    class IWaylandClient;

    class IWaylandRegion
    {
    public:
        virtual ~IWaylandRegion(){}

        virtual void resourceDestroyed() = 0;
        virtual void regionAdd(IWaylandClient& client, int32_t x, int32_t y, int32_t width, int32_t height) = 0;
        virtual void regionSubtract(IWaylandClient& client, int32_t x, int32_t y, int32_t width, int32_t height) = 0;
    };
}

#endif
