#include "register_session.h"


register_session::register_session(shared_sip const& a_shared)
  : shared(a_shared)
  , contact(a_shared.create_contact())
{
  account = shared.account(); // copy to avoid changes
}

void register_session::start()
{
  send_register();
}

register_session::~register_session(void)
{
}


pj_bool_t register_session::on_rx_response(pjsip_rx_data *rdata)
{
    try
    {
        // TODO filter INVITE
        if( !rdata )
        {
            // RCLOG_ERROR( "Either tsx or event pointer is NULL" );
            return PJ_FALSE;
        }
//         register_session* _this = static_cast<register_session*>(rdata->mod_data[0]);
//         if(_this)
//             _this->rx_response(tsx, event);

    }
    catch (std::exception &/*e*/) 
    {
        //log(// std::cout << e.what() << std::endl;
    }
    
    return PJ_TRUE;
}

pj_bool_t register_session::rx_response(pjsip_rx_data *rdata)
{
    return PJ_TRUE;
}


void register_session::on_tsx_state( pjsip_transaction *tsx, pjsip_event *event )
{
  try
  {
    // TODO filter INVITE
    if( !tsx || !event )
    {
      // RCLOG_ERROR( "Either tsx or event pointer is NULL" );
      return;
    }
    register_session* _this = static_cast<register_session*>(tsx->mod_data[0]);
    if(_this)
      _this->tsx_state(tsx, event);
  }
  catch (std::exception &/*e*/) 
  {
    //log(// std::cout << e.what() << std::endl;
  }
}

void register_session::tsx_state( pjsip_transaction *tsx, pjsip_event *event )
{
  if ( !( event->type == PJSIP_EVENT_TSX_STATE && tsx->state > PJSIP_TSX_STATE_CALLING ) )
  {
      if(event)
        call_id = "fff"; //event->body.rx_msg.rdata->msg_info.cid->id.ptr;
          //.rx_msg->rdata->cid->id->ptr;
      return;
  }

  if ( tsx->state == PJSIP_TSX_STATE_COMPLETED )
  {
    //RCLOG_NOTICE( "Received SIP %d %.*s for %s (%d)", tsx->status_code, tsx->status_text.slen, tsx->status_text.ptr, record->contact, record->deviceType );
  }
  else if ( tsx->state == PJSIP_TSX_STATE_TERMINATED && tsx->status_code >= PJSIP_SC_INTERNAL_SERVER_ERROR )
  {
    //RCLOG_NOTICE( "Transport error for %s (%d)", record->contact, record->deviceType );
  }
  else if ( tsx->state == PJSIP_TSX_STATE_TERMINATED && tsx->status_code == PJSIP_SC_REQUEST_TIMEOUT )
  {
    //RCLOG_NOTICE( "Request timed out for %s (%d)", record->contact, record->deviceType );
  }
  else if ( tsx->state == PJSIP_TSX_STATE_DESTROYED )
  {
    // RCLOG_DEBUG("tsx: %#x tsx->state == PJSIP_TSX_STATE_DESTROYED", tsx->obj_name);
  }
}



pjsip_tx_data* register_session::compose_register()
{

  pj_status_t status;

  std::string to = account.user_url();
  pj_str_t to_addr = str2pj(to);
  pj_str_t from_addr = EMPTY_STR;
  pj_str_t contact_uri = str2pj(contact);

  pj_str_t callid;
  pj_str_t* callidptr = NULL;  // put NULL to generate unique Call-ID
  if(!call_id.empty())
  {
      callid = str2pj(call_id);
      callidptr = &callid;
  }

  std::stringstream ss;
  ss << "sip:" << account.proxy;
  std::string req_uri_str = ss.str();
  pj_str_t req_uri = str2pj(req_uri_str);
  pjsip_tx_data* tdata = NULL;

  status = pjsip_endpt_create_request( shared.endpoint(), 
    &pjsip_register_method, 
    &req_uri,       // target URI
    &to_addr,       // From:
    &to_addr,       // To:
    &contact_uri,   // Contact:
    callidptr,        // Call-Id
    1,              // CSeq#
    NULL,           // body
    &tdata );       


  if ( PJ_SUCCESS != status )
    DO_IF_STATUS_FAILS(status, "pjsip_endpt_create_request failed", return NULL);

  pjsip_cid_hdr* cid_hdr = PJSIP_MSG_CID_HDR(tdata->msg);
  call_id = std::string(cid_hdr->id.ptr, cid_hdr->id.slen);

//   pjsip_generic_string_hdr* header = pjsip_generic_string_hdr_create( m_appPool, &header_name, &header_value );
//   pjsip_msg_add_hdr( notifyTData->msg, (pjsip_hdr*) header );

  return tdata;
}

void register_session::send_register()
{
  pj_status_t status;
  pjsip_transaction *tsxTransaction;

  pjsip_tx_data* txData = compose_register();

  if ( txData == NULL )
      DO_IF_STATUS_FAILS(PJSIP_ENOTINITIALIZED  , "compose_register failed", return);

  status = pjsip_tsx_create_uac( shared.register_handler(), txData, &tsxTransaction );
      DO_IF_STATUS_FAILS(status, "pjsip_tsx_create_uac failed", return);

  tsxTransaction->mod_data[0] = static_cast<void*>(this);

  status = pjsip_tsx_send_msg( tsxTransaction, NULL );
      DO_IF_STATUS_FAILS(status, "pjsip_tsx_send_msg failed", return);
}

