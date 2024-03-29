//
// Copyright (c) 2012-2013 Leon Lee author. All rights reserved.
//
//   homepage: http://www.flychk.com
//   e-mail:   mailto:flychk@flychk.com
//
// Use of this source code is governed by a GPLv3 license that can be
// found in the LICENSE file.

#include "stdafx.h"
#include "cmd_file_oper.h"

#include "context_menu.h"
#include "file_op_thread.h"
#include "option.h"
#include "folder_ctrl.h"
#include "explorer_ctrl.h"
#include "search_result_ctrl.h"

#include "router/cmd_parameters.h"
#include "router/cmd_parameter_define.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace fxfile
{
namespace cmd
{
xpr_sint_t FileDeleteCommand::canExecute(CommandContext &aContext)
{
    FXFILE_COMMAND_DECLARE_CTRL;

    xpr_bool_t sEnable = XPR_FALSE;

    FXFILE_COMMAND_IF_FOLDER_CTRL
    {
        sEnable = sFolderCtrl->hasSelShellAttributes(SFGAO_CANDELETE);
    }
    FXFILE_COMMAND_ELSE_IF_SEARCH_RESULT_CTRL
    {
        sEnable = (sSearchResultCtrl->GetSelectedCount() > 0) ? XPR_TRUE : XPR_FALSE;
    }
    FXFILE_COMMAND_ELSE_IF_EXPLORER_CTRL
    {
        sEnable = sExplorerCtrl->hasSelShellAttributes(SFGAO_CANDELETE);
    }

    return (sEnable == XPR_TRUE) ? StateEnable : StateDisable;
}

void FileDeleteCommand::execute(CommandContext &aContext)
{
    FXFILE_COMMAND_DECLARE_CTRL;

    FXFILE_COMMAND_IF_FOLDER_CTRL
    {
        HTREEITEM sTreeItem = XPR_NULL;
        if (sParameters != XPR_NULL)
            sTreeItem = (HTREEITEM)sParameters->get(CommandParameterIdTreeSelectedItem);

        if (sTreeItem == XPR_NULL)
            sTreeItem = sFolderCtrl->GetSelectedItem();

        LPTVITEMDATA sTvItemData = (LPTVITEMDATA)sFolderCtrl->GetItemData(sTreeItem);
        if (XPR_IS_NOT_NULL(sTvItemData) && XPR_TEST_BITS(sTvItemData->mShellAttributes, SFGAO_CANDELETE))
        {
            if (gOpt->mConfig.mExternalDeleteFileOp == XPR_TRUE)
                ContextMenu::invokeCommand(sTvItemData->mShellFolder, (LPCITEMIDLIST *)&sTvItemData->mPidl, 1, CMID_VERB_DELETE, sFolderCtrl->GetSafeHwnd());
            else
            {
                xpr_tchar_t *sSource = new xpr_tchar_t[XPR_MAX_PATH + 1];
                GetName(sTvItemData->mShellFolder, sTvItemData->mPidl, SHGDN_FORPARSING, sSource);
                sSource[_tcslen(sSource)+1] = XPR_STRING_LITERAL('\0');

                WORD sFlags = 0;
                if (GetAsyncKeyState(VK_SHIFT) >= 0)
                    sFlags |= FOF_ALLOWUNDO;

                SHFILEOPSTRUCT *sShFileOpStruct = new SHFILEOPSTRUCT;
                memset(sShFileOpStruct, 0, sizeof(SHFILEOPSTRUCT));
                sShFileOpStruct->hwnd   = AfxGetMainWnd()->GetSafeHwnd();
                sShFileOpStruct->wFunc  = FO_DELETE;
                sShFileOpStruct->fFlags = sFlags;
                sShFileOpStruct->pFrom  = sSource;
                sShFileOpStruct->pTo    = XPR_NULL;

                FileOpThread *sFileOpThread = new FileOpThread;
                sFileOpThread->start(sShFileOpStruct);

                sFolderCtrl->SetFocus();
            }
        }
    }
    FXFILE_COMMAND_ELSE_IF_SEARCH_RESULT_CTRL
    {
        xpr_sint_t sSelCount = sSearchResultCtrl->GetSelectedCount();
        if (sSelCount > 0)
        {
            xpr_sint_t sCount = 0;
            xpr_tchar_t *sSource = XPR_NULL;

            POSITION sPosition = sSearchResultCtrl->GetFirstSelectedItemPosition();
            if (XPR_IS_NOT_NULL(sPosition))
            {
                sSource = new xpr_tchar_t[(XPR_MAX_PATH + 1) * sSelCount + 1];

                xpr_sint_t sIndex;
                xpr_tchar_t sPath[XPR_MAX_PATH + 1] = {0};
                SrItemData *sSrItemData = XPR_NULL;
                xpr_tchar_t *sSourceEnum = sSource;

                while (XPR_IS_NOT_NULL(sPosition))
                {
                    sIndex = sSearchResultCtrl->GetNextSelectedItem(sPosition);

                    sSrItemData = (SrItemData *)sSearchResultCtrl->GetItemData(sIndex);
                    if (XPR_IS_NOT_NULL(sSrItemData))
                    {
                        sSrItemData->getPath(sPath);

                        _tcscpy(sSourceEnum, sPath);
                        sSourceEnum += _tcslen(sPath) + 1;
                        sCount++;
                    }
                }

                sSourceEnum[0] = '\0';
            }

            if (sCount <= 0)
            {
                XPR_SAFE_DELETE_ARRAY(sSource);
                return;
            }

            WORD sFlags = 0;

            // windows vista bug
            //if (sCount > 1)
            //    sFlags |= FOF_MULTIDESTFILES;

            if (GetAsyncKeyState(VK_SHIFT) >= 0)
                sFlags |= FOF_ALLOWUNDO;

            SHFILEOPSTRUCT *sShFileOpStruct = new SHFILEOPSTRUCT;
            memset(sShFileOpStruct, 0, sizeof(SHFILEOPSTRUCT));
            sShFileOpStruct->hwnd   = AfxGetMainWnd()->GetSafeHwnd();
            sShFileOpStruct->wFunc  = FO_DELETE;
            sShFileOpStruct->fFlags = sFlags;
            sShFileOpStruct->pFrom  = sSource;
            sShFileOpStruct->pTo    = XPR_NULL;

            FileOpThread *sFileOpThread = new FileOpThread;
            sFileOpThread->start(sShFileOpStruct);
        }
    }
    FXFILE_COMMAND_ELSE_IF_EXPLORER_CTRL
    {
        xpr_sint_t sSelCount = sExplorerCtrl->GetSelectedCount();
        if (sSelCount <= 0)
            return;

        // confirm trash
        xpr_bool_t sTrash = XPR_FALSE;
        LPSHELLFOLDER sShellFolder = XPR_NULL;
        HRESULT sHResult = ::SHGetDesktopFolder(&sShellFolder);
        if (SUCCEEDED(sHResult))
        {
            LPTVITEMDATA sTvItemData = sExplorerCtrl->getFolderData();
            sTrash = fxfile::base::Pidl::compare(sTvItemData->mFullPidl, CSIDL_BITBUCKET) == 0;
        }
        COM_RELEASE(sShellFolder);

        if (XPR_IS_TRUE(sTrash) || gOpt->mConfig.mExternalDeleteFileOp == XPR_TRUE)
        {
            xpr_sint_t sCount = 0;
            LPSHELLFOLDER sShellFolder = XPR_NULL;
            LPCITEMIDLIST *sPidls = XPR_NULL;

            sExplorerCtrl->getSelItemData(&sShellFolder, &sPidls, sCount);

            if (sCount > 0 && XPR_IS_NOT_NULL(sShellFolder))
                ContextMenu::invokeCommand(sShellFolder, sPidls, sCount, CMID_VERB_DELETE, sExplorerCtrl->GetSafeHwnd());

            XPR_SAFE_DELETE_ARRAY(sPidls);
        }
        else
        {
            xpr_sint_t sCount = 0;
            xpr_tchar_t *sSource = XPR_NULL;

            POSITION sPosition = sExplorerCtrl->GetFirstSelectedItemPosition();
            if (XPR_IS_NOT_NULL(sPosition))
            {
                sSource = new xpr_tchar_t[(XPR_MAX_PATH + 1) * sSelCount + 1];

                xpr_sint_t sIndex;
                xpr_tchar_t sPath[XPR_MAX_PATH + 1];
                xpr_tchar_t *sSourceEnum = sSource;
                LPLVITEMDATA sLvItemData = XPR_NULL;

                while (XPR_IS_NOT_NULL(sPosition))
                {
                    sIndex = sExplorerCtrl->GetNextSelectedItem(sPosition);

                    sLvItemData = (LPLVITEMDATA)sExplorerCtrl->GetItemData(sIndex);
                    if (XPR_IS_NOT_NULL(sLvItemData) && sLvItemData->mShellAttributes & SFGAO_FILESYSTEM)
                    {
                        GetName(sLvItemData->mShellFolder, sLvItemData->mPidl, SHGDN_FORPARSING, sPath);

                        _tcscpy(sSourceEnum, sPath);
                        sSourceEnum += _tcslen(sPath) + 1;
                        sCount++;
                    }
                }
                sSourceEnum[0] = XPR_STRING_LITERAL('\0');
            }

            if (sCount <= 0)
            {
                XPR_SAFE_DELETE_ARRAY(sSource);
                return;
            }

            WORD sFlags = 0;

            // windows vista bug
            //if (sCount > 1)
            //    sFlags |= FOF_MULTIDESTFILES;

            if (GetAsyncKeyState(VK_SHIFT) >= 0)
                sFlags |= FOF_ALLOWUNDO;

            SHFILEOPSTRUCT *sShFileOpStruct = new SHFILEOPSTRUCT;
            memset(sShFileOpStruct, 0, sizeof(SHFILEOPSTRUCT));
            sShFileOpStruct->hwnd   = AfxGetMainWnd()->GetSafeHwnd();
            sShFileOpStruct->wFunc  = FO_DELETE;
            sShFileOpStruct->fFlags = sFlags;
            sShFileOpStruct->pFrom  = sSource;
            sShFileOpStruct->pTo    = XPR_NULL;

            FileOpThread *sFileOpThread = new FileOpThread;
            sFileOpThread->start(sShFileOpStruct);
        }
    }
}

xpr_sint_t FileDuplicateCommand::canExecute(CommandContext &aContext)
{
    FXFILE_COMMAND_DECLARE_CTRL;

    xpr_bool_t sEnable = XPR_FALSE;

    FXFILE_COMMAND_IF_FOLDER_CTRL
    {
        sEnable = sFolderCtrl->hasSelShellAttributes(SFGAO_CANCOPY);
    }
    FXFILE_COMMAND_ELSE_IF_EXPLORER_CTRL
    {
        sEnable = sExplorerCtrl->hasSelShellAttributes(SFGAO_CANCOPY);
    }

    return (sEnable == XPR_TRUE) ? StateEnable : StateDisable;
}

void FileDuplicateCommand::execute(CommandContext &aContext)
{
    FXFILE_COMMAND_DECLARE_CTRL;

    FXFILE_COMMAND_IF_FOLDER_CTRL
    {
        HTREEITEM sTreeItem = sFolderCtrl->GetSelectedItem();
        HTREEITEM sParentTreeItem = sFolderCtrl->GetParentItem(sTreeItem);
        LPTVITEMDATA sTvItemData  = (LPTVITEMDATA)sFolderCtrl->GetItemData(sTreeItem);
        LPTVITEMDATA sTvItemData2 = (LPTVITEMDATA)sFolderCtrl->GetItemData(sParentTreeItem);
        if (XPR_IS_NULL(sTvItemData) || XPR_IS_NULL(sTvItemData2))
            return;

        if ((sTvItemData->mShellAttributes & SFGAO_CANCOPY) != SFGAO_CANCOPY)
            return;

        if ((sTvItemData2->mShellAttributes & SFGAO_FILESYSTEM) != SFGAO_FILESYSTEM)
            return;

        xpr_tchar_t *sSource = new xpr_tchar_t[XPR_MAX_PATH + 1];
        xpr_tchar_t *sTarget = new xpr_tchar_t[XPR_MAX_PATH + 1];
        GetName(sTvItemData->mShellFolder,  sTvItemData->mPidl,  SHGDN_FORPARSING, sSource);
        GetName(sTvItemData2->mShellFolder, sTvItemData2->mPidl, SHGDN_FORPARSING, sTarget);

        sSource[_tcslen(sSource)+1] = XPR_STRING_LITERAL('\0');
        sTarget[_tcslen(sTarget)+1] = XPR_STRING_LITERAL('\0');

        SHFILEOPSTRUCT *sShFileOpStruct = new SHFILEOPSTRUCT;
        memset(sShFileOpStruct, 0, sizeof(SHFILEOPSTRUCT));
        sShFileOpStruct->hwnd   = AfxGetMainWnd()->GetSafeHwnd();
        sShFileOpStruct->wFunc  = FO_COPY;
        sShFileOpStruct->fFlags = FOF_RENAMEONCOLLISION;
        sShFileOpStruct->pFrom  = sSource;
        sShFileOpStruct->pTo    = sTarget;

        FileOpThread *sFileOpThread = new FileOpThread;
        sFileOpThread->start(sShFileOpStruct);
    }
    FXFILE_COMMAND_ELSE_IF_EXPLORER_CTRL
    {
        xpr_sint_t sSelCount = sExplorerCtrl->GetSelectedCount();
        if (sSelCount <= 0)
            return;

        POSITION sPosition = sExplorerCtrl->GetFirstSelectedItemPosition();
        if (XPR_IS_NULL(sPosition))
            return;

        xpr_tchar_t *sSource = new xpr_tchar_t[(XPR_MAX_PATH + 1) * sSelCount + 1];

        xpr_sint_t sIndex;
        xpr_sint_t sCount = 0;
        xpr_tchar_t sPath[XPR_MAX_PATH + 1];
        xpr_tchar_t *sSourceEnum = sSource;
        LPLVITEMDATA sLvItemData = XPR_NULL;

        while (XPR_IS_NOT_NULL(sPosition))
        {
            sIndex = sExplorerCtrl->GetNextSelectedItem(sPosition);

            sLvItemData = (LPLVITEMDATA)sExplorerCtrl->GetItemData(sIndex);
            if (XPR_IS_NOT_NULL(sLvItemData) && sLvItemData->mShellAttributes & SFGAO_FILESYSTEM)
            {
                GetName(sLvItemData->mShellFolder, sLvItemData->mPidl, SHGDN_FORPARSING, sPath);

                _tcscpy(sSourceEnum, sPath);
                sSourceEnum += _tcslen(sPath) + 1;
                sCount++;
            }
        }
        sSourceEnum[0] = XPR_STRING_LITERAL('\0');

        if (sCount <= 0)
        {
            XPR_SAFE_DELETE_ARRAY(sSource);
            return;
        }

        xpr_tchar_t *sTarget = new xpr_tchar_t[XPR_MAX_PATH + 1];
        _tcscpy(sTarget, sExplorerCtrl->getCurPath());
        sTarget[_tcslen(sTarget)+1] = XPR_STRING_LITERAL('\0');

        WORD sFlags = FOF_RENAMEONCOLLISION;
        xpr_uint_t sPasteSelItemMessage = sExplorerCtrl->getPasteSelItemMessage();

        // windows vista bug
        //if (sCount > 1)
        //    sFlags |= FOF_MULTIDESTFILES;

        SHFILEOPSTRUCT *sShFileOpStruct = new SHFILEOPSTRUCT;
        memset(sShFileOpStruct, 0, sizeof(SHFILEOPSTRUCT));
        sShFileOpStruct->hwnd   = AfxGetMainWnd()->GetSafeHwnd();
        sShFileOpStruct->wFunc  = FO_COPY;
        sShFileOpStruct->fFlags = sFlags;
        sShFileOpStruct->pFrom  = sSource;
        sShFileOpStruct->pTo    = sTarget;

        FileOpThread *sFileOpThread = new FileOpThread;
        sFileOpThread->start(sShFileOpStruct, XPR_TRUE, sExplorerCtrl->m_hWnd, sPasteSelItemMessage);
    }
}

xpr_sint_t FileLinkCommand::canExecute(CommandContext &aContext)
{
    FXFILE_COMMAND_DECLARE_CTRL;

    xpr_bool_t sEnable = XPR_FALSE;

    FXFILE_COMMAND_IF_FOLDER_CTRL
    {
        HTREEITEM sTreeItem = sFolderCtrl->GetSelectedItem();
        if (sTreeItem != XPR_NULL)
        {
            LPTVITEMDATA sTvItemData = (LPTVITEMDATA)sFolderCtrl->GetItemData(sTreeItem);
            if (sTvItemData != XPR_NULL)
                sEnable = XPR_TEST_BITS(sTvItemData->mShellAttributes, SFGAO_CANLINK);
        }
    }
    FXFILE_COMMAND_ELSE_IF_EXPLORER_CTRL
    {
        sEnable = (sExplorerCtrl->getRealSelCount() > 0) ? XPR_TRUE : XPR_FALSE;
    }

    return (sEnable == XPR_TRUE) ? StateEnable : StateDisable;
}

void FileLinkCommand::execute(CommandContext &aContext)
{
    FXFILE_COMMAND_DECLARE_CTRL;

    FXFILE_COMMAND_IF_FOLDER_CTRL
    {
        HTREEITEM sTreeItem = sFolderCtrl->GetSelectedItem();

        LPTVITEMDATA sTvItemData = (LPTVITEMDATA)sFolderCtrl->GetItemData(sTreeItem);
        if (XPR_IS_NOT_NULL(sTvItemData))
            ContextMenu::invokeCommand(sTvItemData->mShellFolder, (LPCITEMIDLIST *)&(sTvItemData->mPidl), 1, CMID_VERB_LINK, sFolderCtrl->GetSafeHwnd());
    }
    FXFILE_COMMAND_ELSE_IF_EXPLORER_CTRL
    {
        xpr_sint_t sSelCount = sExplorerCtrl->GetSelectedCount();
        if (sSelCount > 0)
        {
            xpr_sint_t     sCount = 0;
            LPSHELLFOLDER  sShellFolder = XPR_NULL;
            LPCITEMIDLIST *sPidls = XPR_NULL;

            sExplorerCtrl->getSelItemData(&sShellFolder, &sPidls, sCount);

            if (sCount > 0 && XPR_IS_NOT_NULL(sShellFolder))
                ContextMenu::invokeCommand(sShellFolder, sPidls, sCount, CMID_VERB_LINK, sExplorerCtrl->GetSafeHwnd());

            XPR_SAFE_DELETE_ARRAY(sPidls);
        }
    }
}

static xpr_sint_t CALLBACK BrowseCallbackProc(HWND aHwnd, xpr_uint_t aMsg, LPARAM lParam, LPARAM aData)
{
    if (aMsg == BFFM_INITIALIZED)
        ::SendMessage(aHwnd, BFFM_SETSELECTION, XPR_FALSE, aData);

    return 0;
}

xpr_sint_t FolderCopyCommand::canExecute(CommandContext &aContext)
{
    FXFILE_COMMAND_DECLARE_CTRL;

    xpr_bool_t sEnable = XPR_FALSE;

    FXFILE_COMMAND_IF_FOLDER_CTRL
    {
        HTREEITEM sTreeItem = sFolderCtrl->GetSelectedItem();
        if (sTreeItem != XPR_NULL)
        {
            LPTVITEMDATA sTvItemData = (LPTVITEMDATA)sFolderCtrl->GetItemData(sTreeItem);
            if (sTvItemData != XPR_NULL)
                sEnable = XPR_TEST_BITS(sTvItemData->mShellAttributes, SFGAO_CANCOPY);
        }
    }
    FXFILE_COMMAND_ELSE_IF_SEARCH_RESULT_CTRL
    {
        sEnable = (sSearchResultCtrl->GetSelectedCount() > 0) ? XPR_TRUE : XPR_FALSE;
    }
    FXFILE_COMMAND_ELSE_IF_EXPLORER_CTRL
    {
        sEnable = (sExplorerCtrl->getRealSelCount() > 0) ? XPR_TRUE : XPR_FALSE;
    }

    return (sEnable == XPR_TRUE) ? StateEnable : StateDisable;
}

void FolderCopyCommand::execute(CommandContext &aContext)
{
    FXFILE_COMMAND_DECLARE_CTRL;

    FXFILE_COMMAND_IF_FOLDER_CTRL
    {
        if (sFolderCtrl->hasSelShellAttributes(SFGAO_FILESYSTEM | SFGAO_CANCOPY) == XPR_FALSE)
            return;

        BROWSEINFO sBrowseInfo = {0};
        sBrowseInfo.hwndOwner = sFolderCtrl->GetSafeHwnd();
        sBrowseInfo.lpszTitle = gApp.loadString(XPR_STRING_LITERAL("popup.copy_to_folder.folder_browse.title"));
        sBrowseInfo.ulFlags   = BIF_RETURNONLYFSDIRS;// | BIF_USENEWUI;

        LPITEMIDLIST sFullPidl = ::SHBrowseForFolder(&sBrowseInfo);
        if (XPR_IS_NULL(sFullPidl))
            return;

        xpr_sint_t sCount = 0;
        xpr_tchar_t *sSource = XPR_NULL;
        if (sFolderCtrl->getSelItemPath(&sSource, sCount, SFGAO_CANCOPY) == XPR_TRUE)
        {
            xpr_tchar_t *sTarget = new xpr_tchar_t[XPR_MAX_PATH + 1];
            GetName(sFullPidl, SHGDN_FORPARSING, sTarget);
            sTarget[_tcslen(sTarget) + 1] = XPR_STRING_LITERAL('\0'); // double null end

            FileOpThread *sFileOpThread = new FileOpThread;
            sFileOpThread->start(XPR_TRUE, sSource, sCount, sTarget);
        }

        COM_FREE(sFullPidl);
    }
    FXFILE_COMMAND_ELSE_IF_SEARCH_RESULT_CTRL
    {
        xpr_sint_t sCount = sSearchResultCtrl->GetSelectedCount();
        if (sCount <= 0)
            return;

        xpr_tchar_t *sSource = new xpr_tchar_t[(XPR_MAX_PATH + 1) * sCount + 1];
        xpr_sint_t sRCount = 0, sPos = 0, sIndex;
        xpr_tchar_t sPath[XPR_MAX_PATH + 1];
        SrItemData *sSrItemData = XPR_NULL;

        POSITION sPosition = sSearchResultCtrl->GetFirstSelectedItemPosition();
        while (XPR_IS_NOT_NULL(sPosition))
        {
            sIndex = sSearchResultCtrl->GetNextSelectedItem(sPosition);

            sSrItemData = (SrItemData *)sSearchResultCtrl->GetItemData(sIndex);
            if (XPR_IS_NOT_NULL(sSrItemData))
            {
                sSrItemData->getPath(sPath);

                _tcscpy(sSource+sPos, sPath);
                sPos += (xpr_sint_t)_tcslen(sPath) + 1;
                sRCount++;
            }
        }
        sSource[sPos] = '\0';

        BROWSEINFO sBrowseInfo = {0};
        sBrowseInfo.hwndOwner = sSearchResultCtrl->GetSafeHwnd();
        sBrowseInfo.lpszTitle = gApp.loadString(XPR_STRING_LITERAL("popup.copy_to_folder.folder_browse.title"));
        sBrowseInfo.ulFlags   = BIF_RETURNONLYFSDIRS;// | BIF_USENEWUI;
        LPITEMIDLIST sFullPidl = ::SHBrowseForFolder(&sBrowseInfo);

        if (XPR_IS_NOT_NULL(sFullPidl))
        {
            xpr_tchar_t sTarget[XPR_MAX_PATH + 1];
            GetName(sFullPidl, SHGDN_FORPARSING, sTarget);
            sTarget[_tcslen(sTarget)] = '\0';

            SHFILEOPSTRUCT sShFileOpStruct = {0};
            sShFileOpStruct.hwnd   = sSearchResultCtrl->GetSafeHwnd();
            sShFileOpStruct.wFunc  = FO_COPY;
            sShFileOpStruct.fFlags = 0;//(sRCount > 1) ? FOF_MULTIDESTFILES : 0;
            sShFileOpStruct.pFrom  = sSource;
            sShFileOpStruct.pTo    = sTarget;
            ::SHFileOperation(&sShFileOpStruct);
        }

        COM_FREE(sFullPidl);
        XPR_SAFE_DELETE_ARRAY(sSource);
    }
    FXFILE_COMMAND_ELSE_IF_EXPLORER_CTRL
    {
        xpr_sint_t sCount = sExplorerCtrl->GetSelectedCount();
        if (sCount <= 0)
            return;

        LPITEMIDLIST sOldPidl = sExplorerCtrl->getCopyToPidl();

        BROWSEINFO sBrowseInfo = {0};
        sBrowseInfo.hwndOwner = sExplorerCtrl->GetSafeHwnd();
        sBrowseInfo.lpszTitle = gApp.loadString(XPR_STRING_LITERAL("popup.copy_to_folder.folder_browse.title"));
        sBrowseInfo.ulFlags   = BIF_RETURNONLYFSDIRS;// | BIF_USENEWUI;
        sBrowseInfo.lpfn      = (BFFCALLBACK)BrowseCallbackProc;
        sBrowseInfo.lParam    = (LPARAM)sOldPidl;

        LPITEMIDLIST sFullPidl = ::SHBrowseForFolder(&sBrowseInfo);
        if (XPR_IS_NOT_NULL(sFullPidl))
        {
            xpr_sint_t sCount = 0;
            xpr_tchar_t *sSource = XPR_NULL;
            if (sExplorerCtrl->getSelItemPath(&sSource, sCount, SFGAO_CANCOPY) == XPR_TRUE)
            {
                xpr_tchar_t *sTarget = new xpr_tchar_t[XPR_MAX_PATH + 1];
                GetName(sFullPidl, SHGDN_FORPARSING, sTarget);
                sTarget[_tcslen(sTarget) + 1] = XPR_STRING_LITERAL('\0');

                FileOpThread *sFileOpThread = new FileOpThread;
                sFileOpThread->start(XPR_TRUE, sSource, sCount, sTarget);
            }

            sExplorerCtrl->setCopyToPidl(sFullPidl);
        }
    }
}

xpr_sint_t FolderMoveCommand::canExecute(CommandContext &aContext)
{
    FXFILE_COMMAND_DECLARE_CTRL;

    xpr_bool_t sEnable = XPR_FALSE;

    FXFILE_COMMAND_IF_FOLDER_CTRL
    {
        HTREEITEM sTreeItem = sFolderCtrl->GetSelectedItem();
        if (sTreeItem != XPR_NULL)
        {
            LPTVITEMDATA sTvItemData = (LPTVITEMDATA)sFolderCtrl->GetItemData(sTreeItem);
            if (sTvItemData != XPR_NULL)
                sEnable = XPR_TEST_BITS(sTvItemData->mShellAttributes, SFGAO_CANMOVE);
        }
    }
    FXFILE_COMMAND_ELSE_IF_SEARCH_RESULT_CTRL
    {
        sEnable = (sSearchResultCtrl->GetSelectedCount() > 0) ? XPR_TRUE : XPR_FALSE;
    }
    FXFILE_COMMAND_ELSE_IF_EXPLORER_CTRL
    {
        sEnable = (sExplorerCtrl->getRealSelCount() > 0) ? XPR_TRUE : XPR_FALSE;
    }

    return (sEnable == XPR_TRUE) ? StateEnable : StateDisable;
}

void FolderMoveCommand::execute(CommandContext &aContext)
{
    FXFILE_COMMAND_DECLARE_CTRL;

    FXFILE_COMMAND_IF_FOLDER_CTRL
    {
        if (sFolderCtrl->hasSelShellAttributes(SFGAO_FILESYSTEM | SFGAO_CANMOVE) == XPR_FALSE)
            return;

        BROWSEINFO sBrowseInfo = {0};
        sBrowseInfo.hwndOwner = sFolderCtrl->GetSafeHwnd();
        sBrowseInfo.lpszTitle = gApp.loadString(XPR_STRING_LITERAL("popup.move_to_folder.folder_browse.title"));
        sBrowseInfo.ulFlags   = BIF_RETURNONLYFSDIRS;// | BIF_USENEWUI;

        LPITEMIDLIST sFullPidl = ::SHBrowseForFolder(&sBrowseInfo);
        if (XPR_IS_NULL(sFullPidl))
            return;

        xpr_sint_t sCount = 0;
        xpr_tchar_t *sSource = XPR_NULL;
        if (sFolderCtrl->getSelItemPath(&sSource, sCount, SFGAO_CANMOVE) == XPR_TRUE)
        {
            xpr_tchar_t *sTarget = new xpr_tchar_t[XPR_MAX_PATH + 1];
            GetName(sFullPidl, SHGDN_FORPARSING, sTarget);
            sTarget[_tcslen(sTarget)+1] = XPR_STRING_LITERAL('\0');

            FileOpThread *sFileOpThread = new FileOpThread;
            sFileOpThread->start(XPR_FALSE, sSource, sCount, sTarget);
        }

        COM_FREE(sFullPidl);
    }
    FXFILE_COMMAND_ELSE_IF_SEARCH_RESULT_CTRL
    {
        xpr_sint_t sCount = sSearchResultCtrl->GetSelectedCount();
        if (sCount <= 0)
            return;

        xpr_tchar_t *sSource = new xpr_tchar_t[(XPR_MAX_PATH + 1) * sCount + 1];
        xpr_sint_t sRCount = 0, sPos = 0, sIndex;
        xpr_tchar_t sPath[XPR_MAX_PATH + 1];
        SrItemData *sSrItemData = XPR_NULL;

        POSITION sPosition = sSearchResultCtrl->GetFirstSelectedItemPosition();
        while (XPR_IS_NOT_NULL(sPosition))
        {
            sIndex = sSearchResultCtrl->GetNextSelectedItem(sPosition);

            sSrItemData = (SrItemData *)sSearchResultCtrl->GetItemData(sIndex);
            if (XPR_IS_NOT_NULL(sSrItemData))
            {
                sSrItemData->getPath(sPath);

                _tcscpy(sSource + sPos, sPath);
                sPos += (xpr_sint_t)_tcslen(sPath) + 1;
                sRCount++;
            }
        }
        sSource[sPos] = '\0';

        BROWSEINFO sBrowseInfo = {0};
        sBrowseInfo.hwndOwner = sSearchResultCtrl->GetSafeHwnd();
        sBrowseInfo.lpszTitle = gApp.loadString(XPR_STRING_LITERAL("popup.move_to_folder.folder_browse.title"));
        sBrowseInfo.ulFlags   = BIF_RETURNONLYFSDIRS;// | BIF_USENEWUI;
        LPITEMIDLIST sFullPidl = ::SHBrowseForFolder(&sBrowseInfo);

        if (XPR_IS_NOT_NULL(sFullPidl))
        {
            xpr_tchar_t sTarget[XPR_MAX_PATH + 1];
            GetName(sFullPidl, SHGDN_FORPARSING, sTarget);
            sTarget[_tcslen(sTarget)] = '\0';

            SHFILEOPSTRUCT sShFileOpStruct = {0};
            sShFileOpStruct.hwnd   = sSearchResultCtrl->GetSafeHwnd();
            sShFileOpStruct.wFunc  = FO_MOVE;
            sShFileOpStruct.fFlags = 0;//(sRCount > 1) ? FOF_MULTIDESTFILES : 0;
            sShFileOpStruct.pFrom  = sSource;
            sShFileOpStruct.pTo    = sTarget;
            ::SHFileOperation(&sShFileOpStruct);
        }

        COM_FREE(sFullPidl);
        XPR_SAFE_DELETE_ARRAY(sSource);
    }
    FXFILE_COMMAND_ELSE_IF_EXPLORER_CTRL
    {
        xpr_sint_t sCount = sExplorerCtrl->GetSelectedCount();
        if (sCount <= 0)
            return;

        LPITEMIDLIST sOldPidl = sExplorerCtrl->getMoveToPidl();

        BROWSEINFO sBrowseInfo = {0};
        sBrowseInfo.hwndOwner = sExplorerCtrl->GetSafeHwnd();
        sBrowseInfo.lpszTitle = gApp.loadString(XPR_STRING_LITERAL("popup.move_to_folder.folder_browse.title"));
        sBrowseInfo.ulFlags   = BIF_RETURNONLYFSDIRS;
        sBrowseInfo.lpfn      = (BFFCALLBACK)BrowseCallbackProc;
        sBrowseInfo.lParam    = (LPARAM)sOldPidl;

        LPITEMIDLIST sFullPidl = ::SHBrowseForFolder(&sBrowseInfo);
        if (XPR_IS_NOT_NULL(sFullPidl))
        {
            xpr_sint_t sCount = 0;
            xpr_tchar_t *sSource = XPR_NULL;
            if (sExplorerCtrl->getSelItemPath(&sSource, sCount, SFGAO_CANMOVE) == XPR_TRUE)
            {
                xpr_tchar_t *sTarget = new xpr_tchar_t[XPR_MAX_PATH + 1];
                GetName(sFullPidl, SHGDN_FORPARSING, sTarget);
                sTarget[_tcslen(sTarget) + 1] = XPR_STRING_LITERAL('\0');

                FileOpThread *sFileOpThread = new FileOpThread;
                sFileOpThread->start(XPR_FALSE, sSource, sCount, sTarget);
            }

            sExplorerCtrl->setMoveToPidl(sFullPidl);
        }
    }
}
} // namespace cmd
} // namespace fxfile
