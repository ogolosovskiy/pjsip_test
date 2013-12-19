#pragma once

#include "libs.h"

class register_session : public boost::noncopyable //, public boost::enable_shared_from_this<register_session>
{
public:
  typedef boost::shared_ptr<register_session> ptr;
  friend class siplib;

  register_session(shared_sip const& a_shared);
  virtual ~register_session(void);
  void start();

private:
  void send_register();
  
  static void on_tsx_state( pjsip_transaction *tsx, pjsip_event *event );
  void tsx_state( pjsip_transaction *tsx, pjsip_event *event );

  static pj_bool_t on_rx_response(pjsip_rx_data *rdata);
  pj_bool_t rx_response(pjsip_rx_data *rdata);

  pjsip_tx_data* compose_register();

  shared_sip const&     shared;
  account_t             account;
  std::string           contact;
  std::string           call_id;
};
