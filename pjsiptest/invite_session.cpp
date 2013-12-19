#include "invite_session.h"

invite_session::invite_session(shared_sip const& a_shared)
: shared(a_shared)
, call_id("JHGFIKHGFKHGF") // TODO
, to("12054190003")
, session(0) // TODO порядок не тот
, dlg(0)
{
    account = shared.account(); // copy to avoid changes
    contact = shared.create_contact();
}

invite_session::~invite_session(void)
{
}

void invite_session::invite()
{
    compose_initial_invite();
    // TODO разнести макароны по функциям
}

void invite_session::compose_initial_invite()
{
    std::string from = account.user_url();
    pj_str_t from_addr = str2pj(from);

    std::stringstream ss0;
    ss0 << "sip:" << to << "@" << account.user_host;
    std::string to_uri =  ss0.str();
    pj_str_t to_addr = str2pj(to_uri);
    pj_str_t contact_uri = str2pj(contact);
    pj_str_t callid = str2pj(call_id);

    std::stringstream ss;
    ss << "sip:" << to << "@" << account.proxy;
    std::string req_uri_str = ss.str();
    pj_str_t req_uri = str2pj(req_uri_str);
    pjsip_tx_data* tdata = NULL;

    pj_status_t status;

    status = pjsip_dlg_create_uac(pjsip_ua_instance(), 
        &from_addr,	            // (From header)
        &contact_uri,	    //local Contact
        &to_addr,		            //remote URI (To header)
        &req_uri,		            //remote target
        &dlg);	            //dialog

    int tt = PJSIP_ERRNO_START_PJSIP;
    bool test = PJSIP_EINVALIDMSG == status;

    if (status != PJ_SUCCESS) {
        //  app_perror(THIS_FILE, "Unable to create UAC dialog", status);
        return;
    }


    /* If we expect the outgoing INVITE to be challenged, then we should
    * put the credentials in the dialog here, with something like this:
    *
    {
    pjsip_cred_info	cred[1];

    cred[0].realm	  = pj_str("sip.server.realm");
    cred[0].scheme    = pj_str("digest");
    cred[0].username  = pj_str("theuser");
    cred[0].data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
    cred[0].data      = pj_str("thepassword");

    pjsip_auth_clt_set_credentials( &dlg->auth_sess, 1, cred);
    }
    *
    */


    /* Get the SDP body to be put in the outgoing INVITE, by asking
    * media endpoint to create one for us.
    */
    // 	status = pjmedia_endpt_create_sdp( g_med_endpt,	    /* the media endpt	*/
    // 					   dlg->pool,	    /* pool.		*/
    // 					   MAX_MEDIA_CNT,   /* # of streams	*/
    // 					   g_sock_info,     /* RTP sock info	*/
    // 					   &local_sdp);	    /* the SDP result	*/
    // 	PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);


    /* Create the INVITE session, and pass the SDP returned earlier
    * as the session's initial capability.
    */
    status = pjsip_inv_create_uac( dlg, NULL, 0, &session);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
    session->mod_data[0] = this;


    /* If we want the initial INVITE to travel to specific SIP proxies,
    * then we should put the initial dialog's route set here. The final
    * route set will be updated once a dialog has been established.
    * To set the dialog's initial route set, we do it with something
    * like this:
    *
    {
    pjsip_route_hdr route_set;
    pjsip_route_hdr *route;
    const pj_str_t hname = { "Route", 5 };
    char *uri = "sip:proxy.server;lr";

    pj_list_init(&route_set);

    route = pjsip_parse_hdr( dlg->pool, &hname, 
    uri, strlen(uri),
    NULL);
    PJ_ASSERT_RETURN(route != NULL, 1);
    pj_list_push_back(&route_set, route);

    pjsip_dlg_set_route_set(dlg, &route_set);
    }
    *
    * Note that Route URI SHOULD have an ";lr" parameter!
    */

    /* Create initial INVITE request.
    * This INVITE request will contain a perfectly good request and 
    * an SDP body as well.
    */
    status = pjsip_inv_invite(session, &tdata);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    /* Send initial INVITE request. 
    * From now on, the invite session's state will be reported to us
    * via the invite session callbacks.
    */
    status = pjsip_inv_send_msg(session, tdata);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

}


void invite_session::on_state_changed( pjsip_inv_session *inv, pjsip_event *e)
{
    PJ_UNUSED_ARG(e);

    invite_session* _this = static_cast<invite_session*>(inv->mod_data[0]);
    if(_this)
        //_this->state_changed(inv, e);
        int do_nothing_debug = 0;

    if (inv->state == PJSIP_INV_STATE_DISCONNECTED) 
    {
        //      PJ_LOG(3,(THIS_FILE, "Call DISCONNECTED [reason=%d (%s)]",
        //        inv->cause,
        //      pjsip_get_status_text(inv->cause)->ptr));
        //        PJ_LOG(3,(THIS_FILE, "One call completed, application quitting..."));
    } else {
        //PJ_LOG(3,(THIS_FILE, "Call state changed to %s",
        //  pjsip_inv_state_name(inv->state)));
    }
}

void invite_session::on_new_session(pjsip_inv_session *inv, pjsip_event *e)
{
    PJ_UNUSED_ARG(inv);
    PJ_UNUSED_ARG(e);
}

void invite_session::on_media_update( pjsip_inv_session *inv, pj_status_t status)
{
    PJ_UNUSED_ARG(inv);
    PJ_UNUSED_ARG(status);
}
