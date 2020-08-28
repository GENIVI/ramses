//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DYNAMICQUAD_BASE_H
#define RAMSES_DYNAMICQUAD_BASE_H

#include "ramses-client-api/RamsesClient.h"
#include "ScreenspaceQuad.h"

namespace ramses
{
    class RenderPass;
    class RenderGroup;
    class Scene;
    class Appearance;
    class GeometryBinding;
    class Effect;
    class MeshNode;
    class TextureSampler;
}

namespace ramses_internal
{
    // Per-client resources
    struct QuadResources
    {
        ramses::ArrayResource* indices = nullptr;
        ramses::ArrayResource* texCoords = nullptr;
        ramses::ArrayResource* vertexPos = nullptr;
    };

    class DynamicQuad_Base
    {
    public:
        ramses::RenderGroup& getRenderGroup();
        const ramses::RenderGroup& getRenderGroup() const;
        const ramses::MeshNode& getMeshNode() const;

    protected:
        DynamicQuad_Base(ramses::Scene& scene, const ScreenspaceQuad& screenspaceQuad);
        virtual ~DynamicQuad_Base()
        {
        }

        virtual void recreate() = 0;

        static ramses::Effect&  CreateTestEffect(ramses::Scene& scene);

        QuadResources           createRandomizedQuadResources();
        void                    destroyQuadResources(const QuadResources& quad);
        void                    setQuadResources(const QuadResources& quad, ramses::TextureSampler& textureSampler);

        ramses::Scene&                  m_scene;
        const ScreenspaceQuad           m_screenspaceQuad;

        ramses::RenderGroup&            m_renderGroup;
        ramses::MeshNode&               m_meshNode;

        ramses::Effect&                 m_effect;
        ramses::Appearance&             m_appearance;
        ramses::GeometryBinding&        m_geometryBinding;

        QuadResources m_quadResources;

    };
}

#endif
