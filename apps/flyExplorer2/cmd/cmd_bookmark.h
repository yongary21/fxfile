//
// Copyright (c) 2012 Leon Lee author. All rights reserved.
//
//   homepage: http://www.flychk.com
//   e-mail:   mailto:flychk@flychk.com
//
// Use of this source code is governed by a GPLv3 license that can be
// found in the LICENSE file.

#ifndef __FX_CMD_BOOKMARK_H__
#define __FX_CMD_BOOKMARK_H__
#pragma once

#include "cmd_command.h"

namespace cmd
{
XPR_COMMAND_DECLARE_CLASS(BookmarkPopupCommand);
XPR_COMMAND_DECLARE_CLASS(GoBookmarkCommand);
XPR_COMMAND_DECLARE_CLASS(BookmarkAddCommand);
XPR_COMMAND_DECLARE_CLASS(BookmarkModifyCommand);
XPR_COMMAND_DECLARE_CLASS(BookmarkRefreshCommand);
} // namespace cmd

#endif // __FX_CMD_BOOKMARK_H__
