// ----------------------------------------------------------------------------
// Script Name: 
// Creation Date: 
// Last Modified: 
// Copyright (c)2007
// Purpose: 
// ----------------------------------------------------------------------------
I made a change!
// AsciiTab.cpp : implementation file
//

#include "stdafx.h"
#include "edit.h"
#include "AsciiTab.h"
#include "undo.h"
#include "editdoc.h"
#include "hexfind.h"
#include "editmain.h"
#include "editview.h"
#ifdef USE_DETOURS
#include <coolsb\coolscroll.h>
#include <coolsb\coolsb_detours.h>
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#define HEADER_SPACE_CY 0

static const int FONT_POINT_SIZE = 10;

static int iTabStops[4] = {16, 32, 66, 99};

static int iTabStopOrg = {40};
static int iTabStop = {40};

static int iOrgAveCharWidth;
static int iCharWidthArray[2];

static char cCtrlChar[33] = {"@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"};
static char cCharName[33][4] = {"NUL",
                                "SOH",
                                "STX",
                                "ETX",
                                "EOT",
                                "ENQ",
                                "ACK",
                                "BEL",
                                "BS ",
                                "HT ",
                                "LF ",
                                "VT ",
                                "FF ",
                                "CR ",
                                "SO ",
                                "SI ",
                                "DLE",
                                "DC1",
                                "DC2",
                                "DC3",
                                "DC4",
                                "NAK",
                                "SYN",
                                "ETB",
                                "CAN",
                                "EM ",
                                "SUB",
                                "ESC",
                                "FS ",
                                "GS ",
                                "RS ",
                                "US ",
                                "Sp "};

/////////////////////////////////////////////////////////////////////////////
// CATListBox dialog

IMPLEMENT_DYNAMIC(CATListBox, CATListBoxBase)

CATListBox::CATListBox() 
{ 
  SetFixFont(fixfnt); 
}

CATListBox::~CATListBox() 
{ 
}


BEGIN_MESSAGE_MAP(CATListBox, CATListBoxBase)
  ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()

int CATListBox::AddString(LPCTSTR lpszItem)
{
  int index = CATListBoxBase::AddString(_T(""));
  if (index > -1) {
    POSITION pos = lst.AddTail(lpszItem);
    CString* str = &lst.GetAt(pos);
    CATListBoxBase::SetItemData(index, (DWORD_PTR)str);
  }
  return index;
}

BOOL CATListBox::SetTabStops(int nTabStops, LPINT rgTabStops)
{
  if (nTabStops > 0 && rgTabStops) {
    sTabs.SetSize(nTabStops);
    for (int i = 0; i < nTabStops; i++)
      sTabs[i] = rgTabStops[i];
    return TRUE;
  }
  else {
    return FALSE;
  }
}
void CATListBox::SetFixFont(CFont& fnt, LONG lfHeight)
{
  LOGFONT lf;
  CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT))->GetLogFont(&lf);
  lf.lfHeight = globalData.dpiAware_.Scale(lf.lfHeight);
  if (lfHeight != -1)
  {
    CDC* dc = GetDC();
    if (dc)
    {
      lf.lfHeight = -MulDiv(FONT_POINT_SIZE, dc->GetDeviceCaps(LOGPIXELSY), 96);
      ReleaseDC(dc);

      LOGFONT currentFont;
      GetFont()->GetLogFont(&currentFont);
      currentFont.lfHeight = lf.lfHeight;
      strcpy(currentFont.lfFaceName, lf.lfFaceName);

      fnt.CreateFontIndirect(&currentFont);
      return;
    }
    else
      lf.lfHeight = lfHeight;
  }

  fnt.CreateFontIndirect(&lf);
}
void CATListBox::SetFont(CFont* pFont, BOOL bRedraw)
{
  LOGFONT lf;
  if (pFont && ::IsWindow(m_hWnd)) {
    ::SendMessage(m_hWnd, WM_SETFONT,
                  (WPARAM)pFont->GetSafeHandle(), 0);
  }

  GetFont()->GetLogFont(&lf);

  if (fixfnt.GetSafeHandle())
    fixfnt.DeleteObject();

  SetFixFont(fixfnt, lf.lfHeight);

  if (bRedraw)
    RedrawWindow();
}

HBRUSH CATListBox::CtlColor(CDC* pDC, UINT nCtlColor)
{
  pDC->SetTextColor(pGlobalEditFrame->m_clrSubText);
  pDC->SetBkColor(pGlobalEditFrame->m_clrSubBack);
  return (HBRUSH)(pGlobalEditFrame->m_cbrSubBkBrush.GetSafeHandle());
}

/////////////////////////////////////////////////////////////////////////////
// CAsciiTab dialog

CAsciiTab::CAsciiTab(CWnd* pParent /*=NULL*/)
  : CUEStudioDialog(CAsciiTab::IDD, pParent)
{
  //{{AFX_DATA_INIT(CAsciiTab)
  m_Heading = _T("");
  m_bReFnt = FALSE;
  //}}AFX_DATA_INIT
}


void CAsciiTab::DoDataExchange(CDataExchange* pDX)
{
  CUEStudioDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CAsciiTab)
  DDX_Control(pDX, IDC_ASCLIST, m_AsciiTab);
  DDX_Text(pDX, IDC_HEADING, m_Heading);
  DDX_Check(pDX, IDC_CHECK1, m_bReFnt);
  //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAsciiTab, CUEStudioDialog)
  //{{AFX_MSG_MAP(CAsciiTab)
  ON_WM_CTLCOLOR()
  ON_BN_CLICKED(IDC_FONT, OnFont)
  ON_BN_CLICKED(IDC_INSERTCHAR, OnInsertchar)
  ON_LBN_DBLCLK(IDC_ASCLIST, OnDblclkAsclist)
  ON_BN_CLICKED(IDC_CHECK1, OnReFnt)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

extern BOOL PASCAL ReadWindowPlacement(LPWINDOWPLACEMENT pwp, LPCSTR lpSection, LPCSTR lpSubSection, CINIHandler*);
extern void PASCAL WriteWindowPlacement(LPWINDOWPLACEMENT pwp, LPCSTR lpSection, LPCSTR lpSubSection, CINIHandler*);

/////////////////////////////////////////////////////////////////////////////
// CAsciiTab message handlers

void CAsciiTab::OnReFnt()
{
  UpdateData();
  if (m_bReFnt) {
    AfxGetEditApp()->WriteProfileBinary(_T("Ascii Tab"), szFont,
      (BYTE*) &m_lfDefFont, sizeof(m_lfDefFont));
  } else {
    AfxGetEditApp()->WriteProfileString(_T("Ascii Tab"), szFont, NULL);
  }
}

BOOL CAsciiTab::OnInitDialog()
{
  CUEStudioDialog::OnInitDialog();

  const LCID usLcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
    SORT_DEFAULT);
  if (m_AsciiTab.SetLocale(usLcid) == LB_ERR) {
#ifdef _DEBUG
    MessageBox("LB_ERR");
#endif
  }

  UINT nBytes;
  LPLOGFONT lpLogFont = NULL;
  if (!AfxGetEditApp()->GetProfileBinary(_T("Ascii Tab"), szFont,
    reinterpret_cast<LPBYTE*>(&lpLogFont), &nBytes) || (nBytes != sizeof(m_lfDefFont))) {
    // Set font based on selected font for views
    m_lfDefFont = ueView::m_lfDefFont[0];
    m_bReFnt = FALSE;
  } else if (lpLogFont) {
    memcpy(&m_lfDefFont, lpLogFont, sizeof(m_lfDefFont));
    m_bReFnt = TRUE;
    delete[] lpLogFont;
  }

  FillTable();
  UpdateData(FALSE);

  SetFonts();

  CreateResizeBar(m_hWnd, IDC_STATUSBAR);
  InitAnchors(_T("AsciiTabDialog"), FALSE, TRUE);
  InsAnchor(IDOK, ANCHOR_TOPRIGHT);
  InsAnchor(IDC_FONT, ANCHOR_TOPRIGHT);
  InsAnchor(IDC_INSERTCHAR, ANCHOR_TOPRIGHT);
  InsAnchor(IDC_STATIC_TEXT, ANCHOR_TOPRIGHT);
  InsAnchor(IDC_ASCLIST, ANCHOR_ALL);
  InsAnchor(IDC_HEADING, ANCHOR_TOPLEFT);
  InsAnchor(IDC_STATUSBAR, ANCHOR_BOTTOMRIGHT);

  return TRUE;  // return TRUE unless you set the focus to a control
                // EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT CATListBox::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
  return CATListBoxBase::DefWindowProc(message, wParam, lParam);
}

void CATListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
  LOGFONT lf;
  LONG h1, h2;
  GetFont()->GetLogFont(&lf);
  h1 = abs(lf.lfHeight);
  fixfnt.GetLogFont(&lf);
  h2 = abs(lf.lfHeight);
  lpMeasureItemStruct->itemHeight = max(h1, h2) + globalData.dpiAware_.Scale(4);
}

CRect DPIRECT(CRect& rect)
{
  CRect dpiRect = rect;
  //globalData.dpiAware_.ScaleRect(dpiRect);
  return dpiRect;
}

void CATListBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
  CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
  CString* pText = (CString*) lpDrawItemStruct->itemData;
  CRect  rect = lpDrawItemStruct->rcItem;
  CFont* fnt1 = GetFont();
  CFont* fnt2 = &fixfnt;
  int oldBkMode = pDC->SetBkMode(TRANSPARENT);
  COLORREF oldTextColor = pDC->GetTextColor();
  COLORREF oldBkColor = pDC->GetBkColor();
  CFont* oldFnt = pDC->SelectObject(GetParent()->GetFont());
  if((lpDrawItemStruct->itemAction | ODA_FOCUS) &&
    (lpDrawItemStruct->itemState & ODS_FOCUS)) {
    pDC->FillSolidRect(rect, ::GetSysColor(COLOR_HIGHLIGHT));
    pDC->SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
    pDC->SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
    pDC->DrawFocusRect(rect);
  } else if ((lpDrawItemStruct->itemAction | ODA_SELECT) &&
    (lpDrawItemStruct->itemState & ODS_SELECTED)) {
    pDC->FillSolidRect(rect, ::GetSysColor(COLOR_HIGHLIGHT));
    pDC->SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
    pDC->SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
  } else {
    //pDC->FillSolidRect(rect, ::GetSysColor(COLOR_WINDOW));
    //pDC->SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
    //pDC->SetBkColor(::GetSysColor(COLOR_WINDOW));
    pDC->FillSolidRect(rect, pGlobalEditFrame->m_clrSubBack);
    pDC->SetTextColor(pGlobalEditFrame->m_clrSubText);
    pDC->SetBkColor(pGlobalEditFrame->m_clrSubBack);
  }
  if (pText && *pText) {
    if ((GetStyle() & LBS_USETABSTOPS) != LBS_USETABSTOPS) {
      pDC->DrawText(*pText, DPIRECT(rect), DT_LEFT | DT_VCENTER);
    } else {
      CStringArray sTew;
      int items = StringToArray(*pText, sTew, NULL, NULL, _T('\t'));
      if (items > 1) {
        CArray<int> tabs;
        int i, x = rect.left, xstart = rect.left;
        tabs.SetSize(items);
        for (i = 0; i < sTew.GetCount(); i ++) {
          if (sTabs.GetCount() > i) {
            CRect r(sTabs[i],0,0,0);
            Dlu2Pix(r, TRUE, pDC);
            tabs[i] = r.left;
          } else if (sTabs.GetCount()) {
            tabs[i] = tabs[i-1] + tabs[0];
          } else {
            tabs[i] = 70 * i;
          }
        }
        for (i = 0; i < sTew.GetCount(); i ++) {
          if (i == 0)
          {
            rect.left += globalData.dpiAware_.Scale(2);
            pDC->SelectObject(fnt1);
          }
          else
            pDC->SelectObject(fnt2);
          pDC->DrawText(sTew[i], DPIRECT(rect), DT_LEFT | DT_VCENTER);
          x += pDC->GetTextExtent(sTew[i]).cx;
          if (tabs.GetCount() > i) {
            x = xstart + tabs[i];
          } else if (tabs.GetCount()) {
            x = xstart + ((i+1) * tabs[0]);
          }
          rect.left = x;
        }
      } else {
        pDC->DrawText(*pText, DPIRECT(rect), DT_LEFT | DT_VCENTER);
      }
    }
  }

  pDC->SelectObject(oldFnt);
  pDC->SetBkMode(oldBkMode);
  pDC->SetTextColor(oldTextColor);
  pDC->SetBkColor(oldBkColor);
}

int CATListBox::CharToItem(UINT nKey, UINT nIndex)
{
  return nKey;
}

void CAsciiTab::FillTable()
{
  int i;
  int iStrlen;
  char szNum[34];
  char szString[18];  // Format is char (1)          index 0
                      //           space (1)               1
                      //           tab  (1)                2
                      //           Dec Value (4)           3
                      //           tab  (1)                7
                      //           Hex Value (2)           8
                      //           tab  (1)                10
                      //           Name (3)                11
                      //           tab  (1)                12
                      //           Ctrl+Val (2)            15
                      //           Null                    17
                      // Pre initialise with tabs and null
  szString[1] = ' ';
  szString[2] = '\t';
  szString[7] = '\t';
  szString[10] = '\t';
  szString[13] = '\0';

  std::string sChar;
  for (i = 0; i <= 255; i++) {
    szString[0] = (unsigned char)i;
    if (i == 0)
      szString[0] = ' '; // Can't display null or tab
    if (i == 9)
      szString[0] = ' '; // Can't display null or tab

    _itoa(i, szNum, 10);

    iStrlen = IDM_INT_CAST strlen(szNum);
    switch (iStrlen) {
        case 1:
          szString[3] = '0';
          szString[4] = szNum[0];
          szString[5] = ' ';
          szString[6] = ' ';
          break;
        case 2:
          szString[3] = szNum[0];
          szString[4] = szNum[1];
          szString[5] = ' ';
          szString[6] = ' ';
          break;
        case 3:
          szString[3] = szNum[0];
          szString[4] = szNum[1];
          szString[5] = szNum[2];
          szString[6] = ' ';
          break;
    }

    _itoa(i, szNum, 16);
    iStrlen = IDM_INT_CAST strlen(szNum);
    _strupr(szNum);
    switch (iStrlen) {
        case 1:
          szString[8] = '0';
          szString[9] = szNum[0];
          break;
        case 2:
          szString[8] = szNum[0];
          szString[9] = szNum[1];
          break;
    }

    if ((i <= 32) || (i == 127)) { // Add Name
      szString[11] = cCharName[i][0];
      szString[12] = cCharName[i][1];
      szString[13] = cCharName[i][2];
      if (i == 127)
        strcpy(&szString[11], "DEL");
      szString[14] = '\t';
      if ((i < 32)/* || (i == 126)*/) { // Add ctrl
        szString[15] = '^';
        szString[16] = cCtrlChar[i];
        szString[17] = '\0';
      }
      else {
        szString[14] = '\0';
      }
    }
    else {
      szString[10] = '\0';
    }
    m_AsciiTab.AddString(szString[0] == '&' ? (CString('&') + szString) : szString);
  }
}

void CAsciiTab::SetFonts()
{
  m_AsciiTab.SetTabStops(1, &iTabStop);

  if (m_lfDefFont.lfHeight != 0) {
    m_font.DeleteObject();
    CDC* pDC = GetDC();
    //m_lfDefFont.lfCharSet = ANSI_CHARSET;
    m_lfDefFont.lfHeight = -MulDiv(FONT_POINT_SIZE, GetDeviceCaps(pDC->GetSafeHdc(), LOGPIXELSY), 96);
    ReleaseDC(pDC);
    m_font.CreateFontIndirect(&m_lfDefFont);
    m_AsciiTab.SetFont(&m_font);
    m_Heading = m_lfDefFont.lfFaceName;
  } else {
    m_AsciiTab.SetFont(CFont::FromHandle((HFONT)
      GetStockObject(ANSI_FIXED_FONT)));
    iTabStop = iTabStopOrg;
  }

  UpdateData(FALSE);
}

INT_PTR CAsciiTab::DoModal()
{
  return CUEStudioDialog::DoModal();
}

void CAsciiTab::OnFont()
{
  UpdateData();
  if (m_lfDefFont.lfHeight == 0)
    ::GetObject(GetStockObject(ANSI_FIXED_FONT), sizeof(LOGFONT), &m_lfDefFont);

  CFontDialog dlg(&m_lfDefFont, CF_SCREENFONTS|CF_INITTOLOGFONTSTRUCT);
  if (dlg.DoModal() == IDOK)
  {
    // switch to new font.
    m_font.DeleteObject();

    CDC* pDC = GetDC();
    m_lfDefFont.lfHeight = -MulDiv(FONT_POINT_SIZE, GetDeviceCaps(pDC->GetSafeHdc(), LOGPIXELSY), 96);
    ReleaseDC(pDC);

    if (m_font.CreateFontIndirect(&m_lfDefFont))
    {
      m_AsciiTab.SetFont(&m_font);
      m_Heading = m_lfDefFont.lfFaceName;
    }
    if (m_bReFnt)
      OnReFnt();
  }
  UpdateData(FALSE);
}

void CAsciiTab::OnInsertchar()
{
  int iCharIndex = m_AsciiTab.GetCurSel();
  // Get ActiveView
  if ((iCharIndex >0) && (iCharIndex < 256)) {
    if (((CBCGPMDIFrameWnd*)AfxGetMainWnd())->MDIGetActive() != NULL) {
      ueView* pEditView = (ueView*) ((CBCGPMDIFrameWnd*)AfxGetMainWnd())->MDIGetActive()->GetActiveView();
      if (pEditView != NULL)
        pEditView->InsertLiteralChar((unsigned char) iCharIndex);
    }
  }
}

HBRUSH CAsciiTab::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
  switch (nCtlColor) {

    case CTLCOLOR_LISTBOX:
    case CTLCOLOR_BTN:
    case CTLCOLOR_DLG:
    case CTLCOLOR_EDIT:
    case CTLCOLOR_MSGBOX:
    case CTLCOLOR_SCROLLBAR:
    case CTLCOLOR_STATIC:
      // Set color and return the background brush.
      pDC->SetTextColor(pGlobalEditFrame->m_clrSubText);
      pDC->SetBkColor(pGlobalEditFrame->m_clrSubBack);
      return (HBRUSH)(pGlobalEditFrame->m_cbrSubBkBrush.GetSafeHandle());
    default:
      return CUEStudioDialog::OnCtlColor(pDC, pWnd, nCtlColor);
  }
}

void CAsciiTab::OnOK()
{
  CUEStudioDialog::OnOK();
  m_font.DeleteObject();
  pGlobalEditFrame->m_pAsciiTable = NULL;
  DestroyWindow();
  delete []this;
}

void CAsciiTab::OnCancel()
{
   OnOK();
}

void CAsciiTab::OnDblclkAsclist()
{
  OnInsertchar();
}


BEGIN_MESSAGE_MAP(CAsciiTabWnd, CUEPane)
  //{{AFX_MSG_MAP(CAsciiTabWnd)
  ON_WM_CREATE()
  ON_WM_DESTROY()
  ON_WM_SHOWWINDOW()
  ON_WM_VKEYTOITEM()
  ON_WM_CTLCOLOR()
  ON_WM_LBUTTONDBLCLK()
  ON_WM_LBUTTONDOWN()
  ON_WM_RBUTTONDOWN()
  ON_WM_MOUSEMOVE()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT nBytes;
static CFont m_font;
static LOGFONT NEAR m_lfDefFont;

void InsertChar(CDlgWrk* pDlg)
{
  CListBox* pLst = ((CListBox*)pDlg->GetDlgItem(IDC_ASCLIST));
  int iCharIndex = pLst->GetCurSel();
  // Get ActiveView
  if ((iCharIndex > 0) && (iCharIndex < 256)) {
    if (((CBCGPMDIFrameWnd*)AfxGetMainWnd())->MDIGetActive() != NULL) {
      ueView* pEditView = (ueView*) ((CBCGPMDIFrameWnd*)AfxGetMainWnd())->MDIGetActive()->GetActiveView();
      if (pEditView != NULL) {
        if (!pEditView->m_EditHexMode)
          pEditView->InsertLiteralChar((unsigned char) iCharIndex);
      }
    }
  }
}

void SetFonts(CDlgWrk* pDlg)
{
  int tabstops[6] = {5*4,11*4,17*4,23*4};
  CATListBox* pLst = ((CATListBox*)pDlg->GetDlgItem(IDC_ASCLIST));
  pLst->SetTabStops(countof(tabstops), (LPINT) tabstops);

  if (m_lfDefFont.lfHeight != 0) {
    m_font.DeleteObject();
    CDC* pDC = pDlg->GetDC();
    //m_lfDefFont.lfCharSet = ANSI_CHARSET;
    m_lfDefFont.lfHeight = -MulDiv(FONT_POINT_SIZE, GetDeviceCaps(pDC->GetSafeHdc(), LOGPIXELSY), 96);
    pDlg->ReleaseDC(pDC);
    m_font.CreateFontIndirect(&m_lfDefFont);
    pLst->SetFont(&m_font);
    ((CStatic*)pDlg->GetDlgItem(IDC_HEADING))->SetWindowText(m_lfDefFont.lfFaceName);
  } else {
    pLst->SetFont(CFont::FromHandle((HFONT)GetStockObject(ANSI_FIXED_FONT)));
    ((CStatic*)pDlg->GetDlgItem(IDC_HEADING))->SetWindowText(_T(""));
  }
}

static CATListBox lbw;

BOOL __stdcall OnDefDlgProcAscii(CWnd* pMain, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult)
{
  CDlgWrk* pDlg = (CDlgWrk*) pMain;
  switch (uMsg) {
    case WM_INITDIALOG:
      pDlg->BaseDefProc(uMsg, wParam, lParam);
      if (pDlg->GetSafeHwnd()) {
        CATListBox* pLst = new CATListBox;
        pDlg->m_pDat = (void*) pLst;
        pLst->Attach(pDlg->GetDlgItem(IDC_ASCLIST)->GetSafeHwnd());
        pLst->SetLocale(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT));
        LPLOGFONT lpLogFont = NULL;
        if (!AfxGetEditApp()->GetProfileBinary(_T("Ascii Tab"), szFont,
          reinterpret_cast<LPBYTE*>(&lpLogFont), &nBytes) || (nBytes != sizeof(m_lfDefFont))) {
            // Set font based on selected font for views
            m_lfDefFont = ueView::m_lfDefFont[0];
            ((CButton*)pDlg->GetDlgItem(IDC_CHECK1))->SetCheck(0);
        } else if (lpLogFont) {
          memcpy(&m_lfDefFont, lpLogFont, sizeof(m_lfDefFont));
          delete[] lpLogFont;
          ((CButton*)pDlg->GetDlgItem(IDC_CHECK1))->SetCheck(1);
        }
        SetFonts(pDlg);
        int i;
        int iStrlen;
        char szNum[34];
        char szString[18];  // Format is char (1)          index 0
        //           space (1)               1
        //           tab  (1)                2
        //           Dec Value (4)           3
        //           tab  (1)                7
        //           Hex Value (2)           8
        //           tab  (1)                10
        //           Name (3)                11
        //           tab  (1)                12
        //           Ctrl+Val (2)            15
        //           Null                    17
        // Pre initialise with tabs and null
        szString[1] = ' ';
        szString[2] = '\t';
        szString[7] = '\t';
        szString[10] = '\t';
        szString[13] = '\0';

        std::string sChar;
        for (i = 0; i <= 255; i++) {
          szString[0] = (unsigned char)i;
          if (i == 0)
            szString[0] = ' '; // Can't display null or tab
          if (i == 9)
            szString[0] = ' '; // Can't display null or tab

          _itoa(i, szNum, 10);
          iStrlen = IDM_INT_CAST strlen(szNum);
          switch (iStrlen) {
            case 1:
              szString[3] = '0';
              szString[4] = szNum[0];
              szString[5] = ' ';
              szString[6] = ' ';
              break;
            case 2:
              szString[3] = szNum[0];
              szString[4] = szNum[1];
              szString[5] = ' ';
              szString[6] = ' ';
              break;
            case 3:
              szString[3] = szNum[0];
              szString[4] = szNum[1];
              szString[5] = szNum[2];
              szString[6] = ' ';
              break;
          }

          _itoa(i, szNum, 16);
          iStrlen = IDM_INT_CAST strlen(szNum);
          _strupr(szNum);
          switch (iStrlen) {
            case 1:
              szString[8] = '0';
              szString[9] = szNum[0];
              break;
            case 2:
              szString[8] = szNum[0];
              szString[9] = szNum[1];
              break;
          }

          if ((i <= 32) || (i == 127)) { // Add Name
            szString[11] = cCharName[i][0];
            szString[12] = cCharName[i][1];
            szString[13] = cCharName[i][2];
            if (i == 127)
              strcpy(&szString[11], "DEL");
            szString[14] = '\t';
            if ((i < 32)/* || (i == 126)*/) { // Add ctrl
              szString[15] = '^';
              szString[16] = cCtrlChar[i];
              szString[17] = '\0';
            }
            else {
              szString[14] = '\0';
            }
          }
          else {
            szString[10] = '\0';
          }
          pLst->AddString(szString[0] == '&' ? (CString('&') + szString) : szString);
        }
      }
      return TRUE;
    case WM_COMMAND:
      switch (LOWORD(wParam)) {
      case IDC_INSERTCHAR:
        InsertChar(pDlg);
        break;
      case IDC_FONT:
        {
          if (m_lfDefFont.lfHeight == 0)
            ::GetObject(GetStockObject(ANSI_FIXED_FONT), sizeof(LOGFONT), &m_lfDefFont);

          CFontDialog dlg(&m_lfDefFont, CF_SCREENFONTS|CF_INITTOLOGFONTSTRUCT);
          if (dlg.DoModal() == IDOK) {
            // switch to new font.
            m_font.DeleteObject();

            CDC* pDC = pDlg->GetDC();
            m_lfDefFont.lfHeight = -MulDiv(FONT_POINT_SIZE, GetDeviceCaps(pDC->GetSafeHdc(), LOGPIXELSY), 96);
            pDlg->ReleaseDC(pDC);

            if (m_font.CreateFontIndirect(&m_lfDefFont)) {
              ((CListBox*)pDlg->GetDlgItem(IDC_ASCLIST))->SetFont(&m_font);
              ((CStatic*)pDlg->GetDlgItem(IDC_HEADING))->SetWindowText(m_lfDefFont.lfFaceName);
              if (((CButton*)pDlg->GetDlgItem(IDC_CHECK1))->GetCheck()) {
                theApp.WriteProfileBinary(_T("Ascii Tab"), szFont,
                  (BYTE*) &m_lfDefFont, sizeof(m_lfDefFont));
              }
            } else {
              ((CStatic*)pDlg->GetDlgItem(IDC_HEADING))->SetWindowText(_T(""));
            }
          }
        }
        break;
      case IDC_CHECK1:
        if (((CButton*)pDlg->GetDlgItem(IDC_CHECK1))->GetCheck()) {
          AfxGetEditApp()->WriteProfileBinary(_T("Ascii Tab"), szFont,
            (BYTE*) &m_lfDefFont, sizeof(m_lfDefFont));
        } else {
          AfxGetEditApp()->WriteProfileString(_T("Ascii Tab"), szFont, NULL);
        }
        break;
      case IDC_ASCLIST:
        switch (HIWORD(wParam)) {
        case LBN_DBLCLK:
          InsertChar(pDlg);
          break;
        }
        break;
      }
      break;
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORDLG:
      {
        HDC hDC = (HDC) wParam;
        ::SetTextColor(hDC, pGlobalEditFrame->m_clrSubText);
        ::SetBkColor(hDC, pGlobalEditFrame->m_clrSubBack);
        lResult = (LRESULT) (HBRUSH) pGlobalEditFrame->m_cbrSubBkBrush;
      }
      return TRUE;
    case WM_DESTROY:
      if (pDlg->m_pDat) {
        if (((CButton*)pDlg->GetDlgItem(IDC_CHECK1))->GetCheck()) {
          AfxGetEditApp()->WriteProfileBinary(_T("Ascii Tab"), szFont,
            (BYTE*) &m_lfDefFont, sizeof(m_lfDefFont));
        }
        ((CATListBox*)(pDlg->m_pDat))->Detach();
        delete ((CATListBox*)(pDlg->m_pDat));
      }
      break;
  }
  return FALSE;
}


int CAsciiTabWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  CEditApp* pApp = AfxGetEditApp();
  if (CUEPane::OnCreate(lpCreateStruct) == -1)
    return -1;

  //SetSCBStyle(GetSCBStyle() | SCBS_SHOWEDGES | SCBS_SIZECHILD);

  // Create controls
  CRect rect;

  m_wndAscii.m_dfProc = OnDefDlgProcAscii;
  m_wndAscii.Create(IDD_ASCIITAB2, NULL);
  m_wndAscii.InitAnchors();
  m_wndAscii.InsAnchor(IDC_HEADING, _T("tlr"));
  m_wndAscii.InsAnchor(IDC_ASCIITAB_ST_CHAR, _T("tl"));
  m_wndAscii.InsAnchor(IDC_ASCIITAB_ST_DEC, _T("tl"));
  m_wndAscii.InsAnchor(IDC_ASCIITAB_ST_HEX, _T("tl"));
  m_wndAscii.InsAnchor(IDC_ASCIITAB_ST_NAME, _T("tl"));
  m_wndAscii.InsAnchor(IDC_ASCIITAB_ST_CTRL, _T("tl"));
  m_wndAscii.InsAnchor(IDC_ASCLIST, _T("tlrb"));
  m_wndAscii.InsAnchor(IDC_FONT, _T("lb"));
  m_wndAscii.InsAnchor(IDC_CHECK1, _T("bl"));
  m_wndAscii.InsAnchor(IDC_INSERTCHAR, _T("br"));
  m_wndAscii.InsAnchor(IDC_ASCIITAB_ST_REMFNT, _T("bl"));

  m_wndAscii.SetParent(this);
#ifdef USE_THEMES
  m_btnFont.SubclassWindow(m_wndAscii.GetDlgItem(IDC_FONT)->GetSafeHwnd());
  m_btnInsert.SubclassWindow(m_wndAscii.GetDlgItem(IDC_INSERTCHAR)->GetSafeHwnd());
#endif
  m_wndAscii.ShowWindow(SW_SHOWNORMAL);

#ifdef USE_DETOURS
  HWND hWnd = m_wndAscii.GetDlgItem(IDC_ASCLIST)->m_hWnd;
  InitializeCoolSB(hWnd);
  CoolSB_SetStyle(hWnd, SB_BOTH, CSBS_HOTTRACKED);
#endif

  return TRUE;
}


HBRUSH CAsciiTabWnd::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
  switch (nCtlColor) {
    case CTLCOLOR_LISTBOX:
    case CTLCOLOR_BTN:
    case CTLCOLOR_DLG:
    case CTLCOLOR_EDIT:
    case CTLCOLOR_MSGBOX:
    case CTLCOLOR_SCROLLBAR:
    case CTLCOLOR_STATIC:
      // Set color and return the background brush.
      pDC->SetTextColor(pGlobalEditFrame->m_clrSubText);
      pDC->SetBkColor(pGlobalEditFrame->m_clrSubBack);
      return (HBRUSH)(pGlobalEditFrame->m_cbrSubBkBrush.GetSafeHandle());
    default:
      return CUEPane::OnCtlColor(pDC, pWnd, nCtlColor);
  }
}

void CAsciiTabWnd::OnDestroy()
{
  CUEPane::OnDestroy();
}

void CAsciiTabWnd::OnShowWindow(BOOL bShow, UINT nStatus)
{
  if (bShow) {
    if (!InSlide()) {
      m_wndAscii.SetFocus();
    }
  }
}

void CAsciiTabWnd::SetFont(LOGFONT& logFont)
{
  if (((CButton*)m_wndAscii.GetDlgItem(IDC_CHECK1))->GetCheck() == 0) {
    memcpy(&m_lfDefFont, &logFont, sizeof(m_lfDefFont));
    SetFonts(&m_wndAscii);
  }
}
