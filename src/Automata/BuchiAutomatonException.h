#ifndef _BUCHI_AUT_EXCEPTION_H_
#define _BUCHI_AUT_EXCEPTION_H_

#include <exception>

class BuchiAutomatonException : public exception
{
private:
  string msg;

public:
  BuchiAutomatonException(string info = "Buchi exception") : msg(info) {}

  virtual const char* what() const throw () { return msg.c_str(); }
};

#endif
