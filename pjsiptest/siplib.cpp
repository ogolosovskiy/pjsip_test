

// http://svn.pjsip.org/repos/pjproject/trunk/pjsip-apps/src/samples/simpleua.c

#include "siplib.h"

void siplib::log( int level, const char *buf, int len )
{
  std::cout << buf << std::endl;
}

siplib::siplib(account_t const& a_account)
  : pj_endpt(0)
  , to_stop(false)
  , account_data(a_account)
{

  if (pj_lib_init() != PJ_SUCCESS)
  {
    //RCLOG_ERROR( "Init PJLib: %d", status );
    throw sip_exception("PJLib init failure."); 
  }

  // TODO boost thread сделать старт синхронным
  pj_thread  = thread_ptr( new boost::thread( boost::bind( &siplib::loop, this ) ) );  
}


std::string siplib::network_interafce() const
{
    pj_sockaddr hostaddr;
    char hostip[PJ_INET6_ADDRSTRLEN+2];
    pj_status_t status = pj_gethostip(pj_AF_INET(), &hostaddr);
    DO_IF_STATUS_FAILS(status, "Unable to retrieve local host IP", throw sip_exception("Unable to retrieve local host IP"));
    pj_sockaddr_print(&hostaddr, hostip, sizeof(hostip), 2);
    return hostip;
}

std::string siplib::create_contact() const
{
    char temp[255];
    memset(temp,0,sizeof(temp));
    std::string hostip = network_interafce();
    pj_ansi_sprintf(temp, "<sip:%s@%s:%d>",  account_data.user_name.c_str(), hostip.c_str(), SIP_PORT);
    return temp;
}

pj_status_t siplib::pj_lib_init() 
{
  pj_status_t status;

  pj_log_set_log_func( (pj_log_func*) &log );
  pj_log_set_decor( PJ_LOG_HAS_LEVEL_TEXT );
  pj_log_set_level( 5 );

  /* Must init PJLIB first: */
  status = pj_init();
  DO_IF_STATUS_FAILS(status, "pj_init failed ", return status);

  status = pjlib_util_init();
  DO_IF_STATUS_FAILS(status, "pjlib_util_init failed", return status);

  pj_caching_pool_init(&pj_cpool, &pj_pool_factory_default_policy, 0 );

  status = pjsip_endpt_create(&(pj_cpool.factory), "sip library", &pj_endpt);
  DO_IF_STATUS_FAILS(status, "Unable to create PJ endpoint", return status);

  pj_sockaddr addr;
  pj_sockaddr_init( pj_AF_INET(), &addr, NULL, 0 );

  std::string eth = network_interafce();
  pj_str_t  s = str2pj(eth);
  pjsip_host_port pj_host_port = {s, 5060};

  pjsip_transport* transport;
  status = pjsip_udp_transport_start(pj_endpt, &addr.ipv4, &pj_host_port, 1, &transport);
  DO_IF_STATUS_FAILS(status, "Unable to start UDP transport", return status);

  status = pjsip_tsx_layer_init_module(pj_endpt);
  DO_IF_STATUS_FAILS(status, "Unable to start TSX layer module", return status);
    
  status = pjsip_ua_init_module( pj_endpt, NULL );
  DO_IF_STATUS_FAILS(status, "Unable to pjsip_ua_init_module", return status);


  // initialize common Invite modules
  pj_bzero(&inv_session_event_handler, sizeof(inv_session_event_handler));
  inv_session_event_handler.on_state_changed = &invite_session::on_state_changed;
  inv_session_event_handler.on_new_session = &invite_session::on_new_session;
  inv_session_event_handler.on_media_update = &invite_session::on_media_update;
  status = pjsip_inv_usage_init(endpoint(), &inv_session_event_handler);
  PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

  pj_bzero( &inv_sip_event_handler, sizeof( inv_sip_event_handler) );
  inv_sip_event_handler.name            = pj_str("Call Module");
  inv_sip_event_handler.id              = -1;
  inv_sip_event_handler.priority        = PJSIP_MOD_PRIORITY_APPLICATION;
  status = pjsip_endpt_register_module( endpoint(), &inv_sip_event_handler);
  PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

  // http://www.ietf.org/rfc/rfc3262.txt
  status = pjsip_100rel_init_module(endpoint());
  PJ_ASSERT_RETURN(status == PJ_SUCCESS, status);

  // initialize common Register modules
  pj_bzero( &register_event_handler, sizeof( register_event_handler) );
  register_event_handler.name            = pj_str("Register Module");
  register_event_handler.id              = -1;
  register_event_handler.on_tsx_state    = register_session::on_tsx_state;
  register_event_handler.priority        = PJSIP_MOD_PRIORITY_APPLICATION;
  status = pjsip_endpt_register_module( endpoint(), &register_event_handler);
  DO_IF_STATUS_FAILS(status, "Unable to create pjsip_endpt_register_module", throw sip_exception("Unable to create pjsip_endpt_register_module"));

  return PJ_SUCCESS;
}



void siplib::loop()
{
  pj_time_val timeout = { 0, 300}; // // milliseconds
  pj_thread_t *thread;
  pj_thread_desc thread_desc;
  pj_bzero(thread_desc, sizeof(thread_desc));

  pj_status_t status = pj_thread_register( "siplib::loop", thread_desc, &thread );
  DO_IF_STATUS_FAILS(status, "pj_thread_register failed", return );
  
  while (!to_stop)
  {
    pjsip_endpt_handle_events( pj_endpt, &timeout );
    job fun;
    while(jobs.try_pop(fun))
      fun();
  }
}

siplib::~siplib(void)
{
  to_stop = true;
// TODO
//   pj_thread_join(app.thread[i]);
//   pj_thread_destroy(app.thread[i]);
  pj_thread->join();

  pjsip_endpt_unregister_module(endpoint(), &register_event_handler);
  pj_bzero(&register_event_handler, sizeof(register_event_handler));
  register_event_handler.id              = -1;
  pjsip_endpt_unregister_module(endpoint(), &inv_sip_event_handler);
  inv_sip_event_handler.id              = -1;
  pj_bzero(&inv_session_event_handler, sizeof(inv_session_event_handler));

  pjsip_endpt_destroy(pj_endpt);
  pj_caching_pool_destroy(&pj_cpool);
  pj_shutdown();
  registrar.reset();
}

void siplib::to_register()
{
  assert(registrar.use_count()<2);
  registrar.reset();
  registrar = register_session::ptr(new register_session(*this));
  jobs.push(boost::bind(&register_session::start, registrar.get()));
}

void siplib::to_call()
{
  assert(call.use_count()<2);
  call.reset();
  call = invite_session::ptr(new invite_session(*this));
  jobs.push(boost::bind(&invite_session::invite, call.get()));
}

pjsip_endpoint* siplib::endpoint() const
{ 
  return pj_endpt; 
}

account_t siplib::account() const
{ 
  return account_data; 
}
