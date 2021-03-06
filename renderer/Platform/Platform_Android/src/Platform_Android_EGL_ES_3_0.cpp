//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_Android/Platform_Android_EGL_ES_3_0.h"

namespace ramses_internal
{
    IPlatform* Platform_Base::CreatePlatform(const RendererConfig& rendererConfig)
    {
        return new Platform_Android_EGL_ES_3_0(rendererConfig);
    }

    Platform_Android_EGL_ES_3_0::Platform_Android_EGL_ES_3_0(const RendererConfig& rendererConfig)
        : Platform_Android_EGL(rendererConfig)
    {
    }
}
