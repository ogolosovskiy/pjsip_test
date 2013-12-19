#pragma once

struct sip_exception : public std::exception
{
  sip_exception(char const* a_description) 
     : description(a_description)
  {}
  const char * what () const throw ()
  {
    return description.c_str();
  }
  std::string description;
};

