//
// keyvalval.cc
//
// Copyright (C) 1996 Limit Point Systems, Inc.
//
// Author: Curtis Janssen <cljanss@limitpt.com>
// Maintainer: LPS
//
// This file is part of the SC Toolkit.
//
// The SC Toolkit is free software; you can redistribute it and/or modify
// it under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// The SC Toolkit is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with the SC Toolkit; see the file COPYING.LIB.  If not, write to
// the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
// The U.S. Government is granted a limited license as per AL 91-7.
//

#ifdef __GNUG__
#pragma implementation
#endif

#include <string.h>
#include <ctype.h>
#include <math.h>

#include <util/keyval/keyvalval.h>

/////////////////////////////////////////////////////////////////////////

REF_def(KeyValValue)

KeyValValue::KeyValValue(const KeyValValue&)
{
}

KeyValValue::~KeyValValue()
{
}

KeyValValue::KeyValValueError
KeyValValue::doublevalue(double& val) const
{
  KeyValValuedouble def;
  def.doublevalue(val);
  return KeyValValue::WrongType;
}

KeyValValue::KeyValValueError
KeyValValue::booleanvalue(int& val) const
{
  KeyValValueboolean def;
  def.booleanvalue(val);
  return KeyValValue::WrongType;
}

KeyValValue::KeyValValueError
KeyValValue::floatvalue(float& val) const
{
  KeyValValuefloat def;
  def.floatvalue(val);
  return KeyValValue::WrongType;
}

KeyValValue::KeyValValueError
KeyValValue::charvalue(char& val) const
{
  KeyValValuechar def;
  def.charvalue(val);
  return KeyValValue::WrongType;
}

KeyValValue::KeyValValueError
KeyValValue::intvalue(int& val) const
{
  KeyValValueint def;
  def.intvalue(val);
  return KeyValValue::WrongType;
}

KeyValValue::KeyValValueError
KeyValValue::pcharvalue(const char*& val) const
{
  KeyValValuepchar def;
  def.pcharvalue(val);
  return KeyValValue::WrongType;
}

KeyValValue::KeyValValueError
KeyValValue::describedclassvalue(RefDescribedClass& val) const
{
  KeyValValueRefDescribedClass def;
  def.describedclassvalue(val);
  return KeyValValue::WrongType;
}

void
KeyValValue::print(ostream&o) const
{
  o << "(empty value)";
}

ostream&
operator << (ostream&o, const KeyValValue &val)
{
  val.print(o);
  return o;
}

/////////////////////////////////////////////////////////////////////////

KeyValValuedouble::KeyValValuedouble(const KeyValValuedouble&val):
  _val(val._val)
{
}
KeyValValuedouble::~KeyValValuedouble()
{
}

KeyValValue::KeyValValueError
KeyValValuedouble::doublevalue(double&val) const
{
  val = _val;
  return KeyValValue::OK;
}

void
KeyValValuedouble::print(ostream&o) const
{
  o << _val;
}

/////////////////////////////////////////////////////////////////////////

KeyValValueboolean::KeyValValueboolean(const KeyValValueboolean&val):
  _val(val._val)
{
}
KeyValValueboolean::~KeyValValueboolean()
{
}

KeyValValue::KeyValValueError
KeyValValueboolean::booleanvalue(int&val) const
{
  val = _val;
  return KeyValValue::OK;
}

void
KeyValValueboolean::print(ostream&o) const
{
  o << (_val?"true":"false");
}

/////////////////////////////////////////////////////////////////////////

KeyValValuefloat::KeyValValuefloat(const KeyValValuefloat&val):
  _val(val._val)
{
}
KeyValValuefloat::~KeyValValuefloat()
{
}

KeyValValue::KeyValValueError
KeyValValuefloat::floatvalue(float&val) const
{
  val = _val;
  return KeyValValue::OK;
}

void
KeyValValuefloat::print(ostream&o) const
{
  o << _val;
}

/////////////////////////////////////////////////////////////////////////

KeyValValuechar::KeyValValuechar(const KeyValValuechar&val):
  _val(val._val)
{
}
KeyValValuechar::~KeyValValuechar()
{
}

KeyValValue::KeyValValueError
KeyValValuechar::charvalue(char&val) const
{
  val = _val;
  return KeyValValue::OK;
}

void
KeyValValuechar::print(ostream&o) const
{
  o << _val;
}

/////////////////////////////////////////////////////////////////////////

KeyValValueint::KeyValValueint(const KeyValValueint&val):
  _val(val._val)
{
}
KeyValValueint::~KeyValValueint()
{
}

KeyValValue::KeyValValueError
KeyValValueint::intvalue(int&val) const
{
  val = _val;
  return KeyValValue::OK;
}

void
KeyValValueint::print(ostream&o) const
{
  o << _val;
}

/////////////////////////////////////////////////////////////////////////

KeyValValuepchar::KeyValValuepchar(const char* val):
  _val(strcpy(new char[strlen(val)+1],val))
{
}
KeyValValuepchar::KeyValValuepchar(const KeyValValuepchar&val):
  _val(strcpy(new char[strlen(val._val)+1],val._val))
{
}
KeyValValuepchar::~KeyValValuepchar()
{
  delete[] _val;
}
KeyValValue::KeyValValueError
KeyValValuepchar::pcharvalue(const char*&val) const
{
  val = _val;
  return KeyValValue::OK;
}

void
KeyValValuepchar::print(ostream&o) const
{
  o << _val;
}

/////////////////////////////////////////////////////////////////////////

KeyValValueRefDescribedClass::
  KeyValValueRefDescribedClass(const KeyValValueRefDescribedClass& val):
  _val(val._val)
{
}
KeyValValueRefDescribedClass::
  ~KeyValValueRefDescribedClass()
{
}
KeyValValue::KeyValValueError
KeyValValueRefDescribedClass::describedclassvalue(RefDescribedClass&val) const
{
  val = _val;
  return KeyValValue::OK;
}

void
KeyValValueRefDescribedClass::print(ostream&o) const
{
  if (_val.nonnull()) {
      o << "<" << _val->class_name()
        << " " << _val->identifier()
        << ">";
    }
  else {
      o << "<empty object>";
    }
}

/////////////////////////////////////////////////////////////////////////

KeyValValueString::KeyValValueString(
    const char* val, KeyValValueString::Storage s)
{
  switch (s) {
  case Copy:
      _val_to_delete = strcpy(new char[strlen(val)+1], val);
      _val = _val_to_delete;
      break;
  case Steal:
      ExEnv::err() << "KeyValValueString: CTOR: cannot steal const string" << endl;
      abort();
      break;
  case Use:
      _val = val;
      _val_to_delete = 0;
      break;
    }
}
KeyValValueString::KeyValValueString(
    char* val, KeyValValueString::Storage s)
{
  switch (s) {
  case Copy:
      _val_to_delete = strcpy(new char[strlen(val)+1], val);
      _val = _val_to_delete;
      break;
  case Steal:
      _val = val;
      _val_to_delete = val;
      break;
  case Use:
      _val = val;
      _val_to_delete = 0;
      break;
    }
}
KeyValValueString::KeyValValueString(const KeyValValueString&val)
{
  if (val._val_to_delete == 0) {
      _val = val._val;
      _val_to_delete = 0;
    }
  else {
      _val_to_delete = strcpy(new char[strlen(val._val)+1], val._val);
      _val = _val_to_delete;
    }
}
KeyValValueString::~KeyValValueString()
{
  delete[] _val_to_delete;
}
KeyValValue::KeyValValueError
KeyValValueString::doublevalue(double&val) const
{
  val = atof(_val);
  return KeyValValue::OK;
}
KeyValValue::KeyValValueError
KeyValValueString::booleanvalue(int&val) const
{
  char lc_kv[20];
  strncpy(lc_kv,_val,20);
  for (int i=0; i<20; i++) {
      if (isupper(lc_kv[i])) lc_kv[i] = tolower(lc_kv[i]);
    }
  if (!strcmp(lc_kv,"yes")) val = 1;
  else if (!strcmp(lc_kv,"true")) val = 1;
  else if (!strcmp(lc_kv,"1")) val = 1;
  else if (!strcmp(lc_kv,"no")) val = 0;
  else if (!strcmp(lc_kv,"false")) val = 0;
  else if (!strcmp(lc_kv,"0")) val = 0;
  else {
      val = 0;
      return KeyValValue::WrongType;
    }

  return KeyValValue::OK;
}
KeyValValue::KeyValValueError
KeyValValueString::floatvalue(float&val) const
{
  val = (float) atof(_val);
  return KeyValValue::OK;
}
KeyValValue::KeyValValueError
KeyValValueString::charvalue(char&val) const
{
  val = _val[0];
  return KeyValValue::OK;
}
KeyValValue::KeyValValueError
KeyValValueString::intvalue(int&val) const
{
  val = atoi(_val);
  return KeyValValue::OK;
}
KeyValValue::KeyValValueError
KeyValValueString::pcharvalue(const char*&val) const
{
  val = _val;
  return KeyValValue::OK;
}

void
KeyValValueString::print(ostream&o) const
{
  if (_val) o << _val;
  else o << "(empty value)";
}

/////////////////////////////////////////////////////////////////////////////

// Local Variables:
// mode: c++
// c-file-style: "CLJ"
// End:
