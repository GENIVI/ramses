//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCECOMPONENT_H
#define RAMSES_RESOURCECOMPONENT_H

#include "IResourceConsumerComponent.h"
#include "Collections/Guid.h"
#include "ResourceStorage.h"
#include "Components/ResourceHashUsage.h"
#include "ResourceFilesRegistry.h"

#include "TaskFramework/ITask.h"
#include "TaskFramework/EnqueueOnlyOneAtATimeQueue.h"
#include "IResourceStorageChangeListener.h"
#include "Collections/HashSet.h"
#include "TransportCommon/IConnectionStatusListener.h"
#include "IResourceProviderComponent.h"
#include "RamsesFrameworkConfigImpl.h"
#include "TransportCommon/ServiceHandlerInterfaces.h"
#include <deque>
#include <unordered_map>
#include "Utils/StatisticCollection.h"

namespace ramses_internal
{
    class IConnectionStatusUpdateNotifier;
    class ICommunicationSystem;
    class SceneUpdateStreamDeserializer;

    struct ResourceLoadInfo
    {
        ResourceLoadInfo()
            : resourceStream(nullptr)
        {}

        BinaryFileInputStream* resourceStream;
        ResourceFileEntry fileEntry;
        Guid requesterId;

        bool operator <(const ResourceLoadInfo& r) const
        {
            return fileEntry.offsetInBytes < r.fileEntry.offsetInBytes;
        }
    };

    class ResourceComponent :
        public IResourceStorageChangeListener,
        public IResourceProviderComponent,
        public IResourceConsumerComponent,
        public IResourceProviderServiceHandler,
        public IConnectionStatusListener,
        public IResourceConsumerServiceHandler
    {
    public:
        ResourceComponent(ITaskQueue& queue, const Guid& myAddress, ICommunicationSystem& communicationSystem, IConnectionStatusUpdateNotifier& connectionStatusUpdateNotifier,
            StatisticCollectionFramework& statistics, PlatformLock& frameworkLock, uint32_t maximumTotalBytesForAsynResourceLoading = ramses::MAXIMUM_BYTES_FOR_ASYNC_RESOURCE_LOADING);
        virtual ~ResourceComponent() override;

        // implement IResourceProviderComponent
        virtual ManagedResource manageResource(const IResource& resource, bool deletionAllowed = false) override;
        virtual ManagedResourceVector getResources() override;
        virtual ManagedResource getResource(ResourceContentHash hash) override;
        virtual ManagedResource forceLoadResource(const ResourceContentHash& hash) override;

        virtual ResourceHashUsage getResourceHashUsage(const ResourceContentHash& hash) override;
        virtual void addResourceFile(ResourceFileInputStreamSPtr resourceFileInputStream, const ramses_internal::ResourceTableOfContents& toc) override;
        virtual bool hasResourceFile(const String& resourceFileName) const override;
        virtual void forceLoadFromResourceFile(const String& resourceFileName) override;
        virtual void removeResourceFile(const String& resourceFileName) override;

        // implement IResourceConsumerComponent
        virtual void requestResourceAsynchronouslyFromFramework(const ResourceContentHashVector& ids, const ResourceRequesterID& requesterID, const Guid& providerID) override;
        virtual void cancelResourceRequest(const ResourceContentHash& resourceHash, const ResourceRequesterID& requesterID) override;
        virtual ManagedResourceVector popArrivedResources(const ResourceRequesterID& requesterID) override;

        // implement ResourceProviderServiceHandler
        virtual void handleRequestResources(const ResourceContentHashVector& ids, const Guid& participantTherequestCameFrom) override;

        // implement IConnectionStatusListener
        void newParticipantHasConnected(const Guid& guid) override;
        void participantHasDisconnected(const Guid& guid) override;

        // implement ResourceConsumerServiceHandler
        virtual void handleSendResource(absl::Span<const Byte> data, const Guid& providerID) override;
        virtual void handleResourcesNotAvailable(const ResourceContentHashVector& resources, const Guid& providerID) override;

        // internal / testing
        void resourceHasBeenLoadedFromFile(IResource* loadedResource, uint32_t size);
        void handleArrivedResource(const ManagedResource& resource);
        bool hasDeserializerForParticipant(const Guid& participant) const;
        bool hasRequestForResource(ResourceContentHash hash, ResourceRequesterID requester) const;

        virtual void reserveResourceCount(uint32_t totalCount) override;
        ManagedResourceVector resolveResources(ResourceContentHashVector& vec) override;

    private:
        class LoadResourcesFromFileTask : public ITask
        {
        public:
            LoadResourcesFromFileTask(ResourceComponent& component, std::vector<ResourceLoadInfo> resourceToLoadInTask, UInt64 taskCreationTime)
                : m_resourceComponent(component)
                , m_resourcesToLoad(std::move(resourceToLoadInTask))
                , m_taskCreationTime(taskCreationTime)
            {
            }
            virtual void execute() override;
        private:
            ResourceComponent& m_resourceComponent;
            std::vector<ResourceLoadInfo> m_resourcesToLoad;
            UInt64 m_taskCreationTime;
        };

        // implement IResourceStorageChangeListener
        virtual void onBytesNeededByStorageDecreased(uint64_t bytes) override;

        void triggerLoadingResourcesFromFile();

        void sendResourcesFromFile(const std::vector<IResource*>& loadedResources, uint64_t bytesLoaded, const Guid& requesterId);
        const ResourceInfo& getResourceInfo(const ResourceContentHash& hash) const;
        void storeResourceInfo(const ResourceContentHash& hash, const ResourceInfo& resourceInfo);

        using RequestsMap = std::unordered_multimap<ResourceContentHash, ResourceRequesterID>;
        static RequestsMap::const_iterator FindInRequestsRange(std::pair<RequestsMap::const_iterator, RequestsMap::const_iterator> range, ResourceRequesterID requester);

        PlatformLock& m_frameworkLock;
        IConnectionStatusUpdateNotifier& m_connectionStatusUpdateNotifier;
        ResourceStorage m_resourceStorage;
        std::deque<ResourceLoadInfo> m_resourcesToBeLoaded;
        EnqueueOnlyOneAtATimeQueue m_taskQueueForResourceLoading;
        UInt64 m_maximumBytesAllowedForResourceLoading;
        uint64_t m_bytesScheduledForLoading;
        ResourceFilesRegistry m_resourceFiles;

        ICommunicationSystem& m_communicationSystem;
        Guid m_myAddress;

        std::unordered_map<Guid, std::unique_ptr<SceneUpdateStreamDeserializer>> m_resourceDeserializers;
        RequestsMap                                                              m_requestedResources;
        std::unordered_map<ResourceRequesterID, ManagedResourceVector>           m_arrivedResources;
        std::unordered_map<ResourceRequesterID, bool>                            m_hasArrivedRemoteResources;
        std::vector<ResourceContentHash>                                         m_unrequestedResources;

        StatisticCollectionFramework& m_statistics;
    };
}

#endif
