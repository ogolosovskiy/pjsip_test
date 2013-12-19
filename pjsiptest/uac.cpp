#include "uac.h"
#include "siplib.h"

uac::uac(void) 
{
}

void uac::init(account_t const& a_account)
{
  if(sip_impl.get())
    throw sip_exception("cant init lib twice");
  sip_impl = siplib_prt(new siplib(a_account));
}

uac::~uac(void) 
{
  sip_impl.reset();
}

void uac::to_register()
{
  if(!sip_impl)
    throw sip_exception("not initialized");
  sip_impl->to_register();
}


void uac::to_call()
{
  if(!sip_impl)
    throw sip_exception("not initialized");
  sip_impl->to_call();
}