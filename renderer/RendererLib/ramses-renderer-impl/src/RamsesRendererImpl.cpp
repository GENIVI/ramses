//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RamsesRendererImpl.h"
#include "ramses-renderer-api/RendererConfig.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-renderer-api/IRendererEventHandler.h"
#include "ramses-renderer-api/WarpingMeshData.h"
#include "ramses-framework-api/RamsesFramework.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-framework-api/DcsmConsumer.h"
#include "RamsesFrameworkImpl.h"
#include "RendererConfigImpl.h"
#include "DisplayConfigImpl.h"
#include "WarpingMeshDataImpl.h"
#include "RendererSceneControlImpl.h"
#include "DcsmContentControlImpl.h"
#include "DcsmConsumerImpl.h"
#include "RamsesFrameworkTypesImpl.h"
#include "SceneAPI/SceneId.h"
#include "SceneAPI/Handles.h"
#include "Utils/LogMacros.h"
#include "Platform_Base/Platform_Base.h"
#include "RendererAPI/ISystemCompositorController.h"
#include "RendererLib/RendererCommands.h"
#include "BinaryShaderCacheProxy.h"
#include "RendererResourceCacheProxy.h"
#include "RamsesRendererUtils.h"
#include "RendererFactory.h"
#include "FrameworkFactoryRegistry.h"
#include "Ramsh/Ramsh.h"
#include <chrono>

namespace ramses
{
    static const bool rendererRegisterSuccess = RendererFactory::RegisterRendererFactory();

    RamsesRendererImpl::RamsesRendererImpl(RamsesFrameworkImpl& framework, const RendererConfig& config)
        : m_framework(framework)
        , m_binaryShaderCache(config.impl.getBinaryShaderCache() ? new BinaryShaderCacheProxy(*(config.impl.getBinaryShaderCache())) : nullptr)
        , m_rendererResourceCache(config.impl.getRendererResourceCache() ? new RendererResourceCacheProxy(*(config.impl.getRendererResourceCache())) : nullptr)
        , m_pendingRendererCommands()
        , m_rendererFrameworkLogic(framework.getScenegraphComponent(), m_rendererCommandBuffer, framework.getFrameworkLock())
        , m_threadWatchdog(framework.getThreadWatchdogConfig(), ERamsesThreadIdentifier_Renderer)
        , m_displayDispatcher{ std::make_unique<ramses_internal::DisplayDispatcher>(config.impl.getInternalRendererConfig(), m_rendererFrameworkLogic, m_threadWatchdog) }
        , m_systemCompositorEnabled(config.impl.getInternalRendererConfig().getSystemCompositorControlEnabled())
        , m_loopMode(ramses_internal::ELoopMode::UpdateAndRender)
        , m_rendererLoopThreadType(ERendererLoopThreadType_Undefined)
        , m_periodicLogSupplier(framework.getPeriodicLogger(), m_rendererCommandBuffer)
    {
        assert(!framework.isConnected());

        { //Add ramsh commands to ramsh, independent of whether it is enabled or not.
            m_ramshCommands.push_back(std::make_shared<ramses_internal::Screenshot>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<ramses_internal::LogRendererInfo>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<ramses_internal::ShowFrameProfiler>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<ramses_internal::PrintStatistics>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<ramses_internal::TriggerPickEvent>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<ramses_internal::SetClearColor>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<ramses_internal::SetSkippingOfUnmodifiedBuffers>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<ramses_internal::SystemCompositorControllerListIviSurfaces>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<ramses_internal::SystemCompositorControllerSetLayerVisibility>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<ramses_internal::SystemCompositorControllerSetSurfaceVisibility>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<ramses_internal::SystemCompositorControllerSetSurfaceOpacity>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<ramses_internal::SystemCompositorControllerSetSurfaceDestRectangle>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<ramses_internal::SystemCompositorControllerScreenshot>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<ramses_internal::SystemCompositorControllerAddSurfaceToLayer>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<ramses_internal::SystemCompositorControllerRemoveSurfaceFromLayer>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<ramses_internal::SystemCompositorControllerDestroySurface>(m_rendererCommandBuffer));
            m_ramshCommands.push_back(std::make_shared<ramses_internal::SetFrameTimeLimits>(m_rendererCommandBuffer));
            for (const auto& cmd : m_ramshCommands)
                m_framework.getRamsh().add(cmd);
            LOG_INFO(ramses_internal::CONTEXT_SMOKETEST, "Ramsh commands registered from RamsesRenderer");
        }

        LOG_TRACE(ramses_internal::CONTEXT_PROFILING, "RamsesRenderer::RamsesRenderer finished initializing renderer");
    }

    status_t RamsesRendererImpl::doOneLoop()
    {
        if (ERendererLoopThreadType_InRendererOwnThread == m_rendererLoopThreadType)
        {
            return addErrorEntry("Can not call doOneLoop explicitly if renderer is (or was) running in its own thread!");
        }

        m_rendererLoopThreadType = ERendererLoopThreadType_UsingDoOneLoop;
        m_displayDispatcher->dispatchCommands(m_rendererCommandBuffer);
        m_displayDispatcher->doOneLoop();
        return StatusOK;
    }

    status_t RamsesRendererImpl::flush()
    {
        pushAndConsumeRendererCommands(m_pendingRendererCommands);
        return StatusOK;
    }

    displayId_t RamsesRendererImpl::createDisplay(const DisplayConfig& config)
    {
        if (config.validate() != StatusOK)
        {
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "RamsesRenderer::createDisplay: failed to create display, using invalid display configuration - use validate method on object!");
            return displayId_t::Invalid();
        }

        const displayId_t displayId = m_nextDisplayId;
        m_nextDisplayId.getReference() = m_nextDisplayId.getValue() + 1;
        // display's framebuffer is also counted as display buffer together with offscreen buffers
        assert(m_displayFramebuffers.count(displayId) == 0);
        m_displayFramebuffers.insert({ displayId, m_nextDisplayBufferId });
        m_nextDisplayBufferId.getReference() = m_nextDisplayBufferId.getValue() + 1;

        ramses_internal::RendererCommand::CreateDisplay cmd{ ramses_internal::DisplayHandle(displayId.getValue()), config.impl.getInternalDisplayConfig(), m_binaryShaderCache.get() };
        m_pendingRendererCommands.push_back(std::move(cmd));

        return displayId;
    }

    status_t RamsesRendererImpl::destroyDisplay(displayId_t displayId)
    {
        ramses_internal::RendererCommand::DestroyDisplay cmd{ ramses_internal::DisplayHandle(displayId.getValue()) };
        m_pendingRendererCommands.push_back(std::move(cmd));
        m_displayFramebuffers.erase(displayId);

        return StatusOK;
    }

    displayBufferId_t RamsesRendererImpl::getDisplayFramebuffer(displayId_t displayId) const
    {
        const auto it = m_displayFramebuffers.find(displayId);
        if (it == m_displayFramebuffers.cend())
        {
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "RamsesRenderer::getDisplayFramebuffer: there is no display with ID " << displayId);
            return displayBufferId_t::Invalid();
        }
        return it->second;
    }

    const ramses_internal::DisplayDispatcher& RamsesRendererImpl::getDisplayDispatcher() const
    {
        return *m_displayDispatcher;
    }

    ramses_internal::DisplayDispatcher& RamsesRendererImpl::getDisplayDispatcher()
    {
        return *m_displayDispatcher;
    }

    RendererSceneControl* RamsesRendererImpl::getSceneControlAPI()
    {
        if (m_dcsmContentControl)
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "Cannot instantiate RendererSceneControl, another scene control API is already in use, only one can be active per session.");
            return nullptr;
        }

        if (!m_sceneControlAPI)
        {
            LOG_INFO(ramses_internal::CONTEXT_CLIENT, "RamsesRenderer: instantiating RendererSceneControl");
            m_sceneControlAPI = UniquePtrWithDeleter<RendererSceneControl>{ new RendererSceneControl(*new RendererSceneControlImpl(*this)), [](RendererSceneControl* api) { delete api; } };
        }

        return m_sceneControlAPI.get();
    }

    DcsmContentControl* RamsesRendererImpl::createDcsmContentControl()
    {
        if (m_dcsmContentControl || m_sceneControlAPI)
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "Cannot instantiate DcsmContentControl, another scene control API is already in use, only one can be active per session.");
            return nullptr;
        }

        // DcsmContentControl operates on scene control API, the check above makes sure that it can be instantiated.
        // DcsmContentControl will be then the only active 'scene control' API for user, even though in fact RendererSceneControl is the actual API used internally - via DcsmContentControl.
        auto sceneControlAPI = getSceneControlAPI();
        assert(sceneControlAPI != nullptr);

        LOG_INFO(ramses_internal::CONTEXT_CLIENT, "RamsesRenderer: instantiating DcsmContentControl");
        m_dcsmContentControl = UniquePtrWithDeleter<DcsmContentControl>{ new DcsmContentControl(*new DcsmContentControlImpl(m_framework.createDcsmConsumer()->impl, sceneControlAPI->impl)), [](DcsmContentControl* api) { delete api; } };
        return m_dcsmContentControl.get();
    }

    void RamsesRendererImpl::logConfirmationEcho(displayId_t display, const ramses_internal::String& text)
    {
        m_pendingRendererCommands.push_back(ramses_internal::RendererCommand::ConfirmationEcho{ ramses_internal::DisplayHandle{ display.getValue() }, text });
    }

    const ramses_internal::RendererCommands& RamsesRendererImpl::getPendingCommands() const
    {
        return m_pendingRendererCommands;
    }

    status_t RamsesRendererImpl::updateWarpingMeshData(displayId_t displayId, const WarpingMeshData& newWarpingMeshData)
    {
        if (newWarpingMeshData.impl.getWarpingMeshData().getIndices().size() % 3 != 0)
        {
            return addErrorEntry("RamsesRenderer::updateWarpingConfig failed: warping indices not divisible by 3 (not a triangle list)!");
        }

        assert(newWarpingMeshData.impl.getWarpingMeshData().getTextureCoordinates().size() == newWarpingMeshData.impl.getWarpingMeshData().getVertexPositions().size());

        if (newWarpingMeshData.impl.getWarpingMeshData().getVertexPositions().size() == 0 || newWarpingMeshData.impl.getWarpingMeshData().getIndices().size() == 0)
        {
            return addErrorEntry("RamsesRenderer::updateWarpingConfig failed: must provide more than zero indices and vertices!");
        }

        ramses_internal::RendererCommand::UpdateWarpingData cmd{ ramses_internal::DisplayHandle(displayId.getValue()), newWarpingMeshData.impl.getWarpingMeshData() };
        m_pendingRendererCommands.push_back(std::move(cmd));

        return StatusOK;
    }

    displayBufferId_t RamsesRendererImpl::createOffscreenBuffer(displayId_t display, uint32_t width, uint32_t height, uint32_t sampleCount, bool interruptible, EDepthBufferType depthBufferType)
    {
        if (width < 1u || width > 4096u ||
            height < 1u || height > 4096u)
        {
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "RamsesRenderer::createOffscreenBuffer: failed to create offscreen buffer, resolution must be higher than 0x0 and lower than 4096x4096!");
            return {};
        }

        const ramses_internal::DisplayHandle displayHandle(display.getValue());
        const displayBufferId_t bufferId = m_nextDisplayBufferId;
        const ramses_internal::OffscreenBufferHandle bufferHandle(bufferId.getValue());
        m_nextDisplayBufferId.getReference() = m_nextDisplayBufferId.getValue() + 1;

        ramses_internal::ERenderBufferType depthBufferTypeInternal = ramses_internal::ERenderBufferType_NUMBER_OF_ELEMENTS;
        switch (depthBufferType)
        {
        case EDepthBufferType_None:
            depthBufferTypeInternal = ramses_internal::ERenderBufferType_InvalidBuffer;
            break;
        case EDepthBufferType_Depth:
            depthBufferTypeInternal = ramses_internal::ERenderBufferType_DepthBuffer;
            break;
        case EDepthBufferType_DepthStencil:
            depthBufferTypeInternal = ramses_internal::ERenderBufferType_DepthStencilBuffer;
            break;
        }

        ramses_internal::RendererCommand::CreateOffscreenBuffer cmd{ displayHandle, bufferHandle, width, height, sampleCount, interruptible, depthBufferTypeInternal};
        m_pendingRendererCommands.push_back(std::move(cmd));

        return bufferId;
    }

    status_t RamsesRendererImpl::destroyOffscreenBuffer(displayId_t display, displayBufferId_t offscreenBuffer)
    {
        const ramses_internal::DisplayHandle displayHandle(display.getValue());
        const ramses_internal::OffscreenBufferHandle bufferHandle(offscreenBuffer.getValue());
        m_pendingRendererCommands.push_back(ramses_internal::RendererCommand::DestroyOffscreenBuffer{ displayHandle, bufferHandle });

        return StatusOK;
    }

    status_t RamsesRendererImpl::setDisplayBufferClearFlags(displayId_t display, displayBufferId_t displayBuffer, uint32_t clearFlags)
    {
        static_assert(
            static_cast<uint32_t>(ramses::EClearFlags_None) == static_cast<uint32_t>(ramses_internal::EClearFlags_None) &&
            static_cast<uint32_t>(ramses::EClearFlags_Color) == static_cast<uint32_t>(ramses_internal::EClearFlags_Color) &&
            static_cast<uint32_t>(ramses::EClearFlags_Depth) == static_cast<uint32_t>(ramses_internal::EClearFlags_Depth) &&
            static_cast<uint32_t>(ramses::EClearFlags_Stencil) == static_cast<uint32_t>(ramses_internal::EClearFlags_Stencil) &&
            static_cast<uint32_t>(ramses::EClearFlags_All) == static_cast<uint32_t>(ramses_internal::EClearFlags_All), "Type conversion mismatch");

        const auto it = m_displayFramebuffers.find(display);
        if (it == m_displayFramebuffers.cend())
            return addErrorEntry("RendererSceneControl::setDisplayBufferClearFlags failed: display does not exist.");

        ramses_internal::OffscreenBufferHandle bufferHandle{ displayBuffer.getValue() };
        // if buffer to clear is display's framebuffer pass invalid OB to internal renderer
        if (displayBuffer == it->second)
            bufferHandle = ramses_internal::OffscreenBufferHandle::Invalid();

        const ramses_internal::DisplayHandle displayHandle{ display.getValue() };
        m_pendingRendererCommands.push_back(ramses_internal::RendererCommand::SetClearFlags{ displayHandle, bufferHandle, clearFlags });

        return StatusOK;
    }

    status_t RamsesRendererImpl::setDisplayBufferClearColor(displayId_t display, displayBufferId_t displayBuffer, float r, float g, float b, float a)
    {
        const auto it = m_displayFramebuffers.find(display);
        if (it == m_displayFramebuffers.cend())
            return addErrorEntry("RendererSceneControl::setDisplayBufferClearColor failed: display does not exist.");

        ramses_internal::OffscreenBufferHandle bufferHandle{ displayBuffer.getValue() };
        // if buffer to clear is display's framebuffer pass invalid OB to internal renderer
        if (displayBuffer == it->second)
            bufferHandle = ramses_internal::OffscreenBufferHandle::Invalid();

        const ramses_internal::DisplayHandle displayHandle{ display.getValue() };
        m_pendingRendererCommands.push_back(ramses_internal::RendererCommand::SetClearColor{ displayHandle, bufferHandle, { r, g, b, a } });

        return StatusOK;
    }

    ramses::streamBufferId_t RamsesRendererImpl::allocateStreamBuffer()
    {
        const streamBufferId_t bufferId = m_nextStreamBufferId;
        m_nextStreamBufferId.getReference() = m_nextStreamBufferId.getValue() + 1;
        return bufferId;
    }

    streamBufferId_t RamsesRendererImpl::createStreamBuffer(displayId_t display, waylandIviSurfaceId_t source)
    {
        ramses::streamBufferId_t bufferId = allocateStreamBuffer();
        const ramses_internal::DisplayHandle displayHandle{ display.getValue() };
        const ramses_internal::StreamBufferHandle bufferHandle{ bufferId.getValue() };
        const ramses_internal::WaylandIviSurfaceId sourceLL{ source.getValue() };
        m_pendingRendererCommands.push_back(ramses_internal::RendererCommand::CreateStreamBuffer{ displayHandle, bufferHandle, sourceLL });

        return bufferId;
    }

    status_t RamsesRendererImpl::destroyStreamBuffer(displayId_t display, streamBufferId_t streamBuffer)
    {
        const ramses_internal::DisplayHandle displayHandle{ display.getValue() };
        const ramses_internal::StreamBufferHandle bufferHandle{ streamBuffer.getValue() };
        m_pendingRendererCommands.push_back(ramses_internal::RendererCommand::DestroyStreamBuffer{ displayHandle, bufferHandle });

        return StatusOK;
    }

    status_t RamsesRendererImpl::setStreamBufferState(displayId_t display, streamBufferId_t streamBufferId, bool state)
    {
        const ramses_internal::DisplayHandle displayHandle{ display.getValue() };
        const ramses_internal::StreamBufferHandle bufferHandle{ streamBufferId.getValue() };
        m_pendingRendererCommands.push_back(ramses_internal::RendererCommand::SetStreamBufferState{displayHandle, bufferHandle, state});
        return StatusOK;
    }

    status_t RamsesRendererImpl::readPixels(displayId_t displayId, displayBufferId_t displayBuffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        if (width == 0u || height == 0u)
            return addErrorEntry("RamsesRenderer::readPixels failed: width and height must be greater than Zero");

        const auto it = m_displayFramebuffers.find(displayId);
        if (it == m_displayFramebuffers.cend())
            return addErrorEntry("RendererSceneControl::setDisplayBufferClearColor failed: display does not exist.");

        ramses_internal::OffscreenBufferHandle bufferHandle{ displayBuffer.getValue() };
        // if buffer to read from is display's framebuffer pass invalid OB to internal renderer
        if (displayBuffer == it->second)
            bufferHandle = ramses_internal::OffscreenBufferHandle::Invalid();

        ramses_internal::RendererCommand::ReadPixels cmd{ ramses_internal::DisplayHandle{ displayId.getValue() }, bufferHandle, x, y, width, height, false, false, {} };
        m_pendingRendererCommands.push_back(std::move(cmd));

        return StatusOK;
    }

    status_t RamsesRendererImpl::systemCompositorSetIviSurfaceVisibility(uint32_t surfaceId, bool visibility)
    {
        if (!m_systemCompositorEnabled)
            return addErrorEntry("RamsesRenderer::setSurfaceVisibility failed: system compositor was not enabled when creating the renderer.");

        const ramses_internal::WaylandIviSurfaceId waylandIviSurfaceId(surfaceId);
        m_pendingRendererCommands.push_back(ramses_internal::RendererCommand::SCSetIviSurfaceVisibility{ waylandIviSurfaceId, visibility });
        return StatusOK;
    }

    status_t RamsesRendererImpl::systemCompositorSetIviSurfaceOpacity(uint32_t surfaceId, float opacity)
    {
        if (!m_systemCompositorEnabled)
            return addErrorEntry("RamsesRenderer::setSurfaceOpacity failed: system compositor was not enabled when creating the renderer.");

        const ramses_internal::WaylandIviSurfaceId waylandIviSurfaceId(surfaceId);
        m_pendingRendererCommands.push_back(ramses_internal::RendererCommand::SCSetIviSurfaceOpacity{ waylandIviSurfaceId, opacity });
        return StatusOK;
    }

    status_t RamsesRendererImpl::systemCompositorSetIviSurfaceRectangle(uint32_t surfaceId, int32_t x, int32_t y, int32_t width, int32_t height)
    {
        if (!m_systemCompositorEnabled)
            return addErrorEntry("RamsesRenderer::setSurfaceRectangle failed: system compositor was not enabled when creating the renderer.");

        const ramses_internal::WaylandIviSurfaceId waylandIviSurfaceId(surfaceId);
        m_pendingRendererCommands.push_back(ramses_internal::RendererCommand::SCSetIviSurfaceDestRectangle{ waylandIviSurfaceId, x, y, width, height });
        return StatusOK;
    }

    status_t RamsesRendererImpl::systemCompositorSetIviLayerVisibility(uint32_t layerId, bool visibility)
    {
        if (!m_systemCompositorEnabled)
            return addErrorEntry("RamsesRenderer::setLayerVisibility failed: system compositor was not enabled when creating the renderer.");

        const ramses_internal::WaylandIviLayerId waylandIviLayerId(layerId);
        m_pendingRendererCommands.push_back(ramses_internal::RendererCommand::SCSetIviLayerVisibility{ waylandIviLayerId, visibility });
        return StatusOK;
    }

    status_t RamsesRendererImpl::systemCompositorTakeScreenshot(const char* fileName, int32_t screenIviId)
    {
        if (!m_systemCompositorEnabled)
            return addErrorEntry("RamsesRenderer::takeSystemCompositorScreenshot failed: system compositor was not enabled when creating the renderer.");

        m_pendingRendererCommands.push_back(ramses_internal::RendererCommand::SCScreenshot{ screenIviId, fileName });
        return StatusOK;
    }

    status_t RamsesRendererImpl::systemCompositorAddIviSurfaceToIviLayer(uint32_t surfaceId, uint32_t layerId)
    {
        if (!m_systemCompositorEnabled)
            return addErrorEntry("RamsesRenderer::addSurfaceToLayer failed: system compositor was not enabled when creating the renderer.");

        const ramses_internal::WaylandIviSurfaceId waylandIviSurfaceId(surfaceId);
        const ramses_internal::WaylandIviLayerId waylandIviLayerId(layerId);
        m_pendingRendererCommands.push_back(ramses_internal::RendererCommand::SCAddIviSurfaceToIviLayer{ waylandIviSurfaceId, waylandIviLayerId });
        return StatusOK;
    }

    status_t RamsesRendererImpl::dispatchEvents(IRendererEventHandler& rendererEventHandler)
    {
        m_tempRendererEvents.clear();
        m_displayDispatcher->dispatchRendererEvents(m_tempRendererEvents);

        for(const auto& event : m_tempRendererEvents)
        {
            switch (event.eventType)
            {
            case ramses_internal::ERendererEventType::DisplayCreated:
                rendererEventHandler.displayCreated(displayId_t{ event.displayHandle.asMemoryHandle() }, ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType::DisplayCreateFailed:
                rendererEventHandler.displayCreated(displayId_t{ event.displayHandle.asMemoryHandle() }, ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType::DisplayDestroyed:
                rendererEventHandler.displayDestroyed(displayId_t{ event.displayHandle.asMemoryHandle() }, ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType::DisplayDestroyFailed:
                rendererEventHandler.displayDestroyed(displayId_t{ event.displayHandle.asMemoryHandle() }, ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType::ReadPixelsFromFramebuffer:
            case ramses_internal::ERendererEventType::ReadPixelsFromFramebufferFailed:
            {
                const ramses_internal::UInt8Vector& pixelData = event.pixelData;
                const displayId_t displayId{ event.displayHandle.asMemoryHandle() };
                const ramses_internal::OffscreenBufferHandle obHandle = event.offscreenBuffer;
                const displayBufferId_t displayBuffer(obHandle.isValid() ? obHandle.asMemoryHandle() : getDisplayFramebuffer(displayId).getValue());
                const auto eventResult = (event.eventType == ramses_internal::ERendererEventType::ReadPixelsFromFramebuffer ? ERendererEventResult_OK : ERendererEventResult_FAIL);
                assert((event.eventType == ramses_internal::ERendererEventType::ReadPixelsFromFramebuffer) ^ pixelData.empty());
                rendererEventHandler.framebufferPixelsRead(pixelData.data(), static_cast<uint32_t>(pixelData.size()), displayId, displayBuffer, eventResult);
                break;
            }
            case ramses_internal::ERendererEventType::WarpingDataUpdated:
                rendererEventHandler.warpingMeshDataUpdated(displayId_t{ event.displayHandle.asMemoryHandle() }, ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType::WarpingDataUpdateFailed:
                rendererEventHandler.warpingMeshDataUpdated(displayId_t{ event.displayHandle.asMemoryHandle() }, ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType::OffscreenBufferCreated:
                rendererEventHandler.offscreenBufferCreated(displayId_t{ event.displayHandle.asMemoryHandle() }, displayBufferId_t{ event.offscreenBuffer.asMemoryHandle() }, ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType::OffscreenBufferCreateFailed:
                rendererEventHandler.offscreenBufferCreated(displayId_t{ event.displayHandle.asMemoryHandle() }, displayBufferId_t{ event.offscreenBuffer.asMemoryHandle() }, ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType::OffscreenBufferDestroyed:
                rendererEventHandler.offscreenBufferDestroyed(displayId_t{ event.displayHandle.asMemoryHandle() }, displayBufferId_t{ event.offscreenBuffer.asMemoryHandle() }, ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType::OffscreenBufferDestroyFailed:
                rendererEventHandler.offscreenBufferDestroyed(displayId_t{ event.displayHandle.asMemoryHandle() }, displayBufferId_t{ event.offscreenBuffer.asMemoryHandle() }, ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType::WindowClosed:
                rendererEventHandler.windowClosed(displayId_t{ event.displayHandle.asMemoryHandle() });
                break;
            case ramses_internal::ERendererEventType::WindowKeyEvent:
                rendererEventHandler.keyEvent(displayId_t{ event.displayHandle.asMemoryHandle() },
                    RamsesRendererUtils::GetKeyEvent(event.keyEvent.type),
                    event.keyEvent.modifier, RamsesRendererUtils::GetKeyCode(event.keyEvent.keyCode));
                break;
            case ramses_internal::ERendererEventType::WindowMouseEvent:
                rendererEventHandler.mouseEvent(displayId_t{ event.displayHandle.asMemoryHandle() },
                    RamsesRendererUtils::GetMouseEvent(event.mouseEvent.type),
                    event.mouseEvent.pos.x, event.mouseEvent.pos.y);
                break;
            case ramses_internal::ERendererEventType::WindowResizeEvent:
                rendererEventHandler.windowResized(displayId_t{ event.displayHandle.asMemoryHandle() }, event.resizeEvent.width, event.resizeEvent.height);
                break;
            case ramses_internal::ERendererEventType::WindowMoveEvent:
                rendererEventHandler.windowMoved(displayId_t{ event.displayHandle.asMemoryHandle() }, event.moveEvent.posX, event.moveEvent.posY);
                break;
            case ramses_internal::ERendererEventType::FrameTimingReport:
                rendererEventHandler.renderThreadLoopTimings(event.frameTimings.maximumLoopTimeWithinPeriod, event.frameTimings.averageLoopTimeWithinPeriod);
                break;
            default:
                assert(false);
                return addErrorEntry("RamsesRenderer::dispatchEvents failed - unknown renderer event type!");
            }
        }

        return StatusOK;
    }

    status_t RamsesRendererImpl::logRendererInfo()
    {
        m_pendingRendererCommands.push_back(ramses_internal::RendererCommand::LogInfo{ ramses_internal::ERendererLogTopic::All, true, {} });
        return StatusOK;
    }

    status_t RamsesRendererImpl::startThread()
    {
        if (m_rendererLoopThreadType == ERendererLoopThreadType_UsingDoOneLoop)
            return addErrorEntry("RamsesRenderer::startThread Can not call startThread if doOneLoop is called before!");

        // first time starting thread, create dispatching thread
        if (m_rendererLoopThreadType == ERendererLoopThreadType_Undefined)
            m_commandDispatchingThread = std::make_unique<ramses_internal::CommandDispatchingThread>(*m_displayDispatcher, m_rendererCommandBuffer, m_threadWatchdog);

        m_rendererLoopThreadType = ERendererLoopThreadType_InRendererOwnThread;
        m_displayDispatcher->startDisplayThreadsUpdating();
        m_diplayThreadUpdating = true;

        return StatusOK;
    }

    status_t RamsesRendererImpl::stopThread()
    {
        if (ERendererLoopThreadType_InRendererOwnThread != m_rendererLoopThreadType)
            return addErrorEntry("RamsesRenderer::stopThread Can not call stopThread if startThread was not called before!");

        m_displayDispatcher->stopDisplayThreadsUpdating();
        m_diplayThreadUpdating = false;

        return StatusOK;
    }

    bool RamsesRendererImpl::isThreadRunning() const
    {
        return m_diplayThreadUpdating;
    }

    bool RamsesRendererImpl::isThreaded() const
    {
        return m_rendererLoopThreadType == ERendererLoopThreadType_InRendererOwnThread;
    }

    status_t RamsesRendererImpl::setMaximumFramerate(float maximumFramerate)
    {
        if (maximumFramerate <= 0.0f)
            return addErrorEntry("RamsesRenderer::setMaximumFramerate must specify a positive maximumFramerate!");

        if (ERendererLoopThreadType_UsingDoOneLoop == m_rendererLoopThreadType)
            return addErrorEntry("RamsesRenderer::setMaximumFramerate Can not call setMaximumFramerate if doOneLoop is called before because it can only control framerate for rendering thread!");

        m_displayDispatcher->setMinFrameDuration(std::chrono::microseconds{ std::lround(1000000 / maximumFramerate) });
        m_maxFramerate = maximumFramerate;

        return StatusOK;
    }

    status_t RamsesRendererImpl::setMaximumFramerate(float maximumFramerate, displayId_t display)
    {
        if (maximumFramerate <= 0.0f)
            return addErrorEntry("RamsesRenderer::setMaximumFramerate must specify a positive maximumFramerate!");

        if (ERendererLoopThreadType_UsingDoOneLoop == m_rendererLoopThreadType)
            return addErrorEntry("RamsesRenderer::setMaximumFramerate Can not call setMaximumFramerate if doOneLoop is called before because it can only control framerate for rendering thread!");

        m_displayDispatcher->setMinFrameDuration(std::chrono::microseconds{ std::lround(1000000 / maximumFramerate) }, ramses_internal::DisplayHandle{ display.getValue() });

        return StatusOK;
    }

    float RamsesRendererImpl::getMaximumFramerate() const
    {
        return m_maxFramerate;
    }

    status_t RamsesRendererImpl::setLoopMode(ELoopMode loopMode)
    {
        switch (loopMode)
        {
        case ELoopMode_UpdateAndRender:
            m_loopMode = ramses_internal::ELoopMode::UpdateAndRender;
            break;
        case ELoopMode_UpdateOnly:
            m_loopMode = ramses_internal::ELoopMode::UpdateOnly;
            break;
        }

        m_displayDispatcher->setLoopMode(m_loopMode);

        return StatusOK;
    }

    ELoopMode RamsesRendererImpl::getLoopMode() const
    {
        switch (m_loopMode)
        {
        case ramses_internal::ELoopMode::UpdateAndRender:
            return ELoopMode_UpdateAndRender;
        case ramses_internal::ELoopMode::UpdateOnly:
            return ELoopMode_UpdateOnly;
        }

        assert(false);
        return ELoopMode_UpdateAndRender;
    }

    ramses::status_t RamsesRendererImpl::setFrameTimerLimits(uint64_t limitForSceneResourcesUpload, uint64_t limitForClientResourcesUpload, uint64_t limitForOffscreenBufferRender)
    {
        m_pendingRendererCommands.push_back(ramses_internal::RendererCommand::SetLimits_FrameBudgets{ limitForSceneResourcesUpload, limitForClientResourcesUpload, limitForOffscreenBufferRender });
        return StatusOK;
    }

    status_t RamsesRendererImpl::setPendingFlushLimits(uint32_t forceApplyFlushLimit, uint32_t forceUnsubscribeSceneLimit)
    {
        m_pendingRendererCommands.push_back(ramses_internal::RendererCommand::SetLimits_FlushesForceApply{ forceApplyFlushLimit });
        m_pendingRendererCommands.push_back(ramses_internal::RendererCommand::SetLimits_FlushesForceUnsubscribe{ forceUnsubscribeSceneLimit });
        return StatusOK;
    }

    status_t RamsesRendererImpl::setSkippingOfUnmodifiedBuffers(bool enable)
    {
        m_pendingRendererCommands.push_back(ramses_internal::RendererCommand::SetSkippingOfUnmodifiedBuffers{ enable });
        return StatusOK;
    }

    void RamsesRendererImpl::pushAndConsumeRendererCommands(ramses_internal::RendererCommands& cmds)
    {
        m_rendererCommandBuffer.addAndConsumeCommandsFrom(cmds);
    }

    const ramses::RamsesRendererImpl::DisplayFrameBufferMap& RamsesRendererImpl::getDisplayFrameBuffers() const
    {
        return m_displayFramebuffers;
    }
}
