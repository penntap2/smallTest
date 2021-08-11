// ----------------------------------------------------------------------------
// Script Name: 
// Creation Date: 
// Last Modified: 
// Copyright (c)2007
// Purpose: 
// ----------------------------------------------------------------------------


#include "stdafx.h"
#include "idm_util.h"
#include "Account.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

{stuff}
[junk]
(mess)
<cool>
<cool
namespace idm
{ 
  void CopyConfig(const CConfig& src, CConfig& dst)
  {
    CConfig temp(src);
    dst.swap(temp);
  }

  void CopyOptions(const CConfig& src, CConfig& dst)
  {
    CConfig temp(src);
    dst.swap(temp);
  }

  //
  //aBase class for all idm config classes
  //
  CBase::CBase(const CConfig& config)
  {
    CopyConfig(config, config_);
  }

  CBase::~CBase()
  {}

  void CBase::UpdateConfig(bool init)
  {}

  void CBase::GetConfig(CConfig& config)
  {
    UpdateConfig();
    CopyConfig(config_, config);
  }

  void CBase::SetConfig(const CConfig& config)
  {
    for(CConfig::const_iterator itr = config.begin(); itr != config.end(); itr++)
    {
      std::string key = (*itr).first;
      std::string value = (*itr).second;
      config_.erase(key);
      config_[key] = value;
    }
    UpdateConfig(true);
  }

  bool CBase::GetConfigValue(const std::string& key, std::string& value) const
  {
    CConfig::const_iterator itr = config_.find(key);
    if(itr == config_.end() || (*itr).second == "")
    { 
      value = "";
      return false;
    }
    value = (*itr).second;
    return true;
  }

  bool CBase::EraseConfigEntry(const std::string &key)
  {
	  if ( key == "" ){
		  return false;
	  }else{
		  config_.erase(key);
		  return true;
	  }
  }

  void CBase::SetConfigValue(const std::string& key, const std::string& value)
  {
    if(value == "")
    {
      config_.erase(key);
      config_[key] = "";
    }
    else
      config_[key] = value;
  }

  CAccount::CAccount(const CConfig& config)
    : CBase(config)
  {
    account_id_ = idm_common::CreateId();
    modified_ = false;
  }

  CAccount::CAccount(const std::string& account_id, const CConfig& config)
    : CBase(config)
  {
    account_id_ = account_id;
    modified_ = false;
  }

  CAccount::~CAccount()
  {}

  std::string CAccount::GetAccountId() const
  {
    return account_id_;
  }

  void CAccount::GetAccountId(std::string& account_id) const
  {
    account_id = account_id_;
  }
  
  void CAccount::SetAccountId(const std::string& account_id)
  {
    account_id_ = account_id;
  }

  BOOL CAccount::IsDeleted()
  {
    BOOL bDeleted = FALSE;
    std::string sRet = "";
    std::string sKey = idm::IDM_CONFIG_DELETED;
    GetConfigValue(sKey, sRet);
    bDeleted = idm_util::ToNum(sRet.c_str());
    return bDeleted;
  }

  //
  //CAccountList
  //
  CAccountList::CAccountList()
    : default_index_id_(0), index_id_(ACCOUNTINDEX_INVALID+1)
  {
    mutex_ = CreateMutex(0, 0, 0);
    itr_ = account_map_.begin();
	m_bModifiedAccountData = false;
  }

  CAccountList::~CAccountList()
  {
    Clear();
    CloseHandle(mutex_);
  }

  unsigned int CAccountList::Insert(CAccount* account)
  {
    idm_common::CSimpleMutex mutex(mutex_, INFINITE);
    if(mutex.Acquired())
    {
      std::pair<CAccountMap::iterator, bool> ret = account_map_.insert(CAccountMap::value_type(index_id_, account->Clone()));
      itr_ = ret.first;
      if(ret.second)
      {
        index_id_++;
		m_bModifiedAccountData = true;
        return index_id_-1;
      }
    }
    return ACCOUNTINDEX_INVALID;
  }

  void CAccountList::Update(unsigned int index_id, CAccount* account)
  {
    idm_common::CSimpleMutex mutex(mutex_, INFINITE);
    if(mutex.Acquired())
    {
      unsigned int itr_index = ACCOUNTINDEX_INVALID;
      if(itr_ != account_map_.end())
        itr_index = (*itr_).first;
      CAccountMap::iterator itr = account_map_.find(index_id);
      if(itr != account_map_.end())
      {
        delete (*itr).second;  
        (*itr).second = 0;
		m_bModifiedAccountData = true;
        account_map_.erase(itr);
      }
      account_map_[index_id] = account->Clone();
      if(itr_index != ACCOUNTINDEX_INVALID)
        itr_ = account_map_.find(itr_index);
      else
        itr_ = account_map_.find(index_id);
    }
  }

  bool CAccountList::Remove(unsigned int index_id)
  {
    idm_common::CSimpleMutex mutex(mutex_, INFINITE);
    if(mutex.Acquired())
    {
      CAccountMap::iterator itr = account_map_.find(index_id);
      if(itr != account_map_.end())
      {
        unsigned int itr_index = ACCOUNTINDEX_INVALID;
        if(itr_ != account_map_.end())
          itr_index = (*itr_).first;
        erase_list_.push_back((*itr).second->GetAccountId());
        delete (*itr).second; 
        (*itr).second = 0;
        account_map_.erase(itr);
        itr_ = account_map_.find(itr_index);
		m_bModifiedAccountData = true;
        return true;
      }
    }
    return false;
  }
   
  unsigned int CAccountList::GetCurrent()
  {
    if(itr_ == account_map_.end())
      return ACCOUNTINDEX_INVALID;
    return (*itr_).first;
  }

  unsigned int CAccountList::GetCurrent(std::string& account_id)
  {
    if(itr_ == account_map_.end())
      return ACCOUNTINDEX_INVALID;
    account_id = (*itr_).second->GetAccountId();
    return (*itr_).first;
  }

  bool CAccountList::SetCurrent(unsigned int index_id)
  {
    if ( index_id == ACCOUNTINDEX_INVALID ){ return false; }

    idm_common::CSimpleMutex mutex(mutex_, INFINITE);
    if(mutex.Acquired())
    {
      CAccountMap::iterator itr = itr_;
      itr_ = account_map_.find(index_id);
      if(itr_ == account_map_.end())
      {
        itr_ = itr;
        return false;
      }
	  m_bModifiedAccountData = true;
      return true;
    }
    return false;
  }

  bool CAccountList::SetCurrent(const std::string& account_id)
  {
    idm_common::CSimpleMutex mutex(mutex_, INFINITE);
    if(mutex.Acquired())
    {
      for(CAccountMap::const_iterator itr = account_map_.begin(); itr != account_map_.end(); itr++)
      {
        if((*itr).second->GetAccountId() == account_id)
        {
          SetCurrent((*itr).first);
		  m_bModifiedAccountData = true;
          return true;
        }
      }
    }
    return false;
  }

  void CAccountList::Clear()
  {
    idm_common::CSimpleMutex mutex(mutex_, INFINITE);
    if(mutex.Acquired())
    {
      for(CAccountMap::iterator itr = account_map_.begin(); itr != account_map_.end(); itr++)
      {
        delete (*itr).second;
        (*itr).second = 0;
      }
      account_map_.clear();
	  m_bModifiedAccountData = true;
      itr_ = account_map_.end();
    }
  }

  bool CAccountList::Next()
  {
    if(itr_ != account_map_.end())
    {
      itr_++;
    }
    return (itr_ == account_map_.end())? false : true;
  }

  void CAccountList::Begin()
  {
    itr_ = account_map_.begin();
  }

  bool CAccountList::End()
  {
    if(itr_ == account_map_.end())
      return true;
    else
      return false;
  }

  unsigned int CAccountList::FindAccountByName(const std::string& account_name)
  {
    for(CAccountMap::iterator itr = account_map_.begin(); itr != account_map_.end(); itr++)
    {
      if((*itr).second->GetConfigValue(idm::IDM_CONFIG_ACCOUNTNAME) == account_name)
        return (*itr).first;
    }
    return ACCOUNTINDEX_INVALID;
  }

  unsigned int CAccountList::FindAccountById(const std::string& account_id)
  {
    for(CAccountMap::iterator itr = account_map_.begin(); itr != account_map_.end(); itr++)
    {
      if((*itr).second->GetConfigValue(idm::IDM_CONFIG_ACCOUNTID) == account_id)
        return (*itr).first;
    }
    return ACCOUNTINDEX_INVALID;
  }
}
