//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESFRAMEWORK_H
#define RAMSES_RAMSESFRAMEWORK_H

#include "stdint.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "RamsesFrameworkConfig.h"
#include "ramses-framework-api/APIExport.h"
#include "ramses-framework-api/StatusObject.h"
#include <memory>

namespace ramses
{
    class DcsmProvider;
    class DcsmConsumer;
    class RamsesRenderer;
    class RamsesClient;
    class RendererConfig;
    class IRamshCommand;

    /**
    * @brief Class representing ramses framework components that are needed
    *        to initialize an instance of ramses client and renderer.
    */
    class RAMSES_API RamsesFramework : public StatusObject
    {
    public:
        /**
        * @brief Default constructor
        */
        RamsesFramework();

        /**
        * @brief Constructor of RamsesFramework using command line parameters
        *
        * @param[in] argc Number of strings in argv array
        * @param[in] argv Command line parameters as array of string
        */
        RamsesFramework(int32_t argc, char const* const* argv);

        /**
        * @brief Constructor of RamsesFramework using RamsesFrameworkConfig object
        *
        * @param[in] config Configuration object
        */
        explicit RamsesFramework(const RamsesFrameworkConfig& config);

        /**
        * @brief Tries to establish a connection to the RAMSES system.
        *
        * If only local rendering is desired this does not need to be called - but all
        * scenes must be published in #ramses::EScenePublicationMode_LocalOnly.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t connect();

        /**
        * @brief Check if the RamsesClient is connected or not
        *
        * @return true if connected, false otherwise.
        */
        bool isConnected() const;

        /**
        * @brief Disconnects the RamsesClient from the system
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t disconnect();

        /**
        * @brief Create a new RamsesClient linked to this framework. Creation of multiple clients is
        *        supported. It might be disallowed to create any client depending on internal policy.
        *        Ownership of the client will remain with the framework.
        *
        *        Must create all client objects before calling connect().
        *
        * @param applicationName a name for the ramses client application
        *
        * @return The new RamsesClient object or nullptr if the creation failed or was denied.
        */
        RamsesClient* createClient(const char* applicationName);

        /**
        * @brief Destroy a RamsesClient created with this framework.
        *        This method will fail when handed an object created with another RamsesFramework.
        *
        *        May not be called when connected.
        *
        * @param client the object to destroy
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t destroyClient(RamsesClient& client);

        /**
        * @brief Create a new RamsesRenderer linked to this framework.
        *        Only one RamsesRenderer can exist per RamsesFramework instance. It might be disallowed
        *        to create any renderer depending on internal policy. Ownership of the renderer will
        *        remain with the framework.
        *
        *        Must create the renderer object before calling #connect().
        *
        * @param config  Set of configuration flags and attributes for the ramses renderer
        *
        * @return The new RamsesRenderer object or nullptr if the creation failed or was denied.
        */
        RamsesRenderer* createRenderer(const RendererConfig& config);

        /**
        * @brief Destroy a RamsesRenderer created with this framework.
        *        This method will fail when handed an object created with another RamsesFramework.
        *
        *        May not be called when connected.
        *
        * @param renderer the object to destroy
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t destroyRenderer(RamsesRenderer& renderer);

        /**
        * @brief Create a new DcsmProvider linked to this framework.
        *        Only one DcsmProvider can exist per RamsesFramework instance. Depending on user
        *        the creation is not allowed and will always fail.
        *
        *        Must call #connect() before communication with remote Dcsm consumers is possible.
        *
        * @return The new DcsmProvider object or nullptr if the creation failed or was denied.
        */
        DcsmProvider* createDcsmProvider();

        /**
        * @brief Destroy a DcsmProvider created with this framework.
        *        This method will fail when handed an object created with another RamsesFramework.
        *        Ownership of the provider will remain with the framework.
        *
        * @param provider the object to destroy
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t destroyDcsmProvider(const DcsmProvider& provider);

        /**
        * @brief Create a new DcsmConsumer linked to this framework.
        *        Only one DcsmConsumer can exist per RamsesFramework instance. It might be disallowed
        *        to create any consumer depending on internal policy.
        *        Ownership of the consumer will remain with the framework.
        *
        *        Must call #connect() before communication with remote Dcsm providers is possible.
        *
        * @return The new DcsmConsumer object or nullptr if the creation failed or was denied.
        */
        DcsmConsumer* createDcsmConsumer();

        /**
        * @brief Destroy a DcsmConsumer created with this framework.
        *        This method will fail when handed an object created with another RamsesFramework.
        *
        * @param consumer the object to destroy
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t destroyDcsmConsumer(const DcsmConsumer& consumer);

        /**
        * @brief Set the log level for all console log messages and apply it immediately.
        *
        * If this function is called before creation of RamsesFramework it will be overwritten
        * by cmdl args ("log-level", "log-level-console"), environment variable ("RAMSES_LOGLEVEL") and
        * by dynamically injected value by ramsh or dlt viewer/logger.
        * If this function is called after creation of RamsesFramework it can only be overwritten
        * dynamically by ramsh or dlt viewer/logger.
        *
        * @param[in] logLevel the log level to be applied
        */
        static void SetConsoleLogLevel(ELogLevel logLevel);

        /**
        * @brief Register a ramsh command that can be invoked via console and DLT injection
        *
        * This is for testing and debugging purpose only. Command injection is not guaranteed to work in production.
        *
        * The command has to be provided via shared_ptr to avoid lifetime issues. Internally ramses will only store
        * a std::weak_ptr to the command. Therefore it is valid to let go of the shared_ptr on caller side and expect
        * that no calls will happen in the command anymore. This allows to have user obejct references in the command
        * implementation with a shorter lifetime than RamsesFramework.
        *
        * It is not possible to delete commands. They are expected to be long-living and are bound to the lifetime
        * of the RamsesFramework object.
        *
        * @param[in] command the ramsh command
        * @return StatusOK on success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t addRamshCommand(const std::shared_ptr<IRamshCommand>& command);

        /**
        * @brief Destructor of RamsesFramework
        */
        ~RamsesFramework() override;

        /**
        * Stores internal data for implementation specifics of RamsesFramework
        */
        class RamsesFrameworkImpl& impl;

        /**
         * @brief Deleted copy constructor
         * @param other unused
         */
        RamsesFramework(const RamsesFramework& other) = delete;

        /**
         * @brief Deleted copy assignment
         * @param other unused
         * @return unused
         */
        RamsesFramework& operator=(const RamsesFramework& other) = delete;
    };
}

#endif
