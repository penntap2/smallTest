// ----------------------------------------------------------------------------
// Script Name: 
// Creation Date: 
// Last Modified: 
// Copyright (c)2007
// Purpose: 
// ----------------------------------------------------------------------------
I made a change!
// ASpellCheck.cpp: implementation of the CASpellCheck class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "ASpellCheck.h"
#include "ASpellCheckDlg.h"
#include "edit.h"
#include "hexfind.h"
#include "undo.h"
#include "editdoc.h"
#include "editmain.h"
#include "editview.h"
#include "SyntaxHighlighting.h"
#include "unicode\utypes.h"
#include "unicode\utext.h"

//#include "editcon.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// defines/variables
//////////////////////////////////////////////////////////////////////

#define GNU_ASPELL _T("GNU\\Aspell\\")

extern int bUseHTMLParsing[];
//extern int iSyntaxLangType[NUM_LANGUAGES];

//////////////////////////////////////////////////////////////////////
// auto remove ignore lists at the end of ue(s) session
//////////////////////////////////////////////////////////////////////

class CAutoRemoveIgnoreLists {
public:
  CAutoRemoveIgnoreLists() {
  }
  ~CAutoRemoveIgnoreLists() {
    if (!sPath.IsEmpty()) {
      CSetPath sp;
      if (sp.Set(sPath)) {
        for (int i=0; i<sList.GetSize(); i++) {
          DeleteFile(sList[i]);
        }
        sList.RemoveAll();
      }
    }
  }
  CStringArray sList;
  CString sPath;
};

static CAutoRemoveIgnoreLists alst;

//////////////////////////////////////////////////////////////////////
// macros
//////////////////////////////////////////////////////////////////////

#define checkit(new_aspell_config)  \
  if (new_aspell_config==NULL) {  \
  pParentWnd->MessageBox((CString) GetErrorString() + _T("\nPlease install latest version of Aspell for Windows"), FormatString(IDS_SPELLCHECKER), MB_ICONERROR); \
  if (FreeLibrary(LibAspell)) { \
    LibAspell = NULL; \
  } \
  return FALSE; \
  }

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CASpellCheck::CASpellCheck()
{
  Reset();
}

CASpellCheck::~CASpellCheck()
{
  Done();
}

void CASpellCheck::Reset()
{
  bNeedSaveIL = FALSE;
  pParentWnd = AfxGetEditMainWnd();
  m_bRefresh = FALSE;
  dwAddedWords = 0;
  LibAspell = NULL;
  m_bCancel = FALSE;
  speller = NULL;
  checker = NULL;
  nFilter = 0;
  dataDictDir_ = _T("");
}

BOOL CASpellCheck::ReloadOptions()
{
  ueView *pUEView = NULL;
  if (pParentWnd->IsKindOf(RUNTIME_CLASS(ueView)))
    pUEView = (ueView*)pParentWnd;

  if (speller) {
    if (checker) {
      if (delete_aspell_document_checker)
        delete_aspell_document_checker(checker);
      checker = NULL;
    }
    if (dwAddedWords && aspell_speller_save_all_word_lists)
      aspell_speller_save_all_word_lists(speller);
    if (delete_aspell_speller)
      delete_aspell_speller(speller);
    speller = NULL;
  }

  SaveIgnoreList();

  // we need to create new config
  config  = new_aspell_config();

  if (dataDictDir_.IsEmpty()) {
    dataDictDir_ = AfxGetEditApp()->GetProfileString(szSpellChecker, szSpellDataDictDir, _T(""));

    if (dataDictDir_.IsEmpty()) {
#ifdef ULTRAEDITPORTABLE
      CString appPath = idm_util::GetUEPAppPath();
      appPath + _T("GNU\\Aspell\\");
      dataDictDir_ = appPath;
#else
      GetModuleFileName(NULL, dataDictDir_.GetBuffer(MAX_PATH+1), MAX_PATH);
      dataDictDir_.ReleaseBuffer();
      int index = dataDictDir_.ReverseFind('\\');
      //index++;
      if (index != -1/* && index < dataDictDir_.GetLength()*/)
        dataDictDir_.Delete(index, dataDictDir_.GetLength() - index);
#endif
    }
  }
  //if (dataDictDir_[dataDictDir_.GetLength()-1] != '\\')
  //  dataDictDir_ += "\\";
  aspell_config_replace(config, "prefix", dataDictDir_);

  // find dictionaries and fill dictlist
  dlist   = get_aspell_dict_info_list(config);
  dels    = aspell_dict_info_list_elements(dlist);
  while ((entry = aspell_dict_info_enumeration_next(dels)) != 0)
    pDictList.Add(entry->name);
  delete_aspell_dict_info_enumeration(dels);

  // get previous settings from registry
  szSuggestionMode  = AfxGetEditApp()->GetProfileString(szSpellChecker, _T("SuggestionMode"), _T("normal"));
  szDictionary = AfxGetEditApp()->GetProfileString(szSpellChecker, _T("Dictionary"));
  if (szDictionary.IsEmpty() && theApp.GetResourceLanguage() == 0x416)
  {
    szDictionary = _T("pt_BR");
  }

  // If the view contains a unicode file, set the encoding to UTF, 
  // otherwise set the encoding to the code page specified in their configuration
  if (pUEView && pUEView->m_bUnicode) {
    szEncoding = "utf-8";
  }
  else {
    szEncoding = AfxGetEditApp()->GetProfileString(szSpellChecker, _T("Encoding"), _T("iso8859-1"));
  }


  szHomeDirectory   = AfxGetEditApp()->GetProfileString(szSpellChecker, szSpellHomeDir, idm_util::GetDataPath());
#ifdef ULTRAEDITPORTABLE
  //AfxGetEditApp()->adjustUEPIniKeyForPaths(szSpellChecker, szSpellHomeDir, szHomeDirectory);
#endif
  nIgnore           = AfxGetEditApp()->GetProfileInt(szSpellChecker, _T("Ignore"), 0);
  bIgnoreAccent     = AfxGetEditApp()->GetProfileInt(szSpellChecker, _T("Accents"), 0);
  bIgnoreCase       = AfxGetEditApp()->GetProfileInt(szSpellChecker, _T("Case"), 0);
  bIgnoreAllCaps    = AfxGetEditApp()->GetProfileInt(szSpellChecker, _T("AllCaps"), 0);
  bIgnoreCapitalized  = AfxGetEditApp()->GetProfileInt(szSpellChecker, _T("Capitalized"), 0);
  bIgnoreWordWithNumbers  = AfxGetEditApp()->GetProfileInt(szSpellChecker, _T("WordWithNumbers"), 0);
  bIgnoreWordWithMixedCase  = AfxGetEditApp()->GetProfileInt(szSpellChecker, _T("WordWithMixedCase"), 0);
  bUseFilter        = AfxGetEditApp()->GetProfileInt(szSpellChecker, _T("UseFilter"), 0);
  if (bUseFilter)
    nFilter = AfxGetEditApp()->GetProfileInt(szSpellChecker, szFilter, 3);

  if (nIgnore<0) nIgnore = 0;
  if (nIgnore>9) nIgnore = 9;
  // modes `none', `url', `email', `sgml', `ccpp', `tex'
  if (m_bInitForSpellChk && bUseFilter) {
    CString cTmp = ".url";
    if (nFilter == 0) {
      if (pUEView)
      {
        int iFileType = ((ueDocument*)pUEView->GetDocument())->m_iSyntaxFileType;
        if (iFileType >= 0) {
          CSyntaxLanguage synHiLang = CSyntaxHighlighting::Instance().GetLanguage(iFileType);
          int32 langType = synHiLang.GetSyntaxLangType();
          if (synHiLang.IsInHTMLFamily() == TRUE) {
            cTmp = "html";
          } else if (langType == CSyntaxHighlighting::LANG_TYPE_LATEX) {
            cTmp = ".tex";
          } else if ((langType == CSyntaxHighlighting::LANG_TYPE_C) ||
            (langType == CSyntaxHighlighting::LANG_TYPE_CS)) {
            cTmp = ".ccpp";  // context
          }
        }
      }
    } else {
      switch (nFilter) {
      case 1:
        cTmp = ".email"; // mode
        break;
      case 2:
        cTmp = "html";
        break;
      case 3:
        cTmp = ".sgml";  // mode
        break;
      case 4:
        cTmp = ".url";   // mode
        break;
      case 5:
        cTmp = "nroff";
        break;
      case 6:
        cTmp = ".tex";   // mode
        break;
      case 7:
        cTmp = ".ccpp";  // mode
        break;
      default:
        cTmp = ".url";
      }
    }
#ifdef MODESUPPORT
    if (StartWith(cTmp) == _T('.')) {
      cTmp.TrimLeft(_T('.'));
      aspell_config_replace(config, "mode", cTmp);
    } else {
      aspell_config_replace(config, "mode", "none");
      aspell_config_replace(config, "filter", cTmp);
    }
#else
    cTmp.TrimLeft('.');
    if (cTmp == "ccpp")
      cTmp = "context";
    aspell_config_replace(config, "filter", cTmp);
#endif
    if (cTmp.Compare(_T("tex")) == 0)
      aspell_config_replace(config, "f-tex-check-comments", "true");
  }

  // setting _config_ values
  if (!szDictionary.IsEmpty())
    aspell_config_replace(config, "lang", szDictionary);

  if (nIgnore)
    aspell_config_replace(config, "ignore", FormatString(_T("%d"), nIgnore));

  if (!szEncoding.IsEmpty())
    aspell_config_replace(config, "encoding", szEncoding);

  if (!szHomeDirectory.IsEmpty())
    aspell_config_replace(config, "home-dir", szHomeDirectory);

  if (!szSuggestionMode.IsEmpty())
    aspell_config_replace(config, "sug-mode", szSuggestionMode);

  aspell_config_replace(config, "ignore-accents", tf(bIgnoreAccent));
  aspell_config_replace(config, "ignore-case", tf(bIgnoreCase));

  ret = new_aspell_speller(config);

  delete_aspell_config(config);

  if (aspell_error(ret) != 0) {
    //if (m_bInitForSpellChk)
    //  pParentWnd->MessageBox(aspell_error_message(ret), FormatString(IDS_SPELLCHECKER), MB_ICONERROR);
    CString sErrorMessage = aspell_error_message(ret);
    delete_aspell_can_have_error(ret);
    return FALSE;
  }

  speller = to_aspell_speller(ret);

  if (aspell_error(ret) != 0) {
    pParentWnd->MessageBox(aspell_error_message(ret), FormatString(IDS_SPELLCHECKER), MB_ICONERROR);
    delete_aspell_can_have_error(ret);
    return FALSE;
  }

  config = aspell_speller_config(speller);

  AspellCanHaveError *pst_ache = new_aspell_document_checker(speller);
  if (aspell_error(pst_ache) != 0) {
    pParentWnd->MessageBox(aspell_error_message(pst_ache), FormatString(IDS_SPELLCHECKER), MB_ICONERROR);
    return FALSE;
  }
  checker = to_aspell_document_checker(pst_ache);

  LoadIgnoreList();

  return speller!=NULL;
}

BOOL CASpellCheck::Init(CWnd* pParentWindow, bool bInitForSpellChk)
{
  if (LibAspell)
    return FALSE; // spell checker was initialized

  if (pParentWindow)
    pParentWnd = pParentWindow;

  m_bInitForSpellChk = bInitForSpellChk;

  CString szASpell = GetASpell();
  TRACE(szASpell);
  if (szASpell.IsEmpty()) {
    LibAspell = NULL;
    SetLastError(ERROR_MOD_NOT_FOUND);
  } else {
    LibAspell = LoadLibrary(szASpell);
  }
  if (LibAspell) {  // if library exists
    new_aspell_config =
      (struct AspellConfig* (__cdecl *)())
      GetProcAddress(LibAspell, _T("new_aspell_config"));
    checkit(new_aspell_config);

    delete_aspell_config =
      (void (__cdecl *)(struct AspellConfig*))
      GetProcAddress(LibAspell, _T("delete_aspell_config"));
    checkit(delete_aspell_config);

    aspell_config_clone =
      (struct AspellConfig* (__cdecl *)(const struct AspellConfig*))
      GetProcAddress(LibAspell, _T("aspell_config_clone"));
    checkit(aspell_config_clone);

    aspell_config_have =
      (int (__cdecl*)(const struct AspellConfig*, const char*))
      GetProcAddress(LibAspell, _T("aspell_config_have"));
    checkit(aspell_config_have);

    aspell_config_remove =
      (int (__cdecl*)(struct AspellConfig*, const char*))
      GetProcAddress(LibAspell, _T("aspell_config_remove"));
    checkit(aspell_config_remove);

    aspell_config_replace =
      (int (__cdecl *)(struct AspellConfig*, const char*, const char*))
      GetProcAddress(LibAspell, _T("aspell_config_replace"));
    checkit(aspell_config_replace);

    aspell_config_retrieve =
      (const char* (__cdecl *)(struct AspellConfig *, const char *))
      GetProcAddress(LibAspell, _T("aspell_config_retrieve"));
    checkit(aspell_config_retrieve);

    get_aspell_dict_info_list =
      (struct AspellDictInfoList* (__cdecl*)(struct AspellConfig*))
      GetProcAddress(LibAspell, _T("get_aspell_dict_info_list"));
    checkit(get_aspell_dict_info_list);

    aspell_dict_info_list_elements =
      (struct AspellDictInfoEnumeration* (__cdecl *)(const struct AspellDictInfoList*))
      GetProcAddress(LibAspell, _T("aspell_dict_info_list_elements"));
    checkit(aspell_dict_info_list_elements);

    aspell_dict_info_enumeration_next =
      (const struct AspellDictInfo* (__cdecl*)(struct AspellDictInfoEnumeration*))
      GetProcAddress(LibAspell, _T("aspell_dict_info_enumeration_next"));
    checkit(aspell_dict_info_enumeration_next);

    delete_aspell_dict_info_enumeration =
      (void (__cdecl*)(struct AspellDictInfoEnumeration*))
      GetProcAddress(LibAspell, _T("delete_aspell_dict_info_enumeration"));
    checkit(delete_aspell_dict_info_enumeration);

    aspell_error =
      (const struct AspellError* (__cdecl *)(const struct AspellCanHaveError*))
      GetProcAddress(LibAspell, _T("aspell_error"));
    checkit(aspell_error);

    aspell_error_message =
      (const char* (__cdecl *)(const struct AspellCanHaveError*))
      GetProcAddress(LibAspell, _T("aspell_error_message"));
    checkit(aspell_error_message);

    new_aspell_speller =
      (struct AspellCanHaveError* (__cdecl *)(struct AspellConfig*))
      GetProcAddress(LibAspell, _T("new_aspell_speller"));
    checkit(new_aspell_speller);

    delete_aspell_speller =
      (void (__cdecl *)(struct AspellSpeller*))
      GetProcAddress(LibAspell, _T("delete_aspell_speller"));
    checkit(delete_aspell_speller);

    to_aspell_speller =
      (struct AspellSpeller* (__cdecl *)(struct AspellCanHaveError*))
      GetProcAddress(LibAspell, _T("to_aspell_speller"));
    checkit(to_aspell_speller);

    aspell_speller_add_to_personal =
      (int (__cdecl *)(struct AspellSpeller*, const char *, int))
      GetProcAddress(LibAspell, _T("aspell_speller_add_to_personal"));
    checkit(aspell_speller_add_to_personal);

    aspell_speller_check =
      (int (__cdecl *)(struct AspellSpeller*, const char*, int))
      GetProcAddress(LibAspell, _T("aspell_speller_check"));
    checkit(aspell_speller_check);

    aspell_speller_config =
      (struct AspellConfig* (__cdecl *)(struct AspellSpeller*))
      GetProcAddress(LibAspell, _T("aspell_speller_config"));
    checkit(aspell_speller_config);

    aspell_speller_error =
      (const struct AspellError * (__cdecl *)(const struct AspellSpeller*))
      GetProcAddress(LibAspell, _T("aspell_speller_error"));
    checkit(aspell_speller_error);

    aspell_speller_error_message =
      (const char* (__cdecl *)(const struct AspellSpeller*))
      GetProcAddress(LibAspell, _T("aspell_speller_error_message"));
    checkit(aspell_speller_error_message);

    aspell_speller_suggest =
      (const struct AspellWordList * (__cdecl *)(struct AspellSpeller*, const char*, int))
      GetProcAddress(LibAspell, _T("aspell_speller_suggest"));
    checkit(aspell_speller_suggest);

    aspell_speller_save_all_word_lists =
      (int (__cdecl *)(struct AspellSpeller*))
      GetProcAddress(LibAspell, _T("aspell_speller_save_all_word_lists"));
    checkit(aspell_speller_save_all_word_lists);

    delete_aspell_can_have_error =
      (void (__cdecl *)(struct AspellCanHaveError*))
      GetProcAddress(LibAspell, _T("delete_aspell_can_have_error"));
    checkit(delete_aspell_can_have_error);

    delete_aspell_string_enumeration =
      (void (__cdecl*)(struct AspellStringEnumeration*))
      GetProcAddress(LibAspell, _T("delete_aspell_string_enumeration"));
    checkit(delete_aspell_string_enumeration);

    aspell_word_list_elements =
      (struct AspellStringEnumeration* (__cdecl *)(const struct AspellWordList*))
      GetProcAddress(LibAspell, _T("aspell_word_list_elements"));
    checkit(aspell_word_list_elements);

    aspell_string_enumeration_next =
      (const char* (__cdecl *)(struct AspellStringEnumeration*))
      GetProcAddress(LibAspell, _T("aspell_string_enumeration_next"));
    checkit(aspell_string_enumeration_next);

    aspell_document_checker_process =
      (void (__cdecl*)(struct AspellDocumentChecker * ths, const char * str, int size))
      GetProcAddress(LibAspell, _T("aspell_document_checker_process"));
    checkit(aspell_document_checker_process);

    new_aspell_document_checker =
      (struct AspellCanHaveError * (__cdecl*)(struct AspellSpeller * speller))
      GetProcAddress(LibAspell, _T("new_aspell_document_checker"));
    checkit(new_aspell_document_checker);

    to_aspell_document_checker =
      (struct AspellDocumentChecker * (__cdecl*)(struct AspellCanHaveError * obj))
      GetProcAddress(LibAspell, _T("to_aspell_document_checker"));
    checkit(to_aspell_document_checker);

    delete_aspell_document_checker =
      (void (__cdecl* )(struct AspellDocumentChecker * ths))
      GetProcAddress(LibAspell, _T("delete_aspell_document_checker"));
    checkit(delete_aspell_document_checker);

    aspell_document_checker_next_misspelling =
      (struct AspellToken (__cdecl*)(struct AspellDocumentChecker * ths))
      GetProcAddress(LibAspell, _T("aspell_document_checker_next_misspelling"));
    checkit(aspell_document_checker_next_misspelling);

    aspell_config_get_default =
      (const char* (__cdecl*)(struct AspellConfig * ths, const char * key))
      GetProcAddress(LibAspell, _T("aspell_config_get_default"));
    checkit(aspell_config_get_default);

    aspell_document_checker_reset =
      (void (__cdecl*)(struct AspellDocumentChecker * ths))
      GetProcAddress(LibAspell, _T("aspell_document_checker_reset"));
    checkit(aspell_document_checker_reset);
/*
    aspell_document_checker_filter =
      (struct AspellFilter * (__cdecl*)(struct AspellDocumentChecker * ths))
      GetProcAddress(LibAspell, _T("aspell_document_checker_filter"));
    checkit(aspell_document_checker_filter);

    aspell_filter_error =
      (const struct AspellError * (__cdecl*)(const struct AspellFilter * ths))
      GetProcAddress(LibAspell, _T("aspell_filter_error"));
    checkit(aspell_filter_error);

    aspell_filter_error_message =
      (const char * (__cdecl*)(const struct AspellFilter * ths))
      GetProcAddress(LibAspell, _T("aspell_filter_error_message"));
    checkit(aspell_filter_error_message);
*/

    ReloadOptions();

  } else {
    pParentWnd->MessageBox(GetErrorString(), FormatString(IDS_SPELLCHECKER));
    TRACE(_T("Could not load the aspell dll!\r\n"));
    return FALSE;
  }

 return TRUE;
}

typedef struct tagSCSW {
  ueView* pView;
  __int64 lCharPos;
  __int64 lTopLineFilePos;
  int iWordLen;
  // for mark routine
  BOOL bMark;
}
SCSW;

#define     EME_REPLACESEL     (WM_USER + 0x100 + 6)

#if _MSC_VER >= 1800
int CASpellCheck::Check(LPVOID lpBuffer, DWORD dwSize, RBDI pRunBeforeDlgInitFunction, 
                        LPVOID lpArgument, int32 *offsetData)
#else
int CASpellCheck::Check(LPVOID lpBuffer, DWORD dwSize, RBDI pRunBeforeDlgInitFunction, 
                        LPVOID lpArgument, int32_t *offsetData)
#endif
{
  if (!speller || !checker)
    return NULL;

  __int64 lLineStartPos = ((SCSW*)lpArgument)->lCharPos;
refresh:
  SCSW* scsw = (SCSW*) lpArgument;
  ueView* pEditView = scsw->pView;
  bool bUnicode = false;
  
  if (pEditView && pEditView->m_bUnicode)
    bUnicode = true;

  scsw->lCharPos = lLineStartPos;

  char *curline;
  curline = new char[dwSize+3];
  ZeroMemory(curline, sizeof(char)*(dwSize+3));
  _snprintf(curline, dwSize+2, "%s", lpBuffer);

  aspell_document_checker_process(checker, curline, dwSize+3);

  int diff = 0;
  int lenDelta = 0;
  int lenDeltaRet = 0;

  int nInc = bUnicode ? sizeof(WCHAR) : sizeof(char);

  __int64 lCharPos = scsw->lCharPos;

  // Now find the misspellings in the line
  AspellToken token;
  while (token = aspell_document_checker_next_misspelling(checker),
    token.offset < dwSize && token.len > 0)
  {
    char *curword = new char[token.len+1];
    ZeroMemory(curword, sizeof(char)*(token.len+1));
    _snprintf(curword, token.len, "%s", curline + token.offset);

    if (bUnicode)
    {
      // determine start/end of source word based on data from offset map
      scsw->iWordLen = offsetData[token.offset + token.len] - offsetData[token.offset];
      scsw->lCharPos = lCharPos + lenDeltaRet + offsetData[token.offset] * nInc; // byte offset in original UTF16 buffer
    }
    else
    {
      scsw->iWordLen = token.len;
      scsw->lCharPos = lCharPos + lenDeltaRet + token.offset;
    }

    int nRetCode;

    LPCTSTR lpszNewWord = ChangeWord(curword, pRunBeforeDlgInitFunction, lpArgument, &nRetCode, bUnicode);
    if (nRetCode == IDC_ADD || nRetCode == IDC_IGNORE || nRetCode == IDC_IGNOREALL) {
      int lStart, lEnd;
      ECGetSel(pEditView->GetEditCtrl(), &lStart, &lEnd);
      ECRemoveSpellTarget(pEditView->GetEditCtrl(), lStart, lEnd);
    }
    if (!lpszNewWord && m_bRefresh) {
      m_bRefresh = false;
      if (curword)
        delete curword;
      if (curline)
        delete curline;
      goto refresh;
    }
    if (m_bCancel) {
      if (curword)
        delete curword;
      if (curline)
        delete curline;
      return NULL;
    }
    if (lpszNewWord != NULL) {

      if(!(ueDocument*)(pEditView->GetDocument())->GetReadOnly()) {
        int lStart, lEnd;
        // JP - FIXME - For undo this should be fine for now since no word
        // should be bigger than the edit control
        ECGetSel(pEditView->GetEditCtrl(), &lStart, &lEnd);
        ECRemoveSpellTarget(pEditView->GetEditCtrl(), lStart, lEnd);
        if (bUnicode) {
          USES_CONVERSION;

          // convert the UTF-8 spell checked work back to UTF16 so it can be replaced in source buffer
          WCHAR * lpszReplace = GetW(lpszNewWord);
          
          int iReplStrLen = IDM_INT_CAST wcslen(lpszReplace);
          pEditView->AddUndoReplace(scsw->lCharPos, scsw->lCharPos +
            scsw->iWordLen * sizeof(WCHAR), 0, -1, (LPCSTR)lpszReplace,
            iReplStrLen * sizeof(WCHAR));
          ::SendMessage(pEditView->GetEditCtrl().m_hWnd, EME_REPLACESEL, 1, (LPARAM)lpszReplace);

          if (lpszReplace)
            delete lpszReplace;

          // if the changed word is a different length than the original, 
          // keep track of the delta so things can be repositioned in the source buffer
          lenDelta = (iReplStrLen - scsw->iWordLen) * nInc;

          lenDeltaRet += lenDelta;
          pEditView->UpdateFile();
          pEditView->SetPosDelta(scsw->lCharPos, lenDelta);
        } else {
          int iReplStrLen = IDM_INT_CAST strlen(lpszNewWord);
          pEditView->AddUndoReplace(scsw->lCharPos, scsw->lCharPos +
            scsw->iWordLen, 0, -1, lpszNewWord, iReplStrLen);
          ::SendMessage(pEditView->GetEditCtrl().m_hWnd, EME_REPLACESEL, 1, (LPARAM)lpszNewWord);
          lenDelta =  iReplStrLen - token.len;
          lenDeltaRet += lenDelta;
          pEditView->UpdateFile();
          pEditView->SetPosDelta(scsw->lCharPos, lenDelta);
        }
      }
    }
    if (curword)
      delete curword;
  }

  //delete_aspell_document_checker(checker);
  if (curline)
    delete curline;
  return lenDeltaRet;
}

void CASpellCheck::Mark(LPVOID lpBuffer, DWORD dwSize,
                        RBDI pRunBeforeDlgInitFunction, LPVOID lpArgument)
{
  if (!speller || !checker)
    return;

  __int64 lLineStartPos = ((SCSW*)lpArgument)->lCharPos;

  SCSW* scsw = (SCSW*) lpArgument;
  ueView* pEditView = scsw->pView;
  scsw->lCharPos = lLineStartPos;

  CString sCurline;
  static CString snl = "\n\0";

  char *curline = sCurline.GetBuffer(dwSize+8);
  memcpy(curline, lpBuffer, dwSize); curline += dwSize;
  memcpy(curline, (LPCTSTR) snl, snl.GetLength());
  sCurline.ReleaseBuffer(dwSize + snl.GetLength());

  aspell_document_checker_process(checker, sCurline, dwSize+2);

  int diff = 0;
  int nInc = pEditView->m_bUnicode ? sizeof(WCHAR) : sizeof(char);

  __int64 lCharPos = scsw->lCharPos;
  __int64 lOkAreas = lCharPos;
  __int64 lOkALast = lCharPos + dwSize;

  scsw->bMark = TRUE;
  // Now find the misspellings in the line
  AspellToken token;
  while (token = aspell_document_checker_next_misspelling(checker),
    token.offset < dwSize && token.len > 0)
  {
    if (token.len >= 256)
      continue;

    scsw->iWordLen = token.len;
    scsw->lCharPos = lCharPos + token.offset * nInc;

    // invoking routine
    if (pRunBeforeDlgInitFunction) {

      /***********IGNORE OPTIONS*******/
      // now we will check word
      bool capit_a = false;
      bool capit_b = false;
      bool allcaps = true;
      bool numbers = false;
      bool ucase = false;
      bool lcase = false;
      bool accent = false;

      for (int i = 0; i < scsw->iWordLen; i++) {
        int c = ((unsigned char) sCurline[token.offset + i]);
        if (isupper(c)) {
          ucase = true;
          capit_a = i==0;
        } else if (islower(c)) {
          lcase = true;
          allcaps = false;
          capit_b = i!=0;
        } else if (c>='0' && c<='9') {
          numbers = true;
        }
        else if(c>=192 && c<=255) {
            accent = true;
        }
      }

      if (bIgnoreAllCaps && allcaps)
        continue;
      if (bIgnoreCapitalized && capit_a && capit_b)
        continue;
      if (bIgnoreWordWithNumbers && numbers)
        continue;
      if (bIgnoreWordWithMixedCase && ucase && lcase)
        continue;
      if (bIgnoreAccent && accent)
        continue;

      /*****************************/

      if (ignoreList.Find(sCurline.Mid(token.offset, token.len)))
        continue;
      if (scsw->lCharPos > lOkAreas) {
        __int64 scsw_lCharPos = scsw->lCharPos;
        int scsw_iWordLen = scsw->iWordLen;
        scsw->bMark = FALSE;
        scsw->lCharPos = lOkAreas;
        scsw->iWordLen = (int) (scsw_lCharPos - lOkAreas);
        pRunBeforeDlgInitFunction(lpArgument);
        scsw->iWordLen = scsw_iWordLen;
        scsw->lCharPos = scsw_lCharPos;
        scsw->bMark = TRUE;
      }
      pRunBeforeDlgInitFunction(lpArgument);
      lOkAreas = scsw->lCharPos + token.len * nInc;
    }
  }

  if (lOkALast > lOkAreas) {
    if (pRunBeforeDlgInitFunction) {
      scsw->bMark = FALSE;
      scsw->lCharPos = lOkAreas;
      scsw->iWordLen = (int)
        (lOkALast - lOkAreas);
      pRunBeforeDlgInitFunction(lpArgument);
    }
  }

}

int CASpellCheck::GetSuggestions(LPCSTR lpszCheckedWord, CStringArray& pSuggestions)
{
  int result = 0;
  if (!lpszCheckedWord || !*lpszCheckedWord)
    return NULL;  // no word on input -> nothing to check

  int iWordLength = IDM_INT_CAST strlen(lpszCheckedWord);

  if (nIgnore>=iWordLength)
    return NULL;  // word is too short -> nothing to check

  // now we will check word
  //j0:
  bool capit_a = false;
  bool capit_b = false;
  bool allcaps = true;
  bool numbers = false;
  bool ucase = false;
  bool lcase = false;
  bool accent = false;

  for (int i=0; i<iWordLength; i++) {
    int c = ((unsigned char) lpszCheckedWord[i]);
    if (isupper(c)) {
      ucase = true;
      capit_a = i==0;
    } else if (islower(c)) {
      lcase = true;
      allcaps = false;
      capit_b = i!=0;
    } else if (c>='0' && c<='9') {
      numbers = true;
    }
    else if (c>=192 && c<=255){
      accent = true;
    }
  }

  if (bIgnoreAllCaps && allcaps)
    return NULL;
  if (bIgnoreCapitalized && capit_a && capit_b)
    return NULL;
  if (bIgnoreWordWithNumbers && numbers)
    return NULL;
  if (bIgnoreWordWithMixedCase && ucase && lcase)
    return NULL;
  if(bIgnoreAccent && accent)
    return NULL;

  // need to check if word isn't in ignoreList
  if (ignoreList.Find(lpszCheckedWord))
    return NULL;

  const char* lpSugWord;
  const AspellWordList* wList = aspell_speller_suggest(speller, lpszCheckedWord, iWordLength);
  pSuggestions.RemoveAll();
  if (wList) {        // read suggestions
    AspellStringEnumeration* els = aspell_word_list_elements(wList);
    while ((lpSugWord = aspell_string_enumeration_next(els))!=NULL) {
      pSuggestions.Add(lpSugWord);
      result ++;
    }
    delete_aspell_string_enumeration(els);
  }
  return result;
}

LPCTSTR CASpellCheck::ChangeWord(LPCSTR lpszCheckedWord, RBDI pRunBeforeDlgInitFunction, LPVOID lpArgument, int* nRetCode, BOOL bUnicode)
{
  if (nRetCode)
    *nRetCode = IDC_IGNORE;

  BOOL bApplyOnAll = FALSE;
  if (!lpszCheckedWord || !*lpszCheckedWord)
    return NULL;  // no word on input -> nothing to check

  int iWordLength = IDM_INT_CAST strlen(lpszCheckedWord);

  if (nIgnore>=iWordLength)
    return NULL;  // word is too short -> nothing to check

  // now we will check word
  bool capit_a = false;
  bool capit_b = false;
  bool allcaps = true;
  bool numbers = false;
  bool ucase = false;
  bool lcase = false;
  bool accent = false;

  for (int i=0; i<iWordLength; i++) {
    int c = ((unsigned char) lpszCheckedWord[i]);
    if (isupper(c)) {
      ucase = true;
    if(!capit_a)
        capit_a = i==0;
    } else if (islower(c)) {
      lcase = true;
      allcaps = false;
    if(!capit_b)
        capit_b = i!=0;
    } else if (c>='0' && c<='9') {
      numbers = true;
    }
    else if(c>=192 && c<=255){
      accent = true;
    }
  }

  if (bIgnoreAllCaps && allcaps)
    return NULL;
  if (bIgnoreCapitalized && capit_a && capit_b)
    return NULL;
  if (bIgnoreWordWithNumbers && numbers)
    return NULL;
  if (bIgnoreWordWithMixedCase && ucase && lcase)
    return NULL;
  if(bIgnoreAccent && accent)
    return NULL;

  // need to check if word isn't in ignoreList
  if (ignoreList.Find(lpszCheckedWord))
    return NULL;

  // check if word is in changeList
  szReturned = changeList[lpszCheckedWord];
  if (!szReturned.IsEmpty()) {
    // invoking routine
    if (pRunBeforeDlgInitFunction)
      pRunBeforeDlgInitFunction(lpArgument);
    return szReturned;
  }

  const char* lpSugWord;
  const AspellWordList* wList = aspell_speller_suggest(speller, lpszCheckedWord, iWordLength);
  pSuggestions.RemoveAll();
  if (wList) {        // read suggestions
    AspellStringEnumeration* els = aspell_word_list_elements(wList);
    while ((lpSugWord = aspell_string_enumeration_next(els))!=NULL) {
      pSuggestions.Add(lpSugWord);
    }
    delete_aspell_string_enumeration(els);
  }

  // invoking routine
  if (pRunBeforeDlgInitFunction)
    pRunBeforeDlgInitFunction(lpArgument);

  // init dialog
  CASpellCheckDlg dlg;

  // CASpellCheckDlg has UniWnd controls which are wide character.
  // Convert the selected word (UTF8) to  wide character for the control
  WCHAR *pCheckedWord = GetW(lpszCheckedWord);
  if (pCheckedWord != NULL)
  {
    dlg.m_szCheckedWord = pCheckedWord;
    delete pCheckedWord;
  }

  dlg.pASpell = this;
  int iRet = dlg.DoModal();
  if (nRetCode)
    *nRetCode = iRet;

  switch (iRet) {
  case IDCANCEL:
    m_bCancel = TRUE;
    return NULL;

  case IDC_IGNOREALL:
    // word is OK, not need to change it
    bApplyOnAll = TRUE;
  case IDC_IGNORE:
  case IDOK:
    if (bApplyOnAll) {
      // we add this word to ignore list
      ignoreList.AddTail(lpszCheckedWord);
      bNeedSaveIL = TRUE;
    }
    return NULL;

  case IDC_ADD:
    // word will be added to the wordlist
    aspell_speller_add_to_personal(speller, lpszCheckedWord, iWordLength);
    aspell_speller_save_all_word_lists(speller);

    if (aspell_speller_error(speller)!=0) {
      pParentWnd->MessageBox(aspell_speller_error_message(speller), FormatString(IDS_SPELLCHECKER), MB_ICONERROR);
    } else {
      dwAddedWords += 1;
    }
    return NULL;

  case IDC_CHANGEALL:
    // checked word needs to be replaced with the one is returned
    bApplyOnAll = TRUE;

  case IDC_CHANGE:
  {
    // this is a wide char
    CStringW cstrwReturned = dlg.m_szChangeTo;
    CStringA szTemp;
    if (bUnicode)
    {
      // convert this from wide string to UTF-8
      szTemp = GetA(cstrwReturned, CP_UTF8);
      // szTemp = cstrwReturned;
    }
    else
    {
      // convert this pack to the local code page
      szTemp = GetA(cstrwReturned);
    }
    szReturned = szTemp;

    if (bApplyOnAll)
    {
      // we add this word into changeall list
      changeList.SetAt(lpszCheckedWord, szReturned);
    }

    return szReturned;
  }

  case IDC_OPTIONS:
    // user could change any parameter including language, so we need to save
    // added words if any and read options again
    ReloadOptions();
    m_bRefresh = true;
    return NULL;
  default:
    return NULL;
  }
}

BOOL CASpellCheck::MisspelledWord(LPCSTR lpszCheckedWord)
{
  if (!lpszCheckedWord && *lpszCheckedWord == '\0')
    return FALSE;

  int iCheckedWord = IDM_INT_CAST strlen(lpszCheckedWord);

  if (iCheckedWord <= nIgnore)
    return FALSE;

  bool capit_a = false;
  bool capit_b = false;
  bool allcaps = true;
  bool numbers = false;
  bool ucase = false;
  bool lcase = false;

  for (int i = 0; i < iCheckedWord; i++) {
    if (isupper(lpszCheckedWord[i])) {
      ucase = true;
      capit_a = i==0;
    } else if (islower(lpszCheckedWord[i])) {
      lcase = true;
      allcaps = false;
      capit_b = i!=0;
    } else if (lpszCheckedWord[i]>='0' && lpszCheckedWord[i]<='9') {
      numbers = true;
    }
  }

  if (bIgnoreAllCaps && allcaps)
    return FALSE;
  if (bIgnoreCapitalized && capit_a && capit_b)
    return FALSE;
  if (bIgnoreWordWithNumbers && numbers)
    return FALSE;
  if (bIgnoreWordWithMixedCase && ucase && lcase)
    return FALSE;

  if (ignoreList.Find(lpszCheckedWord))
    return FALSE;

  if (aspell_speller_check(speller, lpszCheckedWord, iCheckedWord))
    return FALSE;

  return TRUE;
}

LPCTSTR CASpellCheck::CheckWord(LPCSTR lpszCheckedWord, RBDI pRunBeforeDlgInitFunction, LPVOID lpArgument)
{
  BOOL bApplyOnAll = FALSE;
  if (!lpszCheckedWord || !*lpszCheckedWord)
    return NULL;  // no word on input -> nothing to check

  int iWordLenght = IDM_INT_CAST strlen(lpszCheckedWord);

  if (nIgnore>=iWordLenght)
    return NULL;  // word is too short -> nothing to check

  // now we will check word
j0:
  bool capit_a = false;
  bool capit_b = false;
  bool allcaps = true;
  bool numbers = false;
  bool ucase = false;
  bool lcase = false;

  for (int i=0; i<iWordLenght; i++) {
    if (isupper(lpszCheckedWord[i])) {
      ucase = true;
      capit_a = i==0;
    } else if (islower(lpszCheckedWord[i])) {
      lcase = true;
      allcaps = false;
      capit_b = i!=0;
    } else if (lpszCheckedWord[i]>='0' && lpszCheckedWord[i]<='9') {
      numbers = true;
    }
  }

  if (bIgnoreAllCaps && allcaps)
    return NULL;
  if (bIgnoreCapitalized && capit_a && capit_b)
    return NULL;
  if (bIgnoreWordWithNumbers && numbers)
    return NULL;
  if (bIgnoreWordWithMixedCase && ucase && lcase)
    return NULL;

  // need to check if word isn't in ignoreList
  if (ignoreList.Find(lpszCheckedWord))
    return NULL;

  // check if word is in changeList
  szReturned = changeList[lpszCheckedWord];
  if (!szReturned.IsEmpty()) {
    // invoking routine
    if (pRunBeforeDlgInitFunction)
      pRunBeforeDlgInitFunction(lpArgument);
    return szReturned;
  }

  int iret = aspell_speller_check(speller, lpszCheckedWord, iWordLenght);

  if (iret==1) {
    return NULL;        // word was found
  } else if (iret==0) { // word was not found, we will display dialog
    const char* lpSugWord;
    const AspellWordList* wList = aspell_speller_suggest(speller, lpszCheckedWord, iWordLenght);
    pSuggestions.RemoveAll();
    if (wList) {        // read suggestions
      AspellStringEnumeration* els = aspell_word_list_elements(wList);
      while ((lpSugWord = aspell_string_enumeration_next(els))!=NULL) {
        pSuggestions.Add(lpSugWord);
      }
      delete_aspell_string_enumeration(els);
    }

    // invoking routine
    if (pRunBeforeDlgInitFunction)
      pRunBeforeDlgInitFunction(lpArgument);

    // init dialog
    CASpellCheckDlg dlg;
    dlg.m_szCheckedWord = lpszCheckedWord;
    dlg.pASpell = this;
    switch (dlg.DoModal()) {
    case IDCANCEL:
      m_bCancel = TRUE;
      return NULL;

    case IDC_IGNOREALL:
      // word is OK, not need to change it
      bApplyOnAll = TRUE;
    case IDC_IGNORE:
    case IDOK:
      if (bApplyOnAll) {
        // we add this word to ignore list
        ignoreList.AddTail(lpszCheckedWord);
        bNeedSaveIL = TRUE;
      }
      return NULL;

    case IDC_ADD:
      // word will be added to the wordlist
      aspell_speller_add_to_personal(speller, lpszCheckedWord, iWordLenght);
      if (aspell_speller_error(speller)!=0) {
        pParentWnd->MessageBox(aspell_speller_error_message(speller), FormatString(IDS_SPELLCHECKER), MB_ICONERROR);
      } else {
        dwAddedWords += 1;
      }
      return NULL;

    case IDC_CHANGEALL:
      // checked word needs to be replaced with the one is returned
      bApplyOnAll = TRUE;
    case IDC_CHANGE:
      szReturned = dlg.m_szChangeTo;
      if (bApplyOnAll) {
        // we add this word into changeall list
        changeList.SetAt(lpszCheckedWord, szReturned);
      }
      return szReturned;
    case IDC_OPTIONS:
      // user could change any parameter including language, so we need to save
      // save added words if any and read options again
      ReloadOptions();
      goto j0;
    /*
    case IDC_DICTIONARY:
    case IDC_ENCODING:
    case IDC_SUGGMODE:
    case IDC_MINLEN:
      goto j0;
    */
    default:
      return NULL;
    }
  } else {              // some error
    pParentWnd->MessageBox(aspell_speller_error_message(speller), FormatString(IDS_SPELLCHECKER), MB_ICONERROR);
    return NULL;
  }
}

BOOL CASpellCheck::UpdateConfig(const char* what, const char* value)
{
  AspellConfig* new_config = aspell_config_clone(config);
  aspell_config_replace(new_config, what, value);
  ret = new_aspell_speller(new_config);
  delete_aspell_config(new_config);
  if (aspell_error(ret) != 0) {
    pParentWnd->MessageBox(aspell_error_message(ret), FormatString(IDS_SPELLCHECKER), MB_ICONERROR);
    delete_aspell_can_have_error(ret);
    return FALSE;
  }
  speller = to_aspell_speller(ret);
  config = aspell_speller_config(speller);
  return TRUE;
}

void CASpellCheck::Done()
{
  if (LibAspell) {
    // saving added words
    if (speller) {
      if (checker) {
        if (delete_aspell_document_checker) {
          delete_aspell_document_checker(checker);
        }
        checker = NULL;
      }
      if (dwAddedWords && aspell_speller_save_all_word_lists) {
        aspell_speller_save_all_word_lists(speller);
      }
      if (delete_aspell_speller) {
        delete_aspell_speller(speller);
      }
      speller = NULL;
    }
    if (FreeLibrary(LibAspell)) {
      LibAspell = NULL;
    }
  }
  m_bCancel = FALSE;
  SaveIgnoreList();
  Reset();
}

void CASpellCheck::LoadIgnoreList()
{
  ignoreList.RemoveAll();
  if (!szDictionary.IsEmpty()) {
    CSetPath sp;
    CString sz;
    for (int i=0; i<szDictionary.GetLength(); i++) {
      if (_istalpha(szDictionary[i]))
        sz += szDictionary[i];
    }
    CString szDirectory;
    AfxGetEditApp()->GetDefaultProfiles(0,0,0,0,0,&szDirectory);
    if (szDirectory.GetLength()) {
      szDirectory += GNU_ASPELL;
      if (sp.Set(szDirectory)) {
        CStdioFile list;
        CString s;
        s.Format(_T("ignore-%08X-%s.csv"), AfxGetMainWnd()->GetSafeHwnd(), sz);
        if (list.Open(s, CFile::modeRead)) {
          CString sList;
          while (list.ReadString(sList)) {
            if (!sList.IsEmpty())
              ignoreList.AddTail(sList);
          }
          list.Close();
        }
      }
    }
  }
  bNeedSaveIL = FALSE;
}

void CASpellCheck::SaveIgnoreList()
{
  if (bNeedSaveIL) {
    if (!szDictionary.IsEmpty()) {
      CSetPath sp;
      CString sz;
      for (int i=0; i<szDictionary.GetLength(); i++) {
        if (_istalpha(szDictionary[i]))
          sz += szDictionary[i];
      }
      CString szDirectory;
      AfxGetEditApp()->GetDefaultProfiles(0,0,0,0,0,&szDirectory);
      if (szDirectory.GetLength()) {
        szDirectory += GNU_ASPELL;
        DoPath(szDirectory);
        if (sp.Set(szDirectory)) {
          CStdioFile list;
          if (alst.sPath.IsEmpty())
            alst.sPath = szDirectory;
          CString s;
          s.Format(_T("ignore-%08X-%s.csv"), AfxGetMainWnd()->GetSafeHwnd(), sz);
          if (list.Open(s, CFile::modeCreate|CFile::modeWrite)) {
            POSITION pListPosition = ignoreList.GetHeadPosition();
            while (pListPosition) {
              CString& sList = ignoreList.GetNext(pListPosition);
              if (!sList.IsEmpty())
                list.WriteString(sList+_T("\n"));
            }
            list.Flush();
            list.Close();
            bNeedSaveIL = FALSE;
            alst.sList.Add(s);
          }
        }
      }
    }
  }
}
