#pragma once

class siplib;
#include <boost/shared_ptr.hpp>
typedef boost::shared_ptr<siplib> siplib_prt;
#include <string>
#include <sstream>
#include <iostream>
#include "account.h"


class uac
{
public:
  uac(void);
  void init(account_t const& a_account);
  void to_register();
  void to_call();
  ~uac(void);
private:
   siplib_prt sip_impl;
};
