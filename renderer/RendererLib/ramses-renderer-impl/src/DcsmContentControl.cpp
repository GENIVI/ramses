//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-renderer-api/DcsmContentControl.h"
#include "DcsmContentControlImpl.h"
#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-framework-api/RamsesFramework.h"
#include "ramses-framework-api/DcsmConsumer.h"
#include "DcsmConsumerImpl.h"
#include "APILoggingMacros.h"
#include "RamsesFrameworkTypesImpl.h"
#include "ramses-framework-api/CategoryInfoUpdate.h"

namespace ramses
{
    DcsmContentControl::DcsmContentControl(DcsmContentControlImpl& impl_)
        : StatusObject(impl_)
        , m_impl(impl_)
    {
    }

    DcsmContentControl::~DcsmContentControl() = default;

    status_t DcsmContentControl::addContentCategory(Category category, displayId_t display, const CategoryInfoUpdate& categoryInformation)
    {
        const auto status = m_impl.addContentCategory(category, display, categoryInformation);
        LOG_HL_RENDERER_API4(status, category, display, categoryInformation.getCategoryRect().width, categoryInformation.getCategoryRect().height);
        return status;
    }

    status_t DcsmContentControl::removeContentCategory(Category category)
    {
        const auto status = m_impl.removeContentCategory(category);
        LOG_HL_RENDERER_API1(status, category);
        return status;
    }

    status_t DcsmContentControl::requestContentReady(ContentID contentID, uint64_t timeOut)
    {
        const auto status = m_impl.requestContentReady(contentID, timeOut);
        LOG_HL_RENDERER_API2(status, contentID, timeOut);
        return status;
    }

    status_t DcsmContentControl::showContent(ContentID contentID, AnimationInformation timingInfo)
    {
        const auto status = m_impl.showContent(contentID, timingInfo);
        LOG_HL_RENDERER_API3(status, contentID, timingInfo.startTime, timingInfo.finishTime);
        return status;
    }

    status_t DcsmContentControl::hideContent(ContentID contentID, AnimationInformation timingInfo)
    {
        const auto status = m_impl.hideContent(contentID, timingInfo);
        LOG_HL_RENDERER_API3(status, contentID, timingInfo.startTime, timingInfo.finishTime);
        return status;
    }

    status_t DcsmContentControl::releaseContent(ContentID contentID, AnimationInformation timingInfo)
    {
        const auto status = m_impl.releaseContent(contentID, timingInfo);
        LOG_HL_RENDERER_API3(status, contentID, timingInfo.startTime, timingInfo.finishTime);
        return status;
    }

    status_t DcsmContentControl::setCategoryInfo(Category categoryId, const CategoryInfoUpdate& categoryInfo, AnimationInformation timingInfo)
    {
        const auto status = m_impl.setCategoryInfo(categoryId, categoryInfo, timingInfo);
        LOG_HL_RENDERER_API4(status, categoryId, categoryInfo, timingInfo.startTime, timingInfo.finishTime);
        return status;
    }

    status_t DcsmContentControl::acceptStopOffer(ContentID contentID, AnimationInformation timingInfo)
    {
        const auto status = m_impl.acceptStopOffer(contentID, timingInfo);
        LOG_HL_RENDERER_API3(status, contentID, timingInfo.startTime, timingInfo.finishTime);
        return status;
    }

    status_t DcsmContentControl::assignContentToDisplayBuffer(ContentID contentID, displayBufferId_t displayBuffer, int32_t renderOrder)
    {
        const auto status = m_impl.assignContentToDisplayBuffer(contentID, displayBuffer, renderOrder);
        LOG_HL_RENDERER_API3(status, contentID, displayBuffer, renderOrder);
        return status;
    }

    status_t DcsmContentControl::linkOffscreenBuffer(displayBufferId_t offscreenBufferId, ContentID consumerContentID, dataConsumerId_t consumerId)
    {
        const auto status = m_impl.linkOffscreenBuffer(offscreenBufferId, consumerContentID, consumerId);
        LOG_HL_RENDERER_API3(status, offscreenBufferId, consumerContentID, consumerId);
        return status;
    }

    status_t DcsmContentControl::linkContentToTextureConsumer(ContentID contentID, ContentID consumerContentID, dataConsumerId_t consumerId)
    {
        const auto status = m_impl.linkContentToTextureConsumer(contentID, consumerContentID, consumerId);
        LOG_HL_RENDERER_API3(status, contentID, consumerContentID, consumerId);
        return status;
    }

    status_t DcsmContentControl::linkData(ContentID providerContentID, dataProviderId_t providerId, ContentID consumerContentID, dataConsumerId_t consumerId)
    {
        const auto status = m_impl.linkData(providerContentID, providerId, consumerContentID, consumerId);
        LOG_HL_RENDERER_API4(status, providerContentID, providerId, consumerContentID, consumerId);
        return status;
    }

    status_t DcsmContentControl::unlinkData(ContentID consumerContentID, dataConsumerId_t consumerId)
    {
        const auto status = m_impl.unlinkData(consumerContentID, consumerId);
        LOG_HL_RENDERER_API2(status, consumerContentID, consumerId);
        return status;
    }

    status_t DcsmContentControl::handlePickEvent(ContentID contentID, float bufferNormalizedCoordX, float bufferNormalizedCoordY)
    {
        const auto status = m_impl.handlePickEvent(contentID, bufferNormalizedCoordX, bufferNormalizedCoordY);
        LOG_HL_RENDERER_API3(status, contentID, bufferNormalizedCoordX, bufferNormalizedCoordY);
        return status;
    }

    status_t DcsmContentControl::update(uint64_t timeStampNow, IDcsmContentControlEventHandler& eventHandler)
    {
        const auto status = m_impl.update(timeStampNow, eventHandler);
        LOG_HL_RENDERER_API2(status, timeStampNow, LOG_API_GENERIC_OBJECT_STRING(eventHandler));
        return status;
    }

    status_t DcsmContentControl::sendContentStatus(ContentID contentID, DcsmStatusMessage const& message)
    {
        const auto status = m_impl.sendContentStatus(contentID, message);
        LOG_HL_RENDERER_API2(status, contentID, LOG_API_GENERIC_OBJECT_STRING(message));
        return status;
    }

}
