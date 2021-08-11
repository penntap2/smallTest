// ----------------------------------------------------------------------------
// Script Name: 
// Creation Date: 
// Last Modified: 
// Copyright (c)2007
// Purpose: 
// ----------------------------------------------------------------------------
I made a change!
// ArtisticStyler.cpp : implementation file
//

#include "stdafx.h"
#include "UEStudio.h"
#include "ArtisticStyler.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CArtisticStyler dialog

#define STRDEFAULT _T("default")

CArtisticStyler::CArtisticStyler(CWnd* pParent /*=NULL*/)
  : CUEStudioDialog(CArtisticStyler::IDD, pParent)
{
  //{{AFX_DATA_INIT(CArtisticStyler)
  m_szStyle         = _T("");
  m_szMode          = _T("");
  m_bClass          = FALSE;
  m_bSwitch         = FALSE;
  m_bCase           = FALSE;
  m_bCol1           = FALSE;
  m_bNamespace      = FALSE;
  m_bLabel          = FALSE;
  m_szMaxIndent     = _T("");
  m_szMinIndent     = _T("");
  m_bPreprocessor   = FALSE;
  m_bTabsToSpaces   = FALSE;
  m_szEmptyLines    = _T("");
  m_bBreakBlocks    = FALSE;
  m_szBBAll         = _T("");
  m_bBreakElseIf    = FALSE;
  m_bKeepStatement  = FALSE;
  m_bKeepBlock      = FALSE;
  m_szAlignPointer  = _T("");
  m_szAlignReference = _T("");
  m_bCloseTemplates = FALSE;
  m_bAddBrackets    = FALSE;
  m_bAddOLBrackets  = FALSE;
  m_szMaxCodeLength = _T("");
  m_bBreakAfterLogical = FALSE;
  m_szOptionsFile   = _T("");
  m_bSpacePadding1 = FALSE;
  m_bSpacePadding2 = FALSE;
  m_bSpacePadding3 = FALSE;
  m_bSpacePadding4 = FALSE;
  m_bSpacePadding5 = FALSE;
  m_bSpacePadding6 = FALSE;
  // load data
  //}}AFX_DATA_INIT
}

void CArtisticStyler::LoadData()
{
  UpdateData(TRUE);
  m_szStyle         = theApp.GetProfileString(_T("ArtisticStyle"), _T("Style"), STRDEFAULT);
  m_szMode          = theApp.GetProfileString(_T("ArtisticStyle"), _T("Mode"), STRDEFAULT);
  m_bClass          = theApp.GetProfileInt(_T("ArtisticStyle"), _T("Class"), FALSE);
  m_bSwitch         = theApp.GetProfileInt(_T("ArtisticStyle"), _T("Switch"), FALSE);
  m_bCase           = theApp.GetProfileInt(_T("ArtisticStyle"), _T("Case"), FALSE);
  m_bCol1           = theApp.GetProfileInt(_T("ArtisticStyle"), _T("Col1"), FALSE);
  m_bNamespace      = theApp.GetProfileInt(_T("ArtisticStyle"), _T("Namespace"), FALSE);
  m_bLabel          = theApp.GetProfileInt(_T("ArtisticStyle"), _T("Label"), FALSE);
  m_szMaxIndent     = theApp.GetProfileString(_T("ArtisticStyle"), _T("MaxIndent"));
  m_szMinIndent     = theApp.GetProfileString(_T("ArtisticStyle"), _T("MinIndent"));
  m_bPreprocessor   = theApp.GetProfileInt(_T("ArtisticStyle"), _T("Preprocessor"), FALSE);
  m_bTabsToSpaces   = theApp.GetProfileInt(_T("ArtisticStyle"), _T("TabsToSpaces"), FALSE);
  m_szEmptyLines    = theApp.GetProfileString(_T("ArtisticStyle"), _T("EmptyLines"), STRDEFAULT);
  m_bBreakBlocks    = theApp.GetProfileInt(_T("ArtisticStyle"), _T("BreakBlocks"), FALSE);
  m_szBBAll         = theApp.GetProfileString(_T("ArtisticStyle"), _T("BreakBlocksAll"), STRDEFAULT);
  m_bBreakElseIf    = theApp.GetProfileInt(_T("ArtisticStyle"), _T("BreakElseIf"), FALSE);
  m_bKeepStatement  = theApp.GetProfileInt(_T("ArtisticStyle"), _T("KeepStatement"), FALSE);
  m_bKeepBlock      = theApp.GetProfileInt(_T("ArtisticStyle"), _T("KeepBlock"), FALSE);
  m_szAlignPointer  = theApp.GetProfileString(_T("ArtisticStyle"), _T("AlignPointer"), STRDEFAULT);
  m_szAlignReference = theApp.GetProfileString(_T("ArtisticStyle"), _T("AlignReference"), STRDEFAULT);
  m_bCloseTemplates = theApp.GetProfileInt(_T("ArtisticStyle"), _T("CloseTemplates"), FALSE);
  m_bAddBrackets    = theApp.GetProfileInt(_T("ArtisticStyle"), _T("AddBrackets"), FALSE);
  m_bAddOLBrackets  = theApp.GetProfileInt(_T("ArtisticStyle"), _T("AddOLBrackets"), FALSE);
  m_szMaxCodeLength = theApp.GetProfileString(_T("ArtisticStyle"), _T("MaxCodeLength"));
  m_bBreakAfterLogical = theApp.GetProfileInt(_T("ArtisticStyle"), _T("BreakAfterLogical"), FALSE);
  m_szOptionsFile   = theApp.GetProfileString(_T("ArtisticStyle"), _T("OptionsFile"));
  m_bSpacePadding1  = theApp.GetProfileInt(_T("ArtisticStyle"), _T("SpacePadding1"), FALSE);
  m_bSpacePadding2  = theApp.GetProfileInt(_T("ArtisticStyle"), _T("SpacePadding2"), FALSE);
  m_bSpacePadding3  = theApp.GetProfileInt(_T("ArtisticStyle"), _T("SpacePadding3"), FALSE);
  m_bSpacePadding4  = theApp.GetProfileInt(_T("ArtisticStyle"), _T("SpacePadding4"), FALSE);
  m_bSpacePadding5  = theApp.GetProfileInt(_T("ArtisticStyle"), _T("SpacePadding5"), FALSE);
  m_bSpacePadding6  = theApp.GetProfileInt(_T("ArtisticStyle"), _T("SpacePadding6"), FALSE);
  UpdateData(FALSE);
}

void CArtisticStyler::SaveData()
{
  UpdateData(TRUE);
  theApp.WriteProfileString(_T("ArtisticStyle"), _T("Style"), m_szStyle);
  theApp.WriteProfileString(_T("ArtisticStyle"), _T("Mode"), m_szMode);
  theApp.WriteProfileInt(_T("ArtisticStyle"), _T("Class"), m_bClass);
  theApp.WriteProfileInt(_T("ArtisticStyle"), _T("Switch"), m_bSwitch);
  theApp.WriteProfileInt(_T("ArtisticStyle"), _T("Case"), m_bCase);
  theApp.WriteProfileInt(_T("ArtisticStyle"), _T("Col1"), m_bCol1);
  theApp.WriteProfileInt(_T("ArtisticStyle"), _T("Namespace"), m_bNamespace);
  theApp.WriteProfileInt(_T("ArtisticStyle"), _T("Label"), m_bLabel);
  theApp.WriteProfileString(_T("ArtisticStyle"), _T("MaxIndent"), m_szMaxIndent);
  theApp.WriteProfileString(_T("ArtisticStyle"), _T("MinIndent"), m_szMinIndent);
  theApp.WriteProfileInt(_T("ArtisticStyle"), _T("Preprocessor"), m_bPreprocessor);
  theApp.WriteProfileInt(_T("ArtisticStyle"), _T("TabsToSpaces"), m_bTabsToSpaces);
  theApp.WriteProfileString(_T("ArtisticStyle"), _T("EmptyLines"), m_szEmptyLines);
  theApp.WriteProfileInt(_T("ArtisticStyle"), _T("BreakBlocks"), m_bBreakBlocks);
  theApp.WriteProfileString(_T("ArtisticStyle"), _T("BreakBlocksAll"), m_szBBAll);
  theApp.WriteProfileInt(_T("ArtisticStyle"), _T("BreakElseIf"), m_bBreakElseIf);
  theApp.WriteProfileInt(_T("ArtisticStyle"), _T("KeepStatement"), m_bKeepStatement);
  theApp.WriteProfileInt(_T("ArtisticStyle"), _T("KeepBlock"), m_bKeepBlock);
  theApp.WriteProfileString(_T("ArtisticStyle"), _T("AlignPointer"), m_szAlignPointer);
  theApp.WriteProfileString(_T("ArtisticStyle"), _T("AlignReference"), m_szAlignReference);
  theApp.WriteProfileInt(_T("ArtisticStyle"), _T("CloseTemplates"), m_bCloseTemplates);
  theApp.WriteProfileInt(_T("ArtisticStyle"), _T("AddBrackets"), m_bAddBrackets);
  theApp.WriteProfileInt(_T("ArtisticStyle"), _T("AddOLBrackets"), m_bAddOLBrackets);
  theApp.WriteProfileString(_T("ArtisticStyle"), _T("MaxCodeLength"), m_szMaxCodeLength);
  theApp.WriteProfileInt(_T("ArtisticStyle"), _T("BreakAfterLogical"), m_bBreakAfterLogical);
  theApp.WriteProfileString(_T("ArtisticStyle"), _T("OptionsFile"), m_szOptionsFile);
  theApp.WriteProfileInt(_T("ArtisticStyle"), _T("SpacePadding1"), m_bSpacePadding1);
  theApp.WriteProfileInt(_T("ArtisticStyle"), _T("SpacePadding2"), m_bSpacePadding2);
  theApp.WriteProfileInt(_T("ArtisticStyle"), _T("SpacePadding3"), m_bSpacePadding3);
  theApp.WriteProfileInt(_T("ArtisticStyle"), _T("SpacePadding4"), m_bSpacePadding4);
  theApp.WriteProfileInt(_T("ArtisticStyle"), _T("SpacePadding5"), m_bSpacePadding5);
  theApp.WriteProfileInt(_T("ArtisticStyle"), _T("SpacePadding6"), m_bSpacePadding6);
}

BOOL CArtisticStyler::OnInitDialog()
{
  CUEStudioDialog::OnInitDialog();
  AddCBox(m_cStyle, STRDEFAULT, _T("allman"), _T("ansi"), _T("bsd"), _T("break"),
    _T("java"), _T("attach"),
    _T("kr"), _T("stroustrup"), _T("whitesmith"), _T("banner"), _T("gnu"), _T("linux"),
    _T("horstmann"), _T("otbs"),
    _T("pico"), _T("lisp"), NULL);
  AddCBox(m_cMode, STRDEFAULT, _T("c"), _T("cs"), _T("java"), NULL);
  AddCBox(m_cEmptyLines, STRDEFAULT, _T("fill"), _T("delete"), NULL);
  AddCBox(m_cAlignPointer, STRDEFAULT, _T("type"), _T("middle"), _T("name"), NULL);
  AddCBox(m_cAlignReference, STRDEFAULT, _T("none"), _T("type"), _T("middle"), _T("name"), NULL);
  AddCBox(m_cBreakBlocks, STRDEFAULT, _T("all"), NULL);
  LoadData();
  OnKeepStatement();
  OnMaxCodeLen();
  return TRUE;
}

void CArtisticStyler::OnKeepStatement()
{
  UpdateData();
  m_cBreakElseIf.EnableWindow(m_bKeepStatement == FALSE);
}

void CArtisticStyler::OnMaxCodeLen()
{
  UpdateData();
  m_cBreakAfterLogical.EnableWindow(m_szMaxCodeLength.GetLength() == 0);
}

void CArtisticStyler::OnDestroy()
{
  //SaveData();
  //MakeOptionsLine();
  CUEStudioDialog::OnDestroy();
}

void CArtisticStyler::DoDataExchange(CDataExchange* pDX)
{
  CUEStudioDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CArtisticStyler)
  DDX_Control(pDX, IDC_MODE, m_cMode);
  DDX_Control(pDX, IDC_FORMATTING_SPACE_PADDING1, m_cSpacePadding1);
  DDX_Control(pDX, IDC_FORMATTING_SPACE_PADDING2, m_cSpacePadding2);
  DDX_Control(pDX, IDC_FORMATTING_SPACE_PADDING3, m_cSpacePadding3);
  DDX_Control(pDX, IDC_FORMATTING_SPACE_PADDING4, m_cSpacePadding4);
  DDX_Control(pDX, IDC_FORMATTING_SPACE_PADDING5, m_cSpacePadding5);
  DDX_Control(pDX, IDC_FORMATTING_SPACE_PADDING6, m_cSpacePadding6);
  DDX_Control(pDX, IDC_COMBO1, m_cBreakBlocks);
  DDX_Control(pDX, IDC_STYLE, m_cStyle);
  DDX_Control(pDX, IDC_INDENT_EMPTY_LINES, m_cEmptyLines);
  DDX_Control(pDX, IDC_INDENT_ALIGN_POINTER, m_cAlignPointer);
  DDX_Control(pDX, IDC_INDENT_ALIGN_REFERENCE, m_cAlignReference);
  DDX_Control(pDX, IDC_FORMAT_BREAK_AFTER_LOGICAL, m_cBreakAfterLogical);
  DDX_Control(pDX, IDC_CHECK2, m_cKeepStatement);
  DDX_Control(pDX, IDC_CHECK1, m_cBreakElseIf);
  DDX_CBString(pDX, IDC_STYLE, m_szStyle);
  DDX_CBString(pDX, IDC_MODE, m_szMode);
  DDX_Check(pDX, IDC_INDENT_CLASSES, m_bClass);
  DDX_Check(pDX, IDC_INDENT_SWITCHES, m_bSwitch);
  DDX_Check(pDX, IDC_INDENT_CASES, m_bCase);
  DDX_Check(pDX, IDC_INDENT_COL1, m_bCol1);
  DDX_Check(pDX, IDC_INDENT_NAMESPACES, m_bNamespace);
  DDX_Check(pDX, IDC_INDENT_LABEL, m_bLabel);
  DDX_Text(pDX, IDC_MAXINDENT, m_szMaxIndent);
  DDX_Text(pDX, IDC_MININDENT, m_szMinIndent);
  DDX_Check(pDX, IDC_INDENT_PREPROCESSOR, m_bPreprocessor);
  DDX_Check(pDX, IDC_INDENT_TABSTOSPACES, m_bTabsToSpaces);
  DDX_CBString(pDX, IDC_INDENT_EMPTY_LINES, m_szEmptyLines);
  DDX_Check(pDX, IDC_BREAK_BLOCKS, m_bBreakBlocks);
  DDX_CBString(pDX, IDC_COMBO1, m_szBBAll);
  DDX_Check(pDX, IDC_CHECK1, m_bBreakElseIf);
  DDX_Check(pDX, IDC_FORMATTING_SPACE_PADDING1, m_bSpacePadding1);
  DDX_Check(pDX, IDC_FORMATTING_SPACE_PADDING2, m_bSpacePadding2);
  DDX_Check(pDX, IDC_FORMATTING_SPACE_PADDING3, m_bSpacePadding3);
  DDX_Check(pDX, IDC_FORMATTING_SPACE_PADDING4, m_bSpacePadding4);
  DDX_Check(pDX, IDC_FORMATTING_SPACE_PADDING5, m_bSpacePadding5);
  DDX_Check(pDX, IDC_FORMATTING_SPACE_PADDING6, m_bSpacePadding6);
  DDX_Check(pDX, IDC_CHECK2, m_bKeepStatement);
  DDX_Check(pDX, IDC_CHECK3, m_bKeepBlock);
  DDX_CBString(pDX, IDC_INDENT_ALIGN_POINTER, m_szAlignPointer);
  DDX_CBString(pDX, IDC_INDENT_ALIGN_REFERENCE, m_szAlignReference);
  DDX_Check(pDX, IDC_FORMAT_CLOSE_TEMPLATES, m_bCloseTemplates);
  DDX_Check(pDX, IDC_FORMAT_ADD_BRACKETS, m_bAddBrackets);
  DDX_Check(pDX, IDC_FORMAT_ADD_OLBRACKETS, m_bAddOLBrackets);
  //DDX_CBString(pDX, IDC_FORMAT_MAX_CODE_LENGTH, m_szMaxCodeLength);
  DDX_Check(pDX, IDC_FORMAT_BREAK_AFTER_LOGICAL, m_bBreakAfterLogical);
  DDX_Text(pDX, IDC_EDIT_OPTIONS, m_szOptionsFile);
  //}}AFX_DATA_MAP
  if (pDX->m_bSaveAndValidate) {
    GetDlgItem(IDC_FORMAT_MAX_CODE_LENGTH)->GetWindowText(m_szMaxCodeLength);
  } else {
    GetDlgItem(IDC_FORMAT_MAX_CODE_LENGTH)->SetWindowText(m_szMaxCodeLength);
  }
}

BEGIN_MESSAGE_MAP(CArtisticStyler, CDialog)
  //{{AFX_MSG_MAP(CArtisticStyler)
  ON_WM_DESTROY()
  ON_COMMAND(IDC_BUTTON_OPTIONS, OnBrowseOpt)
  ON_EN_CHANGE(IDC_FORMAT_MAX_CODE_LENGTH, OnMaxCodeLen)
  ON_COMMAND(IDC_CHECK2, OnKeepStatement)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CArtisticStyler::OnBrowseOpt()
{
  UpdateData();
  CFileDialog cfd(TRUE, _T(".*"), NULL,
    OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_HIDEREADONLY, FormatString(IDS_ADDFILES_FILTER));
  CSplitPath sp1(m_szOptionsFile);
  CString sPath = sp1.FullPath();
  cfd.m_ofn.lpstrInitialDir = sPath;
  if (cfd.DoModal()==IDOK) {
    m_szOptionsFile = cfd.GetPathName();
    UpdateData(FALSE);
    ShowOptions();
  }
}

BOOL CArtisticStyler::PreTranslateMessage(MSG* pMsg)
{
  switch (pMsg->message) {
  case WM_KEYDOWN:
     switch (pMsg->wParam) {
     case VK_LEFT:
     case VK_RIGHT:
     case VK_UP:
     case VK_DOWN:
       return
         CUEStudioDialog::PreTranslateMessage(pMsg);
     default:
       break;
     }
  case WM_PASTE:
    {
      CString sClass = GetClassName(pMsg->hwnd);
      if (sClass.Compare(_T("Edit")) == 0) {
        HWND hWnd = ::GetParent(pMsg->hwnd);
        if (hWnd != 0) {
          CString sParentClass = GetClassName(hWnd);
          if (sParentClass.Compare(_T("ComboBox")) == 0)
            return TRUE;
        }
      }
    }
    break;
  default:
    break;
  }
  return CUEStudioDialog::PreTranslateMessage(pMsg);
}

BOOL IsValidOption(const CString& sOption) {
  if (sOption.IsEmpty())
    return FALSE;
  if (sOption.Compare(STRDEFAULT) == 0) {
    return FALSE;
  } else {
    return TRUE;
  }
}

void CArtisticStyler::MakeOptionsLine(CString* pOptions)
{
  if (pOptions == 0) {
    pOptions = &m_szOptions;
  }

  pOptions->Empty();

  if (IsValidOption(m_szStyle))
    *pOptions += _T(" --style=") + m_szStyle;

  if (IsValidOption(m_szMode))
    *pOptions += _T(" --mode=") + m_szMode;

  if (m_bClass)
    *pOptions += _T(" -C");

  if (m_bSwitch)
    *pOptions += _T(" -S");

  if (m_bCase)
    *pOptions += _T(" -K");

  if (m_bNamespace)
    *pOptions += _T(" -N");

  if (m_bLabel)
    *pOptions += _T(" -L");

  if (m_bCol1)
    *pOptions += _T(" -Y");

  if (!m_szMinIndent.IsEmpty())
    *pOptions += _T(" -m") + m_szMinIndent;

  if (!m_szMaxIndent.IsEmpty())
    *pOptions += _T(" -M") + m_szMaxIndent;

  if (m_bPreprocessor)
    *pOptions += _T(" --indent-preprocessor");

  if (m_bTabsToSpaces)
    *pOptions += _T(" --convert-tabs");

  if (IsValidOption(m_szEmptyLines)) {
    if (stricmp(m_szEmptyLines, "delete") == 0) {
      *pOptions += _T(" --delete-empty-lines");
    } else if (stricmp(m_szEmptyLines, "fill") == 0) {
      *pOptions += _T(" --fill-empty-lines");
    }
  }

  if (IsValidOption(m_szAlignPointer)) {
    *pOptions += _T(" --align-pointer=") + m_szAlignPointer;
  }

  if (IsValidOption(m_szAlignReference)) {
    *pOptions += _T(" --align-reference=") + m_szAlignReference;
  }

  if (m_bBreakBlocks) {
    if (IsValidOption(m_szBBAll))
      *pOptions += _T(" --break-blocks=") + m_szBBAll;
    else
      *pOptions += _T(" --break-blocks");
  }

  if (m_bBreakElseIf)
    *pOptions += _T(" --break-elseifs");

  if (m_bKeepStatement)
    *pOptions += _T(" -o");

  if (m_bKeepBlock)
    *pOptions += _T(" -O");

  if (m_bCloseTemplates)
    *pOptions += _T(" -xy");

  if (m_bAddBrackets)
    *pOptions += _T(" -j");

  if (m_bAddOLBrackets)
    *pOptions += _T(" -J");

  if (!m_szMaxCodeLength.IsEmpty())
    *pOptions += _T(" -xC") + m_szMaxCodeLength;

  if (m_bBreakAfterLogical)
    *pOptions += _T(" --break-after-logical");

  if (m_bSpacePadding1)
    *pOptions += _T(" --pad-oper");

  if (m_bSpacePadding2)
    *pOptions += _T(" --pad-paren");

  if (m_bSpacePadding3)
    *pOptions += _T(" --pad-first-paren-out");

  if (m_bSpacePadding4)
    *pOptions += _T(" --pad-header");

  if (m_bSpacePadding5)
    *pOptions += _T(" --pad-paren-in");

  if (m_bSpacePadding6)
    *pOptions += _T(" --pad-paren-out");

  if (!m_szOptionsFile.IsEmpty()) {
    *pOptions += _T(" --options=\"") + m_szOptionsFile + _T("\"");
  }

  pOptions->TrimLeft();
  pOptions->TrimRight();
}

void CArtisticStyler::ShowOptions()
{
  static bool imin = false;
  if (imin==false) {
    imin = true;
    CString str;
    try {
      CDataExchange dx(this, TRUE);
      DoDataExchange(&dx);
    } catch (...) {
    }
    MakeOptionsLine(&str);
    SendDlgItemMessage(IDC_CMDLINE, WM_SETTEXT, 0, (LPARAM) (LPCTSTR) str);
    imin = false;
  }
}

BOOL CArtisticStyler::OnCommand(WPARAM wParam, LPARAM lParam)
{
  BOOL bRet = CUEStudioDialog::OnCommand(wParam, lParam);
  switch (LOWORD(wParam)) {
  case IDC_CMDLINE:
    break;
  case IDC_INDENT_ALIGN_POINTER:
  case IDC_INDENT_ALIGN_REFERENCE:
  case IDC_INDENT_EMPTY_LINES:
  case IDC_COMBO1:
  case IDC_STYLE:
  case IDC_MODE:
    //switch (HIWORD(wParam)) {
    //case CBN_:
      ShowOptions();
      //break;
    //}
    break;
  default:
    ShowOptions();
    break;
  }
  return bRet;
}

void CArtisticStyler::OnOK()
{
  UpdateData();
  if (!m_szOptionsFile.IsEmpty()) {
    if (!FileExists(m_szOptionsFile)) {
      AfxMessageBox(FormatString(IDS_OPTIONS_FILE_DOESNT_EXIST, m_szOptionsFile), MB_ICONERROR);
      return;
    }
  }
  if (!m_szMaxCodeLength.IsEmpty()) {
    int nMaxCodeLen = atoi(m_szMaxCodeLength);
    if ((nMaxCodeLen < 50) || (nMaxCodeLen > 200)) {
      AfxMessageBox(FormatString(IDS_OPTIONS_VALUE_BETWEEN1, 50, 200), MB_ICONERROR);
      return;
    }
  }
  if (!m_szMaxIndent.IsEmpty()) {
    int nMaxIndent = atoi(m_szMaxIndent);
    if ((nMaxIndent < 40) || (nMaxIndent > 120)) {
      AfxMessageBox(FormatString(IDS_OPTIONS_VALUE_BETWEEN2, 40, 120), MB_ICONERROR);
      return;
    }
  }
  if (!m_szMinIndent.IsEmpty()) {
    int nMinIndent = atoi(m_szMinIndent);
    if ((nMinIndent < 0) || (nMinIndent > 3)) {
      AfxMessageBox(FormatString(IDS_OPTIONS_VALUE_BETWEEN3, 0, 3), MB_ICONERROR);
      return;
    }
  }
  SaveData();
  MakeOptionsLine();
  EndDialog(IDOK);
}