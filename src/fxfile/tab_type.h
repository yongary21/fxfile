//
// Copyright (c) 2013 Leon Lee author. All rights reserved.
//
//   homepage: http://www.flychk.com
//   e-mail:   mailto:flychk@flychk.com
//
// Use of this source code is governed by a GPLv3 license that can be
// found in the LICENSE file.

#ifndef __FXFILE_TAB_TYPE_H__
#define __FXFILE_TAB_TYPE_H__ 1
#pragma once

namespace fxfile
{
enum TabType
{
    TabTypeNone,
    TabTypeExplorer,
    TabTypeSearchResult,
    TabTypeFileScrap,
    TabTypeFtp,
    TabTypeArchive,
    TabTypeTerminal,
    TabTypePlugin = 1000,
};
} // namespace fxfile

#endif // __FXFILE_TAB_TYPE_H__
