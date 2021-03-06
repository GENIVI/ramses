//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_GLSLEFFECT_H
#define RAMSES_GLSLEFFECT_H

#include "Collections/StringOutputStream.h"
#include "Collections/Vector.h"
#include "Collections/HashMap.h"
#include "SceneAPI/EFixedSemantics.h"
#include "SceneAPI/RenderState.h"
#include "Resource/IResource.h"
#include "absl/types/optional.h"

struct TBuiltInResource;
namespace glslang
{
    class TProgram;
    class TShader;
    class TPoolAllocator;
}

namespace ramses_internal
{
    class EffectResource;

    class GlslEffect
    {
    public:
        GlslEffect(const String& vertexShader,
            const String& fragmentShader,
            const String& geometryShader,
            const std::vector<String>& compilerDefines,
            const HashMap<String, EFixedSemantics>& semanticInputs,
            const String& name);

        ~GlslEffect();

        EffectResource* createEffectResource(ResourceCacheFlag cacheFlag);

        UInt32 getShadingLanguageVersion() const;
        String getEffectErrorMessages() const;

    private:
        struct ShaderParts
        {
            String version;
            String defines;
            String userCode;
        };

        const String m_vertexShader;
        const String m_fragmentShader;
        const String m_geometryShader;
        const std::vector<String> m_compilerDefines;
        const HashMap<String, EFixedSemantics> m_semanticInputs;
        const String m_name;

        mutable StringOutputStream m_errorMessages;
        EffectResource* m_effectResource;
        UInt32 m_shadingLanguageVersion;

        String createDefineString() const;
        bool createShaderParts(ShaderParts& outParts, const String& defineString, const String& userShader) const;
        String mergeShaderParts(const ShaderParts& shaderParts) const;
        bool parseShader(glslang::TShader& tShader, const TBuiltInResource& glslCompilationResources, const ShaderParts& shaderParts, const String& shaderName);
        glslang::TProgram* linkProgram(glslang::TShader* vertexShader, glslang::TShader* fragmentShader, glslang::TShader* geometryShader) const;
        bool extractAndCheckShaderVersions(const glslang::TProgram* program);
        bool extractAndCheckExtensions(const glslang::TProgram* program);
        bool isSupportedExtension(const String& extension) const;
    };

}

#endif
