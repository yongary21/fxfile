//
// Copyright (c) 2001-2012 Leon Lee author. All rights reserved.
//
//   homepage: http://www.flychk.com
//   e-mail:   mailto:flychk@flychk.com
//
// Use of this source code is governed by a GPLv3 license that can be
// found in the LICENSE file.

#ifndef __FX_REN_TAB_DLG_3_H__
#define __FX_REN_TAB_DLG_3_H__
#pragma once

#include "RenTabDlg.h"

class DlgState;

class RenTabDlg3 : public RenTabDlg
{
    typedef RenTabDlg super;

public:
    RenTabDlg3(void);

public:
    void enableWindowDelete(void);

protected:
    DlgState *mDlgState;

public:
    virtual xpr_bool_t PreTranslateMessage(MSG* pMsg);

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

protected:
    DECLARE_MESSAGE_MAP()
    virtual xpr_bool_t OnInitDialog(void);
    afx_msg void OnApply(void);
    afx_msg void OnSelchangeDeleteSelection(void);
    afx_msg void OnDestroy(void);
};

#endif // __FX_REN_TAB_DLG_3_H__