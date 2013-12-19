#include "uac.h"
#include <boost/thread.hpp>

int main(int argc, char* argv[])
{
    account_t acc;
    acc.user_name = "12054190005";
    acc.user_host = "sip-rndmsg.lab.nordigy.ru";
    acc.auth_user = "400017457008";
    acc.proxy = "sip-rndmsg.lab.nordigy.ru:5090"; 
    uac a;
    a.init(acc);
    a.to_register();
    boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
    a.to_call();
    boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
    a.to_register();
    boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
    a.to_call();
    boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
    return 0;
}

