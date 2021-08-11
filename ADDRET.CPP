// ----------------------------------------------------------------------------
// Script Name: 
// Creation Date: 
// Last Modified: 
// Copyright (c)2007
// Purpose: 
// ----------------------------------------------------------------------------
I made a change!
// change here
// addret.cpp : implementation file
//

#include "stdafx.h"
#include "edit.h"
#include "addret.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddRet dialog


CAddRet::CAddRet(CWnd* pParent /*=NULL*/)
    : CDialog(CAddRet::IDD, pParent)
{
    //{{AFX_DATA_INIT(CAddRet)
    m_ColumnNum = 1;
    m_UseWindow = 0;
    //}}AFX_DATA_INIT
}

void CAddRet::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CAddRet)
    DDX_Text(pDX, IDC_COLUMN, m_ColumnNum);
    DDV_MinMaxInt(pDX, m_ColumnNum, 1, 2048);
    DDX_Radio(pDX, IDC_USEWINDOW, m_UseWindow);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAddRet, CDialog)
    //{{AFX_MSG_MAP(CAddRet)
  ON_EN_CHANGE(IDC_COLUMN, OnChangeColumn)
  ON_EN_UPDATE(IDC_COLUMN, OnChangeColumn)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CAddRet message handlers

void CAddRet::OnChangeColumn()
{
  // TODO: If this is a RICHEDIT control, the control will not
  // send this notification unless you override the CDialog::OnInitDialog()
  // function and call CRichEditCtrl().SetEventMask()
  // with the ENM_CHANGE flag ORed into the mask.

  // TODO: Add your control notification handler code here
    SendDlgItemMessage(IDC_USECOLUMN, BM_SETCHECK, BST_CHECKED, (LPARAM) 0);
    SendDlgItemMessage(IDC_USEWINDOW, BM_SETCHECK, BST_UNCHECKED, (LPARAM) 0);
}
