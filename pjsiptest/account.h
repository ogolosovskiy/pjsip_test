
#pragma once;

struct account_t
{
  std::string user_name;
  std::string user_host;
  std::string user_url() const
  {
      std::stringstream ss;
      ss << "sip:" << user_name << "@" << user_host;
      return ss.str();
  }
  std::string proxy; // host:port
  std::string auth_user; // sip url
  std::string password; 
};