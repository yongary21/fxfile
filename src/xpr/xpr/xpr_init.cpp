//
// Copyright (c) 2012 Leon Lee author. All rights reserved.
//
// Use of this source code is governed by a GPLv3 license that can be
// found in the LICENSE file.

#include "xpr_init.h"
#include "xpr_system.h"

#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

namespace xpr
{
XPR_DL_API void initialize(void)
{
    initSystemInfo();

    ::xmlInitParser();
}

XPR_DL_API void finalize(void)
{
    ::xmlCleanupParser();
}
} // namespace xpr
