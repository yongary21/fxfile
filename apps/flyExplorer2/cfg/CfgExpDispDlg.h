//
// Copyright (c) 2001-2012 Leon Lee author. All rights reserved.
//
//   homepage: http://www.flychk.com
//   e-mail:   mailto:flychk@flychk.com
//
// Use of this source code is governed by a GPLv3 license that can be
// found in the LICENSE file.

#ifndef __FX_CFG_EXP_DISP_DLG_H__
#define __FX_CFG_EXP_DISP_DLG_H__
#pragma once

#include "CfgDlg.h"

class CfgExpDispDlg : public CfgDlg
{
    typedef CfgDlg super;

public:
    CfgExpDispDlg(void);

protected:
    virtual void OnApply(void);

protected:
    xpr_bool_t loadImage(xpr_sint_t aWidth, xpr_sint_t aHeight, std::tstring &aPath, xpr_bool_t &aSatistifed);

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual xpr_bool_t OnCommand(WPARAM wParam, LPARAM lParam);

protected:
    DECLARE_MESSAGE_MAP()
    virtual xpr_bool_t OnInitDialog(void);
    afx_msg void OnCustomIcon16x16Browse(void);
    afx_msg void OnCustomIcon32x32Browse(void);
};

#endif // __FX_CFG_EXP_DISP_DLG_H__
