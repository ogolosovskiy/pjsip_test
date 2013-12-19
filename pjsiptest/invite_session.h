#pragma once

#include "libs.h"

class invite_session : public boost::noncopyable
{
public:

  typedef boost::shared_ptr<invite_session> ptr;
  friend class siplib;

  invite_session(shared_sip const& a_shared);
  virtual ~invite_session(void);

  void invite();


private:
  void compose_initial_invite();

  static void on_state_changed( pjsip_inv_session *inv, pjsip_event *e);
  static void on_new_session(pjsip_inv_session *inv, pjsip_event *e);
  static void on_media_update( pjsip_inv_session *inv, pj_status_t status);
  
  account_t account;
  std::string call_id;
  std::string to;
  
  shared_sip const&     shared;

  pjsip_inv_session*    session;
  pjsip_dialog*         dlg;

  std::string contact;
    
};
