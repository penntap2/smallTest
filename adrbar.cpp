// ----------------------------------------------------------------------------
// Script Name: 
// Creation Date: 
// Last Modified: 
// Copyright (c)2007
// Purpose: 
// ----------------------------------------------------------------------------
I made a change!
#include "stdafx.h"
#include "UEStdafx.h"
#include "adrbar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(COpenAdrBar, CUEPane)

/////////////////////////////////////////////////////////////////////////////
// CPrjFrame

BEGIN_MESSAGE_MAP(COpenAdrBar, CUEPane)
  ON_CBN_DBLCLK(ID_OPEN_ADRBOX, OnBoxDoubleClick)
  ON_WM_CREATE()
  ON_WM_SIZE()
END_MESSAGE_MAP()

// My new change is here!

COpenAdrBar::COpenAdrBar()
{

}

COpenAdrBar::~COpenAdrBar()
{
}

int COpenAdrBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (CUEPane::OnCreate(lpCreateStruct) == -1)
    return -1;
  
  if (m_cBox.Create(WS_VISIBLE | WS_CHILD | CBS_SIMPLE | CBS_NOINTEGRALHEIGHT, CRect(0,0,0,0), this, ID_OPEN_ADRBOX)) {
    m_cBox.SendMessage(WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT));
    if (SHAutoComplete(FindChildWindow(m_cBox.m_hWnd, _T("Edit"), NULL),
      SHACF_AUTOAPPEND_FORCE_ON | SHACF_AUTOSUGGEST_FORCE_ON | SHACF_FILESYSTEM) != S_OK) {
        // auto complete is not set
    }
    CRect rect;
    CWnd* wLbox = CWnd::FindWindowEx(m_cBox.m_hWnd, NULL,
      _T("ComboLBox"), NULL);
    if (wLbox->GetSafeHwnd()) {
      wLbox->GetWindowRect(rect);
      m_cBox.ScreenToClient(rect);
      CSize szOpenAdr(200, rect.top);
      SetMinSize(szOpenAdr);
    }
  }

  return 0;
}

void COpenAdrBar::OnSize(UINT nType, int cx, int cy)
{
  CUEPane::OnSize(nType, cx, cy);
}
