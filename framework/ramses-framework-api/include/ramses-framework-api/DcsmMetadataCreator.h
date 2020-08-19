//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DCSMMETADATACREATOR_H
#define RAMSES_DCSMMETADATACREATOR_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-framework-api/StatusObject.h"
#include "ramses-framework-api/CarModelViewMetadata.h"
#include <string>

namespace ramses
{
    /**
     * @brief Class to create DCSM metadata object to be used in @ref DcsmProvider.
     *
     * It is not required or expected to set all metadata entries before handing the
     * DcsmMetadataCreator to DcsmProvider. Only the set entries will be transferred to
     * consumer side. It is not required to always provide all values on update but only
     * provide a delta to the last update for a given content.
     *
     * A new DcsmMetadataCreator object shall be used for each DcsmProvider call.
     */
    class RAMSES_API DcsmMetadataCreator : public StatusObject
    {
    public:
        /// Constructor
        DcsmMetadataCreator();

        /// Destructor
        ~DcsmMetadataCreator();

        /**
         * @brief Set preview image metadata entry. Data must point to a valid
         *        in memory representation of a PNG file.
         *
         * @param data pointer to memory buffer with PNG content
         * @param dataLength length in bytes of data
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t setPreviewImagePng(const void* data, size_t dataLength);

        /**
         * @brief Set preview description metadata entry. Must be a valid utf32
         *        string.
         *
         * @param previewDescription
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t setPreviewDescription(std::u32string previewDescription);

        /**
         * @brief Set widget order metadata entry. Used to determine order within a dcsm category.
         *
         * @param widgetOrder
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t setWidgetOrder(int32_t widgetOrder);

        /**
         * @brief Set widget background ID metadata entry. Requests specific background at consumer side.
         *
         * @param backgroundID identifier of requested background
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t setWidgetBackgroundID(int32_t backgroundID);

        /**
         * @brief Set widget hud line ID metadata entry. Requests specific HUD line style at consumer side.
         *
         * @param hudLineID identifier of requested hud line style
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t setWidgetHUDLineID(int32_t hudLineID);

        /**
         * @brief Set car model metadata entry. Requests car model to be put into widget at consumer side.
         *
         * @param carModel identifier of requested car model
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t setCarModel(int32_t carModel);

        /**
         * @brief Set car model view metadata entry. Requests car model view parameters at consumer side.
         *
         * @param viewMetadata view metadata for the car model
         * @param timingInfo hint to receiver when to start/finish animation to given values
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t setCarModelView(const CarModelViewMetadata& viewMetadata, const AnimationInformation& timingInfo);

        /**
         * @brief Set car model visibility metadata entry. Requests car model visibility at consumer side.
         *
         * @param visiblity visibility of car model
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t setCarModelVisibility(bool visiblity);

        /**
         * @brief Set the exclusive background metadata entry. Requests to show widget exclusively
         *        without other overlapping content in background.
         *
         * @param state true to request exclusive state, false to allow background content.
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t setExclusiveBackground(bool state);

        /**
         * @brief Set the streamID metadata entry. Requests to start/show specified stream.
         *
         * @param streamID identifier of stream to start/show
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t setStreamID(int32_t streamID);

        /**
         * @brief Deleted move constructor
         * @param other unused
         */
        DcsmMetadataCreator(DcsmMetadataCreator&& other) = delete;

        /**
         * @brief Deleted move assignment
         * @param other unused
         * @return unused
         */
        DcsmMetadataCreator& operator=(DcsmMetadataCreator&& other) = delete;

        /**
         * @brief Deleted copy constructor
         * @param other unused
         */
        DcsmMetadataCreator(const DcsmMetadataCreator& other) = delete;

        /**
         * @brief Deleted copy assignment
         * @param other unused
         * @return unused
         */
        DcsmMetadataCreator& operator=(const DcsmMetadataCreator& other) = delete;

        /**
         * Stores internal data for implementation specifics of DcsmMetadataCreator
         */
        class DcsmMetadataCreatorImpl& impl;
    };
}

#endif
