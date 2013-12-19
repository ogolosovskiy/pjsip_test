#pragma once

//  Precompile HEADER for any common

#include <memory>
#include <exception>
#include <string>
#include <sstream>
#include <iostream>

#include <pjsip.h>
#include <pjlib-util.h>
#include <pjlib.h>
#pragma warning( push ) 
#pragma warning( disable : 4244 )
#include <pjsip_ua.h>
#pragma warning( pop ) 

#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/enable_shared_from_this.hpp> 
#include <boost/function.hpp>

#include "sip_exception.h"
#include "account.h"

inline pj_str_t str2pj(std::string const& str)
{
  return pj_str(const_cast<char*>(str.c_str()));
}

#define EMPTY_STR pj_str('\0');

class shared_sip
{
public:
  virtual pjsip_endpoint* endpoint() const = 0;
  virtual account_t       account() const = 0;
  virtual std::string create_contact() const = 0;
  virtual std::string network_interafce() const = 0;
  virtual pjsip_module* register_handler() const = 0;
  virtual pjsip_module* invite_handler() const = 0;
};

// TODO ERROR level log FILE STRING
#define DO_IF_STATUS_FAILS(status, description, do) {\
    if(status != PJ_SUCCESS) { \
    std::stringstream str; \
    str << description << " erorr code: " << status; \
    std::cout << str.str()  << std::endl; \
    do;  } }

int const SIP_PORT = 5060;