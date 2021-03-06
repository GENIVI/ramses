//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESRENDERER_H
#define RAMSES_RAMSESRENDERER_H

#include "ramses-framework-api/StatusObject.h"
#include "ramses-renderer-api/RendererConfig.h"
#include "ramses-framework-api/RamsesFramework.h"

namespace ramses
{
    class SystemCompositorController;
    class DisplayConfig;
    class IRendererEventHandler;
    class WarpingMeshData;
    class RendererSceneControl;
    class DcsmContentControl;

    /**
    * @brief RamsesRenderer is the main renderer component which provides API to configure
    *        and control the way content will be rendered on display(s).
    * @details All the commands in this class are put to a queue and submitted only when RamsesRenderer::flush is called,
    *          they are then executed asynchronously in the renderer core, the order of execution is preserved.
    *          Most of the commands have a corresponding callback which reports the result back to the caller
    *          via RamsesRenderer::dispatchEvents.
    *          Some commands can fail immediately by returning a status with value other than StatusOK,
    *          in such case there will be no callback, because the command will not even be submitted.
    *          RamsesRenderer API is not thread-safe.
    */
    class RAMSES_API RamsesRenderer : public StatusObject
    {
    public:
        /**
        * @brief   Prepare content to be rendered in next frame and render next frame.
        * @details Update and render of all displays is executed sequentially in caller's thread.
        *          For active rendering it is recommended to use threaded mode instead (#startThread).
        *          Once this method is called a threaded mode cannot be used anymore.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t doOneLoop();

        /**
        * @brief   Starts update and render loop in threaded mode.
        * @details Each display will update and render in its own thread.
        *          First call to this method enables threaded mode, afterwards it is not possible to call
        *          #doOneLoop anymore.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t startThread();

        /**
        * @brief   Stops thread(s) running the update and render of displays.
        * @details This function can only be used if startThread was successfully called before.
        *          The looping can be restarted by calling #startThread again.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t stopThread();

        /**
        * @brief Get the current state of rendering thread(s) running
        *
        * @return Returns true if thread is running (started and not stopped), false otherwise.
        */
        bool isThreadRunning() const;

        /**
        * @brief   Sets the maximum frame rate per second for the update/render loop when in threaded mode.
        * @details The parameter is of type float in order to specify any desired frame time (e.g. below 1 FPS).
        *          The same maximum frame rate will be applied to all displays.
        *          This function can only be used in threaded mode (#startThread).
        *          The default value is 60 FPS.
        * @param maximumFramerate The maximum frame rate per second to set for the render loop.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setMaximumFramerate(float maximumFramerate);

        /**
        * @brief   Sets the maximum frame rate per second for the update/render loop when in threaded mode for given display
        * @details The maximumFramerate parameter is of type float in order to specify any desired frame time (e.g. below 1 FPS).
        *          The maximum frame rate will be applied to the display thread responsible for given display id.
        *          This function can only be used in threaded mode (#startThread).
        *          The default value is 60 FPS.
        * @param renderer The renderer to apply the value to. This api is added in static fashion for ABI compatibility.
        * @param maximumFramerate The maximum frame rate per second to set for the render loop.
        * @param displayId The display id the framerate should be applied to.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        static status_t setMaximumFramerate(RamsesRenderer& renderer, float maximumFramerate, displayId_t displayId);

        /**
        * @brief Get the current value for maximum frame rate per second currently set (#setMaximumFramerate).
        *
        * @return Maximum frame rate per second
        */
        float getMaximumFramerate() const;

        /**
        * @brief   Sets the mode of operation for render loop.
        * @details Mode can be changed during run-time, in case of threaded mode also while running
        *          (no need to #stopThread).
        *          By default loop mode is set to render and update.
        *
        * @param loopMode The mode to be used for render loop.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setLoopMode(ELoopMode loopMode);

        /**
        * @brief Get the current value for loop mode set using #setLoopMode
        *
        * @return The loop mode
        */
        ELoopMode getLoopMode() const;

        /**
        * @brief Sets time limits for time-out of different sections of render and update loop.
        * @details Setting time limits for render/update loop sections instructs the renderer to monitor the amount of time
        *          consumed by the sections and interrupt their execution if the set time limit was exceeded.
        *          The time limit for every section is calculated since beginning of frame.
        *          If a section is interrupted the renderer will execute the sections in the same order in the next frame,
        *          but it will continue to execute the interrupted section(s) from the point where it stopped.
        *
        *          Since time limits are calculated relative to the start of the frame, the values set
        *          should be monotonically increasing in the order of:
        *            1. Uploading client resources
        *            2. Uploading scene resources
        *            3. Rendering scenes mapped to interruptible offscreen buffers
        *
        *          By default sections have infinite time limit, so renderer would not try to interrupt their execution.
        *
        * !! IMPORTANT !! Scene resource actions can not be interrupted like other resources. Therefore, if this timer is exceeded, a scene will be
        * force-unsubscribed. Use this timer with caution and merely as a sanity check, NOT as a performance measure! Scenes should not be over-using
        * scene resources, precisely because they can not be interrupted.
        *
        * @param[in] limitForSceneResourcesUpload  Time limit in microseconds (since beginning of frame) for uploading scene resources to GPU
        * @param[in] limitForClientResourcesUpload Time limit in microseconds (since beginning of frame) for uploading client resources to GPU
        * @param[in] limitForOffscreenBufferRender Time limit in microseconds (since beginning of frame) for rendering scenes that are mapped to interruptible offscreen buffers
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        *         StatusOK does not guarantee successful read back, the result event has its own status.
        */
        status_t setFrameTimerLimits(uint64_t limitForSceneResourcesUpload, uint64_t limitForClientResourcesUpload, uint64_t limitForOffscreenBufferRender);

        /**
        * @brief Sets the number of pending flushes accepted before force-applying them to their scene, or forcefully insubscribing the scene.
        *
        * @details RAMSES guarantees that a scene flush is only going to be rendered if all resources (textures,
        * shaders etc.) are received and uploaded to the GPU. This allows a misbehaving client to flood a renderer
        * with flushes which are never executed, causing the renderer to exhaust heap memory. If too many pending flushes
        * are received and queued, they are force-applied to the scene, even if some resources are missing, thus potentially
        * causing flickering. This method allows overriding the number of flushes after a scene is updated to the latest
        * flush and all pending flushes wiped from memory. A low number (e.g. 1) will cause frequent flickering,
        * whereas a high number (e.g. 100000) could cause the renderer to go out of memory. The second parameter
        * (forceUnsubscribeSceneLimit) controls after how many pending flushes a scene is completely unsubscribed.
        * This can be used as a protection against malicious remote scenes. It does not affect local scenes though.
        * It is advisable to set forceUnsubscribeSceneLimit to higher number than forceApplyFlushLimit,
        * because re-subscribing a scene is causing a lot of network traffic and unnecessary memory operations, not to mention the
        * scene is then also not visible until re-subscribed, mapped and shown.
        *
        * @param[in] forceApplyFlushLimit Number of flushes that can be pending before force applying occurs.
        * @param[in] forceUnsubscribeSceneLimit Number of flushes that can be pending before force un-subscribe occurs.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setPendingFlushLimits(uint32_t forceApplyFlushLimit, uint32_t forceUnsubscribeSceneLimit);

        /**
        * @brief     Enable or disable skipping of rendering of unmodified buffers.
        *            By default the renderer does not re-render buffers (framebuffer or offscreen buffer)
        *            if there was no change to any of the content assigned to the buffer.
        *            This can save hardware resources or free up resources for rendering of other buffers.
        *            It can be however desired to disable such optimization for profiling of worst case scenario
        *            or debugging of a graphical glitch.
        *
        * @param[in] enable  Enable or disable the feature (enabled initially)
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        *         StatusOK does not guarantee successful read back, the result event has its own status.
        */
        status_t setSkippingOfUnmodifiedBuffers(bool enable = true);

        /**
         * @brief Creates a display based on provided display config.
         *        Creation of a display is an asynchronous action and the display will be created during the next render loop.
         *        The result of the createDisplay can be retrieved via dispatchEvents which will trigger a callback
         *        after the asynchronous action was processed.
         *
         * @param config The display config to create and configure the new display.
         * @return Display id that can be used to refer to the created display.
         *         displayId_t::Invalid() in case of error. Display creation can still fail even
         *         if a valid display id is returned, the result of the actual creation
         *         can be retrieved via dispatchEvents.
         */
        displayId_t createDisplay(const DisplayConfig& config);

        /**
        * @brief Destroy a display.
        *        Destruction of a display is an asynchronous action and the actual display components (window, device, etc.)
        *        will be released during the next render loop.
        *        The result of the destroyDisplay can be retrieved via dispatchEvents which will trigger a callback with the result
        *        after the asynchronous action was processed.
        *
        * @param displayId The display id of the display to destroy.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t destroyDisplay(displayId_t displayId);

        /**
        * @brief Get display's framebuffer ID.
        *        Every display upon creation has one framebuffer which can be referenced by a display buffer ID
        *        to be used in various API methods that work with either a framebuffer or an offscreen buffer (e.g. RamsesRenderer::setBufferClearColor).
        *
        * @param displayId The ID of display for which the framebuffer ID is being queried.
        * @return Display's framebuffer ID or invalid ID if display does not exist.
        */
        displayBufferId_t getDisplayFramebuffer(displayId_t displayId) const;

        /**
        * @brief   Will create an offscreen buffer that can be used to render scenes into (see RamsesRenderer::assignSceneToDisplayBuffer)
        *          and can be linked as input to a consumer texture sampler (see RamsesRenderer::linkOffscreenBufferToConsumer).
        * @details The created offscreen buffer always has color, depth and stencil buffer attached.
        *          A multisampled buffer will be created if sampleCount greater than 0, note that the value is just a hint for the device,
        *          the actual number of samples might be different depending on device driver implementation.
        *          If the number of samples exceeds device capabilities the number of samples it will be clamped to its
        *          maximum supported (creation will succeeded with a warning log).
        *
        * @param[in] display id of display for which the buffer should be created
        * @param[in] width width of the buffer to be created (has to be higher than 0 and lower than 4096)
        * @param[in] height height of the buffer to be created (has to be higher than 0 and lower than 4096)
        * @param[in] sampleCount Optional sample count for MSAA. Default value is 0 for no MSAA.
        * @return Identifier of the created offscreen buffer.
        *         In case of unsupported resolution \c displayBufferId_t::Invalid() will be returned with no renderer event generated.
        *         Note that the buffer will be created asynchronously and there will be a renderer event once the operation is finished.
        */
        displayBufferId_t createOffscreenBuffer(displayId_t display, uint32_t width, uint32_t height, uint32_t sampleCount = 0u);

        /**
        * @brief Will create an offscreen buffer that can be used to render scenes into (see RamsesRenderer::assignSceneToDisplayBuffer)
        *        and can be linked as input to a consumer texture sampler (see RamsesRenderer::linkOffscreenBufferToConsumer).
        *
        *        The created offscreen buffer always has a color buffer, but depth and stencil buffers can be configured.
        *
        * @param[in] renderer The renderer to call this method on. This API is temporarily added in static fashion for ABI compatibility.
        * @param[in] display id of display for which the buffer should be created
        * @param[in] width width of the buffer to be created (has to be higher than 0 and lower than 4096)
        * @param[in] height height of the buffer to be created (has to be higher than 0 and lower than 4096)
        * @param[in] sampleCount Optional sample count for MSAA number of samples. Default value is Zero, which disables MSAA for the offscreen buffer.
        * @param[in] depthBufferType Optional setting to configure depth and stencil buffers.
        * @return Identifier of the created offscreen buffer.
        *         In case of unsupported resolution \c displayBufferId_t::Invalid() will be returned with no renderer event generated.
        *         Note that the buffer will be created asynchronously and there will be a renderer event once the operation is finished.
        */
        static displayBufferId_t createOffscreenBuffer(RamsesRenderer& renderer, displayId_t display, uint32_t width, uint32_t height, uint32_t sampleCount = 0u, EDepthBufferType depthBufferType = EDepthBufferType_DepthStencil);

        /**
        * @brief     Additional API to create an offscreen buffer as interruptible.
        *            This allows the renderer to interrupt rendering of scenes to such offscreen buffer
        *            if the time budget for rendering is exceeded within a frame (see SetFrameTimerLimits).
        *            The rendering continues next frame starting from the interruption point.
        *
        *            The renderer creates two render targets on GPU (front and back) for every
        *            interruptible offscreen buffer. It then renders into the back render target
        *            of the offscreen buffer, while it is possible to read the content of front render target
        *            with content from previous frame (or older if interrupted for several frames).
        *
        *            Note that whenever a scene gets assigned to interruptible offscreen buffer,
        *            it is not guaranteed anymore that it will be fully rendered every frame.
        *            Essentially it is rendered with lower priority.
        *
        *            The created offscreen buffer always has color, depth and stencil buffers.
        *
        * @param[in] display  Id of display for which the buffer should be created
        * @param[in] width    Width of the buffer to be created (has to be higher than 0 and lower than 4096)
        * @param[in] height   Height of the buffer to be created (has to be higher than 0 and lower than 4096)
        * @return Identifier of the created offscreen buffer.
        *         In case of unsupported resolution \c displayBufferId_t::Invalid() will be returned with no renderer event generated.
        *         Note that the buffer will be created asynchronously and there will be a renderer event once the operation is finished.
        */
        displayBufferId_t createInterruptibleOffscreenBuffer(displayId_t display, uint32_t width, uint32_t height);

        /**
        * @brief     Additional API to create an offscreen buffer as interruptible.
        *            This allows the renderer to interrupt rendering of scenes to such offscreen buffer
        *            if the time budget for rendering is exceeded within a frame (see SetFrameTimerLimits).
        *            The rendering continues next frame starting from the interruption point.
        *
        *            The renderer creates two render targets on GPU (front and back) for every
        *            interruptible offscreen buffer. It then renders into the back render target
        *            of the offscreen buffer, while it is possible to read the content of front render target
        *            with content from previous frame (or older if interrupted for several frames).
        *
        *            Note that whenever a scene gets assigned to interruptible offscreen buffer,
        *            it is not guaranteed anymore that it will be fully rendered every frame.
        *            Essentially it is rendered with lower priority.
        *
        *            The created offscreen buffer always has a color buffer, but depth and stencil buffers can be configured.
        *
        * @param[in] renderer The renderer to call this method on. This API is temporarily added in static fashion for ABI compatibility.
        * @param[in] display  Id of display for which the buffer should be created
        * @param[in] width    Width of the buffer to be created (has to be higher than 0 and lower than 4096)
        * @param[in] height   Height of the buffer to be created (has to be higher than 0 and lower than 4096)
        * @param[in] depthBufferType Optional setting to configure depth and stencil buffers.
        * @return Identifier of the created offscreen buffer.
        *         In case of unsupported resolution \c displayBufferId_t::Invalid() will be returned with no renderer event generated.
        *         Note that the buffer will be created asynchronously and there will be a renderer event once the operation is finished.
        */
        static displayBufferId_t createInterruptibleOffscreenBuffer(RamsesRenderer& renderer, displayId_t display, uint32_t width, uint32_t height, EDepthBufferType depthBufferType = EDepthBufferType_DepthStencil);

        /**
        * @brief Will destroy a previously created offscreen buffer.
        *        If there are any consumer texture samplers linked to this buffer, these links will be removed.
        *        Offscreen buffer will fail to be destroyed if there are any scenes assigned to it, these scenes
        *        have to be first assigned to another buffer or framebuffer or unmapped from display.
        *
        * @param[in] display id of display which the buffer belongs to
        * @param[in] offscreenBuffer id of buffer to destroy
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t destroyOffscreenBuffer(displayId_t display, displayBufferId_t offscreenBuffer);

        /**
        * @brief   Sets clear flags for a display buffer (display's framebuffer or offscreen buffer).
        * @details By default all display buffers' color, depth and stencil are cleared every frame when they are rendered to.
        *          This can be overridden for performance or special effect reasons.
        *          There is no event callback for this operation, the change can be assumed to be effective
        *          in the next frame rendered after flushed.
        *
        * @param[in] renderer The renderer to call this method on. This API is temporarily added in static fashion for ABI compatibility.
        * @param[in] display Id of display that the buffer to set clearing belongs to.
        * @param[in] displayBuffer Id of display buffer to set clearing,
        *                          if #ramses::displayBufferId_t::Invalid() is passed then the clearing is set for display's framebuffer.
        * @param[in] clearFlags Bitmask of the #ramses::EClearFlags, use bit OR to select which buffer component to clear
        *                       or #ramses::EClearFlags_All to clear all (default).
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        static status_t setDisplayBufferClearFlags(RamsesRenderer& renderer, displayId_t display, displayBufferId_t displayBuffer, uint32_t clearFlags);

        /**
        * @brief   Sets clear color of a display buffer (display's framebuffer or offscreen buffer).
        * @details Clear color is used to clear the whole buffer at the beginning of a rendering cycle (typically every frame).
        *          Default clear color is (0, 0, 0, 1).
        *          There is no event callback for this operation, the clear color change can be assumed to be effective
        *          in the next frame rendered after flushed.
        *
        * @param[in] display Id of display that the buffer to set clear color belongs to.
        * @param[in] displayBuffer Id of display buffer to set clear color,
        *                          if #ramses::displayBufferId_t::Invalid() is passed then the clear color is set for display's framebuffer.
        * @param[in] r Clear color red channel value [0,1]
        * @param[in] g Clear color green channel value [0,1]
        * @param[in] b Clear color blue channel value [0,1]
        * @param[in] a Clear color alpha channel value [0,1]
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setDisplayBufferClearColor(displayId_t display, displayBufferId_t displayBuffer, float r, float g, float b, float a);

        /**
        * @brief Triggers an asynchronous read back of a display buffer memory from GPU to system memory.
        * @details The color data from the provided rectangle coordinates
        *          will be read back and stored as RGBA8. If the coordinates
        *          lie outside the rendered region the result is undefined.
        *
        *          If a read pixels command is issued for a display buffer
        *          while a previous read pixels command for the same buffer was not
        *          yet executed only the last submitted read pixel command gets executed.
        *
        *          The pixel data can be obtained as a renderer event after the asynchronous read back is finished,
        *          see RamsesRenderer::dispatchEvents for details.
        * @param[in] displayId id of display to read pixels from.
        * @param[in] displayBuffer Id of display buffer to read pixels from,
        *                          if #ramses::displayBufferId_t::Invalid() is passed then pixels are read from the display's framebuffer.
        * @param[in] x The starting offset in the original image (i.e. left border) in pixels.
        * @param[in] y The starting offset in the original image (i.e. lower border) in pixels.
        *          The origin of the image is supposed to be in the lower left corner.
        * @param[in] width The width of the read image in pixels. Must be greater than Zero.
        * @param[in] height The height of the read image in pixels. Must be greater than Zero.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        *         StatusOK does not guarantee successful read back, the result event has its own status.
        */
        status_t readPixels(displayId_t displayId, displayBufferId_t displayBuffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height);

        /**
        * @brief Get scene control API
        * @details Typical application using Ramses has different components controlling the renderer
        *          (display management, frame limits and looping control, etc.) and controlling the states
        *          of content to be rendered (show/hide scene, data link, assign to display buffer, etc.).
        *          The scene control part can be obtained using this method, calling this method
        *          multiple times is allowed and will always return the same pointer, i.e. there is only
        *          a single instance per RamsesRenderer.
        *          This method will return nullptr in case an internal policy disallows controlling of scenes
        *          through this API - this could mean that there is another, incompatible scene control
        *          mechanism in use.
        *          Scene control API has its own independent flush and event dispatching,
        *          see #ramses::RendererSceneControl for details.
        *
        *          Obtaining the #ramses::RendererSceneControl will disallow usage of different type of scene control
        *          (#ramses::DcsmContentControl).
        *          #RamsesRenderer is owner of the #ramses::RendererSceneControl API and the pointer
        *          stays valid as long as this #RamsesRenderer instance is alive. It cannot be destroyed
        *          without destroying the #RamsesRenderer.
        *
        * @return Pointer to scene control API, or nullptr on error
        */
        RendererSceneControl* getSceneControlAPI();

        /**
        * @brief Create #ramses::DcsmContentControl to control content states
        * @details #ramses::DcsmContentControl can be used to control content states (a DCSM content is an abstraction
        *          for a scene). In addition #ramses::DcsmContentControl handles content states in the DCSM protocol
        *          context, it is essentially a combination of renderer scene control API and #ramses::DcsmConsumer.
        *          There can be only a single instance of #ramses::DcsmContentControl within #RamsesRenderer. Calling
        *          this method more than once will fail.
        *          This method will return nullptr in case an internal policy disallows controlling of scenes
        *          through this API - this could mean that there is another, incompatible scene control
        *          mechanism in use.
        *          #ramses::DcsmContentControl has its own event dispatching mechanism,
        *          see #ramses::DcsmContentControl for details.
        *
        *          Obtaining the #ramses::DcsmContentControl will disallow usage of different type of scene control
        *          (#ramses::RendererSceneControl).
        *          #RamsesRenderer is owner of the #ramses::DcsmContentControl API and the pointer
        *          stays valid as long as this #RamsesRenderer instance is alive. It cannot be destroyed
        *          without destroying the #RamsesRenderer.
        *
        * @return Pointer to #ramses::DcsmContentControl, or nullptr on error
        */
        DcsmContentControl* createDcsmContentControl();

        /////////////////////////////////////////////////
        //      System Compositor API
        /////////////////////////////////////////////////

        /**
        * @brief Set visibility of given surface at the system compositor
        * @param surfaceId id of the surface to set visibility of
        * @param visibility visibility to set
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        **/
        status_t setSurfaceVisibility(uint32_t surfaceId, bool visibility);

        /**
        * @brief Set opacity of given surface at the system compositor
        * @param surfaceId id of the surface to set opacity of
        * @param opacity Opacity in the range 0.0 (fully transparent) to 1.0 (fully opaque)
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        **/
        status_t setSurfaceOpacity(uint32_t surfaceId, float opacity);

        /**
        * @brief Set output rectangle of given surface at the system compositor
        * @param surfaceId id of the surface to set the rectangle for
        * @param x Output position of surface along the x-axis
        * @param y Output position of surface along the y-axis
        * @param width Output width of surface
        * @param height Output height of surface
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setSurfaceRectangle(uint32_t surfaceId, int32_t x, int32_t y, int32_t width, int32_t height);

        /**
        * @brief Set visibility of given layer at the system compositor
        * @param layerId id identifying the layer
        * @param visibility If \c true the layer's visibility will be enabled, otherwise disabled
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        **/
        status_t setLayerVisibility(uint32_t layerId, bool visibility);

        /**
        * @brief Trigger the System Compositor to take a screenshot and store it in a file.
        * @param fileName File name including path, for storing the screenshot.
        * @param screenIviId >= 0 to trigger a screenshot on the given IVI screen id, -1 to trigger screenshot
        *             on a single existing screen (fails asynchronously if more than one screen exists)
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t takeSystemCompositorScreenshot(const char* fileName, int32_t screenIviId);

        /////////////////////////////////////////////////
        //      End of System Compositor API
        /////////////////////////////////////////////////

        /**
        * @brief Updates the warping mesh for the warping postprocessing based on the new config.
        * @details Display must be created with warping enabled in order for this operation to succeed.
        *
        * @param[in] displayId id of display to update.
        * @param[in] newWarpingMeshData Holds the geometry needed to create the mesh for display warping.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t updateWarpingMeshData(displayId_t displayId, const WarpingMeshData& newWarpingMeshData);

        /**
        * @brief Most RamsesRenderer methods push commands to an internal queue which is submitted
        *        when calling RamsesRenderer::flush. The commands are then executed during a render loop
        *        (RamsesRenderer::doOneLoop or in a render thread if used RamsesRenderer::startThread).
        *        Some of these calls result in an event (can be both informational and data).
        *        Such events and their result can be retrieved using the dispatchEvents call.
        *        *IMPORTANT* Renderer events must be regularly consumed by calling dispatchEvents()
        *        in order to prevent buffer overflow of the internal event queue,
        *        even if the application is not interested in those events.
        *
        * @param rendererEventHandler User class that implements the callbacks that can be triggered if a corresponding event happened.
        *                             Check ramses::IRendererEventHandler documentation for more details.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t dispatchEvents(IRendererEventHandler& rendererEventHandler);

        /**
        * @brief Submits renderer commands (API calls on this instance of RamsesRenderer)
        *        since previous flush to be executed in the next renderer update loop.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t flush();

        /**
        * @brief Prints detailed information about renderer state and contents to the log output.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t logRendererInfo();

        /**
        * Stores internal data for implementation specifics of RamsesRenderer
        */
        class RamsesRendererImpl& impl;

        /**
         * @brief Constructor of RamsesRenderer
         */
        explicit RamsesRenderer(RamsesRendererImpl&);

        /**
         * @brief Deleted default constructor
         */
        RamsesRenderer() = delete;

        /**
         * @brief Deleted copy constructor
         * @param other unused
         */
        RamsesRenderer(const RamsesRenderer& other) = delete;

        /**
         * @brief Deleted copy assignment
         * @param other unused
         * @return unused
         */
        RamsesRenderer& operator=(const RamsesRenderer& other) = delete;

    private:
        /**
        * @brief RendererFactory is the factory for RamsesRenderer
        */
        friend class RendererFactory;

        /**
        * @brief Destructor of RamsesRenderer
        */
        virtual ~RamsesRenderer();
    };
}

#endif
