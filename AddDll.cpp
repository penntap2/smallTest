// ----------------------------------------------------------------------------
// Script Name: 
// Creation Date: 
// Last Modified: 
// Copyright (c)2007
// Purpose: 
// ----------------------------------------------------------------------------
I made a change!
// AddDll.cpp : implementation file
//
// change here.
// change here.
// change here.

#include "stdafx.h"
#include "UEStudio.h"
#include "AddDll.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddDll dialog


CAddDll::CAddDll(CWnd* pParent /*=NULL*/)
  : CUEStudioDialog(CAddDll::IDD, pParent)
{
  //{{AFX_DATA_INIT(CAddDll)
  m_szFolder = _T("");
  //}}AFX_DATA_INIT
}


void CAddDll::DoDataExchange(CDataExchange* pDX)
{
  CUEStudioDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CAddDll)
  DDX_Control(pDX, IDOK, m_bOK);
  DDX_Control(pDX, IDC_PRJFOLDER, m_cFolders);
  DDX_Control(pDX, IDC_LIST1, m_cList);
  DDX_CBString(pDX, IDC_PRJFOLDER, m_szFolder);
  //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddDll, CUEStudioDialog)
  //{{AFX_MSG_MAP(CAddDll)
  ON_BN_CLICKED(IDOK, OnOk)
  ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
  ON_LBN_SELCHANGE(IDC_LIST1, OnSelchangeList1)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddDll message handlers

static void AddChildFolders(CProTreeCtrl* pTree, HTREEITEM hParent, HTREEITEM hSelected, CComboBox* pCb)
{
  int iImage;
  HTREEITEM hItem = pTree->GetChildItem(hParent);
  while (hItem) {
    pTree->GetItemImage(hItem, iImage, iImage);
    if (iImage<=ICO_OFOLDER) {
      CString szFolder = pTree->GetPath(hItem, NULL, _T("\\"), FALSE);
      int iItem = pCb->AddString(szFolder);
      if (hItem==hSelected)
        pCb->SetCurSel(iItem);
      AddChildFolders(pTree, hItem, hSelected, pCb);
    }
    hItem = pTree->GetNextSiblingItem(hItem);
  }
}

BOOL CAddDll::OnInitDialog()
{
  CUEStudioDialog::OnInitDialog();
  AddDLLs();
  AddChildFolders(pTree, pTree->GetRootItem(), pTree->GetSelectedItem(), &m_cFolders);
  return TRUE;
}

void CAddDll::OnOk()
{
  int iCount = m_cList.GetSelCount();
  if (iCount) {
    CString szItem;
    int* pItems = new int[iCount];
    if (pItems) {
      iCount = m_cList.GetSelItems(iCount, pItems);
      for (int iIndex=0; iIndex<iCount; iIndex++) {
        m_cList.GetText(pItems[iIndex], szItem);
        pList->AddTail(szItem);
      }
      delete[] pItems;
    }
  }
  CUEStudioDialog::OnOK();
}

void CAddDll::OnSelchangeList1()
{
  m_bOK.EnableWindow(m_cList.GetSelCount());
}

void CAddDll::OnBrowse()
{
  CString sFolder =
    BrowseForFolder(FormatString(IDS_BROWSE_DLL_DIR), m_hWnd);
  if (!sFolder.IsEmpty()) {
    if (SetCurrentDirectory(sFolder)) {
      AddDLLs();
      SetCurrentDirectory(idm_common::GetAppPath());
    } else {
      ShowErrorMessage();
    }
  }
}

void CAddDll::AddDLLs()
{
  CFileFind ff;
  m_cList.ResetContent();
  BOOL bContinue = ff.FindFile(_T("*.dll"));
  while (bContinue) {
    bContinue = ff.FindNextFile();
    m_cList.AddString(ff.GetFilePath());
  }
}