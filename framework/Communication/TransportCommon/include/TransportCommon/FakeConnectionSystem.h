//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FAKECONNECTIONSYSTEM_H
#define RAMSES_FAKECONNECTIONSYSTEM_H

#include "TransportCommon/FakeConnectionStatusUpdateNotifier.h"
#include "ICommunicationSystem.h"
#include "Components/ManagedResource.h"

namespace ramses_internal
{
    class SceneActionCollection;

    class FakeConnectionSystem : public ICommunicationSystem
    {
    public:
        virtual bool connectServices() override
        {
            return true;
        }

        virtual bool disconnectServices() override
        {
            return true;
        }

        virtual IConnectionStatusUpdateNotifier& getRamsesConnectionStatusUpdateNotifier() override
        {
            static FakeConnectionStatusUpdateNotifier fake;
            return fake;
        }

        virtual IConnectionStatusUpdateNotifier& getDcsmConnectionStatusUpdateNotifier() override
        {
            static FakeConnectionStatusUpdateNotifier fake;
            return fake;
        }

        virtual bool broadcastNewScenesAvailable(const SceneInfoVector& /*newScenes*/) override
        {
            return true;
        }

        virtual bool broadcastScenesBecameUnavailable(const SceneInfoVector& /*unavailableScenes*/) override
        {
            return true;
        }

        virtual bool sendScenesAvailable(const Guid& /*to*/, const SceneInfoVector& /*availableScenes*/) override
        {
            return true;
        }

        virtual bool sendSubscribeScene(const Guid& /*to*/, const SceneId& /*sceneId*/) override
        {
            return true;
        }

        virtual bool sendUnsubscribeScene(const Guid& /*to*/, const SceneId& /*sceneId*/) override
        {
            return true;
        }

        virtual bool sendInitializeScene(const Guid& /*to*/, const SceneId& /*sceneId*/) override
        {
            return true;
        }

        virtual bool sendSceneUpdate(const Guid& /*to*/, const SceneId& /*sceneId*/, const ISceneUpdateSerializer& /*serializer*/) override
        {
            return true;
        }

        virtual bool sendRendererEvent(const Guid& /*to*/, const SceneId& /*sceneId*/, const std::vector<Byte>& /*data*/) override
        {
            return true;
        }

        virtual bool sendDcsmBroadcastOfferContent(ContentID /*contentID*/, Category, ETechnicalContentType /*technicalContentType*/, const std::string&) override
        {
            return true;
        }

        virtual bool sendDcsmOfferContent(const Guid& /*to*/, ContentID /*contentID*/, Category, ETechnicalContentType /*technicalContentType*/, const std::string&) override
        {
            return true;
        }

        virtual bool sendDcsmContentDescription(const Guid& /*to*/, ContentID /*contentID*/, TechnicalContentDescriptor /*technicalContentDescriptor*/) override
        {
            return true;
        }

        virtual bool sendDcsmContentReady(const Guid& /*to*/, ContentID /*contentID*/) override
        {
            return true;
        }

        virtual bool sendDcsmContentEnableFocusRequest(const Guid& /*to*/, ContentID /*contentID*/, int32_t ) override
        {
            return true;
        }

        virtual bool sendDcsmContentDisableFocusRequest(const Guid& /*to*/, ContentID /*contentID*/, int32_t) override
        {
            return true;
        }

        virtual bool sendDcsmBroadcastRequestStopOfferContent(ContentID /*contentID*/) override
        {
            return true;
        }

        virtual bool sendDcsmBroadcastForceStopOfferContent(ContentID /*contentID*/) override
        {
            return true;
        }

        virtual bool sendDcsmCanvasSizeChange(const Guid& /*to*/, ContentID /*contentID*/, const CategoryInfo& /*sizeinfo*/, AnimationInformation) override
        {
            return true;
        }

        virtual bool sendDcsmContentStateChange(const Guid& /*to*/, ContentID /*contentID*/, EDcsmState /*status*/, const CategoryInfo&, AnimationInformation) override
        {
            return true;
        }

        virtual bool sendDcsmContentStatus(const Guid& /*to*/, ContentID /*contentID*/, uint64_t /*messageid*/, std::vector<Byte> const& /*message*/) override
        {
            return true;
        }

        virtual bool sendDcsmUpdateContentMetadata(const Guid& /*to*/, ContentID /*contentID*/, const DcsmMetadata& /*metadata*/) override
        {
            return true;
        }

        virtual void setSceneProviderServiceHandler(ISceneProviderServiceHandler* /*handler*/) override
        {
        }

        virtual void setSceneRendererServiceHandler(ISceneRendererServiceHandler* /*handler*/) override
        {
        }

        virtual void setDcsmProviderServiceHandler(IDcsmProviderServiceHandler* /*handler*/) override
        {
        }

        virtual void setDcsmConsumerServiceHandler(IDcsmConsumerServiceHandler* /*handler*/) override
        {
        }

        virtual void logConnectionInfo() override
        {
        }

        virtual void triggerLogMessageForPeriodicLog() override
        {
        }
    };
}

#endif
