#pragma once

#include "libs.h"
#include "sip_exception.h"
#include "uac.h"
#include "sip_exception.h"
#include "concurrent_queue.h"
#include "register_session.h"
#include "invite_session.h"


class siplib : public shared_sip
{

public:

  typedef std::auto_ptr<boost::thread> thread_ptr;
  typedef boost::function<void()> job;

  siplib(account_t const& a_account);
  virtual ~siplib(void);
  void to_register();
  void to_call();
  std::string create_contact() const;
  std::string network_interafce() const;
  pjsip_endpoint* endpoint() const;
  account_t account() const;
  pjsip_module* register_handler() const 
  { 
    assert(register_event_handler.id!=-1);
    //  PJ lib workaround
    return &(const_cast<siplib*>(this)->register_event_handler);
  }
  pjsip_module* invite_handler() const 
  {
    assert(inv_sip_event_handler.id!=-1);
    //  PJ lib workaround
    return &(const_cast<siplib*>(this)->inv_sip_event_handler);
  }

private:
  static void log( int level, const char *buf, int len );
  pj_status_t pj_lib_init();
  void loop();

  pjsip_endpoint*                 pj_endpt; 
  pj_caching_pool                 pj_cpool;

  thread_ptr                      pj_thread;
  bool                            to_stop;

  register_session::ptr           registrar;    // TODO change pointer type and storing model
  invite_session::ptr             call;         // TODO change pointer type and storing model

  pjsip_inv_callback              inv_session_event_handler;
  pjsip_module                    inv_sip_event_handler;
  pjsip_module                    register_event_handler;


  account_t                       account_data;
  std::string                     contact;

  concurrent_queue<job>           jobs;
};
