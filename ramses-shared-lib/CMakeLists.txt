#  -------------------------------------------------------------------------
#  Copyright (C) 2018 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

ADD_DEFINITIONS(-DRAMSES_LINK_SHARED_EXPORT)

if (ramses-sdk_BUILD_FULL_SHARED_LIB)

    RENDERER_MODULE_PER_CONFIG_STATIC(ramses-shared-lib
        TYPE                    SHARED_LIBRARY
        ENABLE_INSTALL          ON

        ${ramses-shared-lib-MIXIN}
        ${ramses-shared-lib-renderer-MIXIN}
        )

    foreach(lib-suffix ${RENDERER_CONFIG_LIST})
        set(tgt "ramses-shared-lib-${lib-suffix}")
        if (TARGET ${tgt})
            set_target_properties(${tgt} PROPERTIES RAMSES_VERSION "${ramses-sdk_VERSION}")
        endif()
    endforeach()

endif()

if (ramses-sdk_BUILD_CLIENT_ONLY_SHARED_LIB)

    # build shared library without renderer
    SET(ramses-sdk_CLIENT_ONLY_LIB_NAME "ramses-shared-lib-client-only")

    ACME_MODULE(
        NAME                    ${ramses-sdk_CLIENT_ONLY_LIB_NAME}
        TYPE                    SHARED_LIBRARY
        ENABLE_INSTALL          ON

        ${ramses-shared-lib-MIXIN}
        )

    set_target_properties(${ramses-sdk_CLIENT_ONLY_LIB_NAME} PROPERTIES RAMSES_VERSION "${ramses-sdk_VERSION}")

    SET_TARGET_PROPERTIES(${ramses-sdk_CLIENT_ONLY_LIB_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})
    GET_TARGET_PROPERTY(MYMODULE_SOVERSION ${ramses-sdk_CLIENT_ONLY_LIB_NAME} SOVERSION)
    ACME_INFO("    setting shared library version to ${MYMODULE_SOVERSION}")

endif()
