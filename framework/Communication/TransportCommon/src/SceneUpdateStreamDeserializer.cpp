//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TransportCommon/SceneUpdateStreamDeserializer.h"
#include "TransportCommon/SingleSceneUpdateWriter.h"
#include "TransportCommon/SceneUpdateSerializationHelper.h"
#include "Utils/LogMacros.h"
#include "Utils/BinaryInputStream.h"

namespace ramses_internal
{
    SceneUpdateStreamDeserializer::Result SceneUpdateStreamDeserializer::processData(absl::Span<const Byte> data)
    {
        // check state + input
        if (m_hasFailed)
        {
            LOG_ERROR_P(CONTEXT_FRAMEWORK, "SceneUpdateStreamDeserializer::processData: Invalid state due to previous error");
            return fail();
        }

        if (data.size() < sizeof(uint32_t)*2 + 1)  // must at least be packet header + 'some' data
        {
            LOG_ERROR_P(CONTEXT_FRAMEWORK, "SceneUpdateStreamDeserializer::processData: Packet too small (size {})", data.size());
            return fail();
        }

        // read and verify header
        BinaryInputStream is(data.data());
        uint32_t packetNum = 0;
        uint32_t hasMorePackets = 0;
        is >> packetNum
           >> hasMorePackets;

        if (packetNum != m_nextExpectedPacketNum)
        {
            LOG_ERROR_P(CONTEXT_FRAMEWORK, "SceneUpdateStreamDeserializer::processData: expected packet {}, got {}", m_nextExpectedPacketNum, packetNum);
            return fail();
        }

        if (hasMorePackets == SingleSceneUpdateWriter::hasMorePacketsFlag)
            ++m_nextExpectedPacketNum;
        else if (hasMorePackets == SingleSceneUpdateWriter::lastPacketFlag)
            m_nextExpectedPacketNum = 1;
        else
        {
            LOG_ERROR_P(CONTEXT_FRAMEWORK, "SceneUpdateStreamDeserializer::processData: Corrupted packet, more flag is {}", hasMorePackets);
            return fail();
        }

        // read all bytes from packet
        while (is.getCurrentReadBytes() < data.size())
        {
            if (m_currentBlockSize != 0)
                continueReadingBlock(is, data.size());
            else
            {
                if (!startReadingNewBlock(is, data.size()))
                    return fail();
            }

            // check if read full block
            if (m_currentBlock.size() == m_currentBlockSize)
            {
                if (!finalizeBlock())
                    return fail();
            }
        }

        // check if done with update
        if (m_nextExpectedPacketNum == 1)
        {
            if (!m_currentBlock.empty() || m_currentBlockSize != 0)
            {
                LOG_ERROR_P(CONTEXT_FRAMEWORK, "SceneUpdateStreamDeserializer::processData: Last packet but data left to read (read {}, needed {}, type {})",
                            m_currentBlock.size(), m_currentBlockSize, m_blockType);
                return fail();
            }

            Result toReturn = std::move(m_currentResult);
            toReturn.result = ResultType::HasData;
            m_currentResult = Result{ResultType::Empty, SceneActionCollection(), {}};
            return toReturn;
        }

        return Result{ResultType::Empty, SceneActionCollection(), {}};
    }

    void SceneUpdateStreamDeserializer::continueReadingBlock(BinaryInputStream& is, size_t dataSize)
    {
        assert(m_currentBlockSize > m_currentBlock.size());

        const size_t remainingDatInPacket = dataSize - is.getCurrentReadBytes();
        const size_t remainingBytesToReadForBlock = m_currentBlockSize - m_currentBlock.size();
        const size_t readBytes = std::min(remainingDatInPacket, remainingBytesToReadForBlock);

        m_currentBlock.insert(m_currentBlock.end(), is.readPosition(), is.readPosition() + readBytes);
        is.skip(readBytes);
    }

    bool SceneUpdateStreamDeserializer::startReadingNewBlock(BinaryInputStream& is, size_t dataSize)
    {
        assert(m_currentBlock.empty());

        // block header is guaranteed continuous
        if (is.getCurrentReadBytes() + sizeof(uint32_t)*2 > dataSize)
        {
            LOG_ERROR_P(CONTEXT_FRAMEWORK, "SceneUpdateStreamDeserializer::startReadingNewBlock: Could not read header");
            return false;
        }

        is >> m_blockType
           >> m_currentBlockSize;

        return true;
    }

    bool SceneUpdateStreamDeserializer::finalizeBlock()
    {
        SingleSceneUpdateWriter::BlockType blockType = static_cast<SingleSceneUpdateWriter::BlockType>(m_blockType);

        if (blockType == SingleSceneUpdateWriter::BlockType::SceneActionCollection)
        {
            if (!handleSceneActionCollection())
                return false;
        }
        else if (blockType == SingleSceneUpdateWriter::BlockType::Resource)
        {
            if (!handleResource())
                return false;
        }
        else
        {
            // only warn on unknown block, no fail
            LOG_WARN_P(CONTEXT_FRAMEWORK, "SceneUpdateStreamDeserializer::finalizeBlock: Ignore unexpected block type {}", m_blockType);
        }

        m_currentBlock.clear();
        m_currentBlockSize = 0;
        return true;
    }

    SceneUpdateStreamDeserializer::Result SceneUpdateStreamDeserializer::fail()
    {
        m_hasFailed = true;
        return Result{ResultType::Failed, SceneActionCollection(), {}};
    }

    bool SceneUpdateStreamDeserializer::handleSceneActionCollection()
    {
        if (m_currentBlock.size() < sizeof(uint32_t)*2)
        {
            LOG_ERROR_P(CONTEXT_FRAMEWORK, "SceneUpdateStreamDeserializer::handleSceneActionCollection: Block too small ({})", m_currentBlock.size());
            return false;
        }
        if (m_currentResult.actions.numberOfActions() != 0)
        {
            LOG_ERROR_P(CONTEXT_FRAMEWORK, "SceneUpdateStreamDeserializer::handleSceneActionCollection: More than one SceneActionCollection in packet");
            return false;
        }

        BinaryInputStream is(m_currentBlock.data());
        uint32_t descSize = 0;
        uint32_t dataSize = 0;
        is >> descSize
           >> dataSize;

        m_currentResult.actions = SceneActionSerialization::Deserialize(absl::Span<const Byte>(is.readPosition(), descSize),
                                                                        absl::Span<const Byte>(is.readPosition() + descSize, dataSize));
        return true;
    }

    bool SceneUpdateStreamDeserializer::handleResource()
    {
        if (m_currentBlock.size() < sizeof(uint32_t)*2)
        {
            LOG_ERROR_P(CONTEXT_FRAMEWORK, "SceneUpdateStreamDeserializer::handleResource: Block to small ({})", m_currentBlock.size());
            return false;
        }

        BinaryInputStream is(m_currentBlock.data());
        uint32_t descSize = 0;
        uint32_t dataSize = 0;
        is >> descSize
           >> dataSize;

        m_currentResult.resources.push_back(ResourceSerialization::Deserialize(absl::Span<const Byte>(is.readPosition(), descSize),
                                                                               absl::Span<const Byte>(is.readPosition() + descSize, dataSize)));
        return true;

    }
}
