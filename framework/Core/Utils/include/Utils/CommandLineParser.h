//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_COMMANDLINEPARSER_H
#define RAMSES_COMMANDLINEPARSER_H

#include "Utils/CommandLineArgument.h"
#include <PlatformAbstraction/PlatformTypes.h>
#include "StringUtils.h"

namespace ramses_internal
{
    class CommandLineParser
    {
    public:
        CommandLineParser(Int argc, char const* const* argv);

        const String& getProgramName() const;
        const CommandLineArgument* getOption(const String& shortname, const String& longname, bool canHaveValue, UInt32* searchIndexInOut = nullptr) const;
        StringVector getNonOptions() const;


    private:
        using ArgumentList = std::vector<CommandLineArgument>;
        mutable ArgumentList m_args;
        String m_programName;
    };
}

#endif
