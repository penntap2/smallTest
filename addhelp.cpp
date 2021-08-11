// ----------------------------------------------------------------------------
// Script Name: 
// Creation Date: 
// Last Modified: 
// Copyright (c)2007
// Purpose: 
// ----------------------------------------------------------------------------
I made a change!
// addhelp.cpp : implementation file
//
// change here.

#include "stdafx.h"
#include "edit.h"
#include "hexfind.h"
#include "undo.h"
#include "editdoc.h"
#include "editmain.h"
#include "addhelp.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#ifdef _WIN32
#define _fstrcpy strcpy
#define far
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddHelp dialog


CAddHelp::CAddHelp(CWnd* pParent /*=NULL*/)
  : CDialog(CAddHelp::IDD, pParent)
{
  //{{AFX_DATA_INIT(CAddHelp)
  m_szHelpFile = _T("");
  m_szHelpMenu = _T("");
  //}}AFX_DATA_INIT
}

void CAddHelp::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CAddHelp)
  DDX_Control(pDX, IDC_HELPFILELIST, m_ListControl);
  DDX_Text(pDX, IDC_HELPFILE, m_szHelpFile);
  DDX_Text(pDX, IDC_HELPMENU, m_szHelpMenu);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAddHelp, CDialog)
  //{{AFX_MSG_MAP(CAddHelp)
  ON_LBN_DBLCLK(IDC_HELPFILELIST, OnDblclkHelpfilelist)
  ON_BN_CLICKED(IDC_HELPBROWSE, OnHelpbrowse)
  ON_BN_CLICKED(IDC_HELPDEL, OnHelpdel)
  ON_BN_CLICKED(IDC_HELPINS, OnHelpins)
  ON_BN_CLICKED(IDC_HELPREPL, OnHelprepl)
  ON_EN_CHANGE(IDC_HELPFILE, OnChangeHelpItems)
  ON_BN_CLICKED(IDC_HELPBROWSE3, OnHelpbrowse3)
  ON_EN_CHANGE(IDC_HELPMENU, OnChangeHelpItems)
  ON_BN_CLICKED(IDC_HELPBROWSE2, OnHelpbrowse2)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CAddHelp message handlers

BOOL CAddHelp::OnInitDialog()
{
  CDialog::OnInitDialog();

  char szNum[2];
  szNum[1] = 0;

  // Load up the arrays.
  m_szHelpFileArray.SetSize(MAX_HELP_FILES);
  m_szMenuNameArray.SetSize(MAX_HELP_FILES);

  struct HelpFiles far * lpHelpFileStruct = (HelpFiles far *) GlobalLock(pGlobalEditFrame->m_hHelpFiles);

  int i;
  for (i = 0; i < MAX_HELP_FILES; i++) {
    szNum[0] = 0x30 + i;

    m_szHelpFileArray.SetAt(i,lpHelpFileStruct[i].szHelpFile);
    m_szMenuNameArray.SetAt(i,lpHelpFileStruct[i].szMenuName);

    if (m_szMenuNameArray[i] != "")
      m_ListControl.AddString(m_szMenuNameArray[i]);
  }

  m_ListControl.AddString("");
  GlobalUnlock(pGlobalEditFrame->m_hHelpFiles);

  m_ListControl.SetCurSel(0);

  EnableControls();

  return TRUE;  // return TRUE unless you set the focus to a control
                // EXCEPTION: OCX Property Pages should return FALSE
}

void CAddHelp::OnDblclkHelpfilelist()
{
  // TODO: Add your control notification handler code here
  int i = m_ListControl.GetCurSel();
  if (i < 0)
    i = 0;

  m_szHelpFile = m_szHelpFileArray[i];
  m_szHelpMenu = m_szMenuNameArray[i];
  UpdateData(FALSE);
  EnableControls();
}

void CAddHelp::OnHelpbrowse()
{
  // TODO: Add your control notification handler code here
  CString strFilter;
  strFilter = GetResourceString(IDS_HELPFILE_TYPE);// += "Help Files";
  strFilter += (char)'\0';
  strFilter += "*.HLP; *.CHM; *.COL;";
  strFilter += (char)'\0';        // next string please

  CString sTitle;
  sTitle = GetResourceString(IDS_SELECTHELP_FILE);
  AfxGetEditApp()->OnFileBrowse(TRUE, m_szHelpFile, sTitle, &strFilter);
  UpdateData(FALSE);
  EnableControls();
}

void CAddHelp::OnHelpdel()
{
  // TODO: Add your control notification handler code here
  UpdateData(TRUE);
  int i = m_ListControl.GetCurSel();
  if (i < 0)
    i = 0;
  int j;
  for (j = i; j < (MAX_HELP_FILES-1); j++) {
    m_szHelpFileArray.SetAt(j,m_szHelpFileArray[j+1]);
    m_szMenuNameArray.SetAt(j,m_szMenuNameArray[j+1]);
  }
  m_szHelpFileArray.SetAt(j,"");
  m_szMenuNameArray.SetAt(j,"");

  m_ListControl.DeleteString(i);
  m_ListControl.SetCurSel(max(i-1,0));
}

void CAddHelp::OnHelpins()
{
  // TODO: Add your control notification handler code here
  UpdateData(TRUE);
  // Make sure the fields are not blank
  if (m_szHelpMenu != "") {
    int i = m_ListControl.GetCurSel();
    if (i < 0)
      i = 0;
    int j;
    for (j = MAX_HELP_FILES-1; j > i; j--) {
      m_szHelpFileArray.SetAt(j,m_szHelpFileArray[j-1]);
      m_szMenuNameArray.SetAt(j,m_szMenuNameArray[j-1]);
    }
    m_ListControl.InsertString(i, m_szHelpMenu);
    m_ListControl.SetCurSel(i);
    OnHelprepl();
  }
}

void CAddHelp::OnHelprepl()
{
  // TODO: Add your control notification handler code here
  UpdateData(TRUE);
  int i = m_ListControl.GetCurSel();
  if (i < 0)
    i = 0;
  m_szHelpFileArray.SetAt(i,m_szHelpFile);
  m_szMenuNameArray.SetAt(i,m_szHelpMenu);

  m_ListControl.DeleteString(i);
  m_ListControl.InsertString(i, m_szHelpMenu);
}

void CAddHelp::OnOK()
{
  // TODO: Add extra validation here
  CDialog::OnOK();
}

INT_PTR CAddHelp::DoModal()
{
  int iReturn = CDialog::DoModal();

  if (iReturn == IDOK) {
    m_szHelpFileArray.SetSize(MAX_HELP_FILES,1);
    m_szMenuNameArray.SetSize(MAX_HELP_FILES,1);

    struct HelpFiles far * lpHelpFileStruct = (HelpFiles far *) GlobalLock(pGlobalEditFrame->m_hHelpFiles);
    char szNum[2];
    szNum[1] = 0;

    int i;
    for (i = 0; i < MAX_HELP_FILES; i++) {
      szNum[0] = 0x30 + i;
      AfxGetEditApp()->WriteProfileString(szHelpFiles, CString(szHelpFileNum)+szNum, m_szHelpFileArray[i]);
      _fstrcpy(lpHelpFileStruct[i].szHelpFile, m_szHelpFileArray[i]);
      AfxGetEditApp()->WriteProfileString(szHelpFiles, CString(szHelpMenu)+szNum, m_szMenuNameArray[i]);
      _fstrcpy(lpHelpFileStruct[i].szMenuName,    m_szMenuNameArray[i]);
    }
    GlobalUnlock(pGlobalEditFrame->m_hHelpFiles);
  }

  return iReturn;
}

void CAddHelp::EnableControls()
{
  UpdateData(TRUE);
  CWnd* pWnd1 = GetDlgItem(IDC_HELPINS);
  CWnd* pWnd2 = GetDlgItem(IDC_HELPREPL);

  if ((m_szHelpFile == "") || (m_szHelpMenu == "")) {
    if (pWnd1 != NULL)
      pWnd1->EnableWindow(FALSE);
    if (pWnd2 != NULL)
      pWnd2->EnableWindow(FALSE);
  }
  else {
    if (pWnd1 != NULL)
      pWnd1->EnableWindow(TRUE);
    if (pWnd2 != NULL)
      pWnd2->EnableWindow(TRUE);
  }
}

void CAddHelp::OnChangeHelpItems()
{
  // TODO: Add your control notification handler code here
    EnableControls();
}

/////////////////////////////////////////////////////////////////////////////
// CSelHelp dialog


CSelHelp::CSelHelp(CWnd* pParent /*=NULL*/)
  : CDialog(CSelHelp::IDD, pParent)
{
  //{{AFX_DATA_INIT(CSelHelp)
  m_szHelpTopic = _T("");
  //}}AFX_DATA_INIT
}


void CSelHelp::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CSelHelp)
  DDX_Control(pDX, IDC_HELPFILELIST, m_cFileList);
  DDX_Text(pDX, IDC_HELPTOPIC, m_szHelpTopic);
  //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelHelp, CDialog)
  //{{AFX_MSG_MAP(CSelHelp)
  ON_LBN_DBLCLK(IDC_HELPFILELIST, OnDblclkHelpfilelist)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSelHelp message handlers

void CSelHelp::OnDblclkHelpfilelist()
{
  // TODO: Add your control notification handler code here
  OnOK();
}

void CSelHelp::OnOK()
{
  UpdateData(TRUE);

  struct HelpFiles far * lpHelpFileStruct = (HelpFiles far *) GlobalLock(pGlobalEditFrame->m_hHelpFiles);
  int i = m_cFileList.GetCurSel();
  char szFile[_MAX_PATH+1];

  strcpy(szFile, lpHelpFileStruct[i].szHelpFile);

  m_szHelpFile = szFile;
  GlobalUnlock(pGlobalEditFrame->m_hHelpFiles);

  CDialog::OnOK();
}

BOOL CSelHelp::OnInitDialog()
{
  CDialog::OnInitDialog();

  struct HelpFiles far * lpHelpFileStruct = (HelpFiles far *) GlobalLock(pGlobalEditFrame->m_hHelpFiles);

  int i;
  for (i = 0; i < MAX_HELP_FILES; i++) {
    if (lpHelpFileStruct[i].szMenuName[0] != 0) {
      m_cFileList.AddString(lpHelpFileStruct[i].szMenuName);
    }
  }
  GlobalUnlock(pGlobalEditFrame->m_hHelpFiles);
  m_cFileList.SetCurSel(0);

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

void CAddHelp::OnHelpbrowse3()
{
  // MSDN
  UpdateData();
  m_szHelpFile = _T("http://search.microsoft.com/search/results.aspx?View=msdn&st=a&qu=$K&c=4&s=2");
  m_szHelpMenu = _T("MSDN Online");
  UpdateData(0);
  SendMessage(WM_COMMAND, IDC_HELPINS);
}

void CAddHelp::OnHelpbrowse2()
{
  // GOOGLE
  UpdateData();
  m_szHelpFile = _T("http://www.google.com/search?hl=en&ie=UTF-8&oe=UTF-8&q=$K&btnG=Google+Search");
  m_szHelpMenu = _T("Google Search");
  UpdateData(0);
  SendMessage(WM_COMMAND, IDC_HELPINS);
}
