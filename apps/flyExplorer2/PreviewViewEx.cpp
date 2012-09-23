//
// Copyright (c) 2001-2012 Leon Lee author. All rights reserved.
//
//   homepage: http://www.flychk.com
//   e-mail:   mailto:flychk@flychk.com
//
// Use of this source code is governed by a GPLv3 license that can be
// found in the LICENSE file.

#include "stdafx.h"
#include "PreviewViewEx.h"

#include "resource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(PreviewViewEx, CPreviewView)

PreviewViewEx::PreviewViewEx(void)
{
}

PreviewViewEx::~PreviewViewEx(void)
{
}

BEGIN_MESSAGE_MAP(PreviewViewEx, super)
    ON_WM_CREATE()
END_MESSAGE_MAP()

xpr_sint_t PreviewViewEx::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (super::OnCreate(lpCreateStruct) == -1)
        return -1;

    m_pToolBar->EnableToolTips(XPR_TRUE);

    // Subclassing
    mPrintButton  .SubclassDlgItem(AFX_ID_PREVIEW_PRINT,   m_pToolBar);
    mPrevButton   .SubclassDlgItem(AFX_ID_PREVIEW_PREV,    m_pToolBar);
    mNextButton   .SubclassDlgItem(AFX_ID_PREVIEW_NEXT,    m_pToolBar);
    mZoomInButton .SubclassDlgItem(AFX_ID_PREVIEW_ZOOMIN,  m_pToolBar);
    mZoomOutButton.SubclassDlgItem(AFX_ID_PREVIEW_ZOOMOUT, m_pToolBar);
    mCloseButton  .SubclassDlgItem(AFX_ID_PREVIEW_CLOSE,   m_pToolBar);

    mPrintButton  .SetBitmaps(IDB_PREVIEW_PRINT_HOT,   RGB(255,0,255), IDB_PREVIEW_PRINT_COLD,   RGB(255,0,255));
    mPrevButton   .SetBitmaps(IDB_PREVIEW_PREV_HOT,    RGB(255,0,255), IDB_PREVIEW_PREV_COLD,    RGB(255,0,255));
    mNextButton   .SetBitmaps(IDB_PREVIEW_NEXT_HOT,    RGB(255,0,255), IDB_PREVIEW_NEXT_COLD,    RGB(255,0,255));
    mZoomInButton .SetBitmaps(IDB_PREVIEW_ZOOMIN_HOT,  RGB(255,0,255), IDB_PREVIEW_ZOOMIN_COLD,  RGB(255,0,255));
    mZoomOutButton.SetBitmaps(IDB_PREVIEW_ZOOMOUT_HOT, RGB(255,0,255), IDB_PREVIEW_ZOOMOUT_COLD, RGB(255,0,255));

    mPrevButton.SetAlign(mPrevButton.GetAlign() + 2);

    mPrintButton.SetWindowText(theApp.loadString(XPR_STRING_LITERAL("print_preview.button.print")));
    mPrevButton .SetWindowText(theApp.loadString(XPR_STRING_LITERAL("print_preview.button.previous_page")));
    mNextButton .SetWindowText(theApp.loadString(XPR_STRING_LITERAL("print_preview.button.next_page")));
    mCloseButton.SetWindowText(theApp.loadString(XPR_STRING_LITERAL("print_preview.button.close")));

    return 0;
}

xpr_bool_t PreviewViewEx::PreCreateWindow(CREATESTRUCT& cs)
{
    if (cs.lpszClass == XPR_NULL)
        cs.lpszClass = AfxRegisterWndClass(CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW|CS_BYTEALIGNCLIENT);
    return CView::PreCreateWindow(cs);
}

#ifdef XPR_CFG_BUILD_DEBUG
void PreviewViewEx::AssertValid() const
{
    CView::AssertValid();
}

void PreviewViewEx::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}
#endif // XPR_CFG_BUILD_DEBUG

void PreviewViewEx::Close(void)
{
    OnPreviewClose();
}
