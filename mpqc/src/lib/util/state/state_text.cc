//
// state_text.cc
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

#include <stdarg.h>
#include <stdio.h>
#include <util/class/class.h>
#include <util/state/state.h>
#include <util/state/state_ptr.h>
#include <util/state/stateptrSet.h>
#include <util/state/statenumSet.h>
#include <util/state/stateptrImplSet.h>
#include <util/state/statenumImplSet.h>

#include <util/state/classdImplMap.h>

StateOutText::StateOutText() :
  StateOutFile()
{
}

StateOutText::StateOutText(ostream&s) :
  StateOutFile(s)
{
}

StateOutText::StateOutText(const char *path) :
  StateOutFile(path)
{
}

StateOutText::~StateOutText()
{
}

StateInText::StateInText() :
  StateInFile(),
  _newlines(0)
{
}

StateInText::StateInText(istream& s) :
  StateInFile(s),
  _newlines(0)
{
}

StateInText::StateInText(const char *path) :
  StateInFile(path),
  _newlines(0)
{
}

StateInText::~StateInText()
{
}

///////////////////////////////////////////////////////////////////////

// no_newline() and its associated _no_newline static variable
// are used to omit the next newline generated by newline() in
// the input/output stream
static int _no_newline = 0;
void
no_newline()
{
  _no_newline = 1;
}
static int _no_array = 0;
void
no_array()
{
  _no_array = 1;
}

///////////////////////////////////////////////////////////////////////

int
StateInText::read(char*s)
{
  istream in(buf_);
  in >> s;
  if (in.fail()) {
      cerr << "StateInText::read(char*): failed" << endl;
      abort();
    }
  return strlen(s)+1;
}

int
StateInText::read(int&i)
{
  istream in(buf_);
  in >> i;
  if (in.fail()) {
      cerr << "StateInText::read(int&): failed\n" << endl;
      abort();
    }
  return (sizeof(int));
}

int
StateInText::read(float&f)
{
  istream in(buf_);
  in >> f;
  if (in.fail()) {
      cerr << "StateInText::read(float&): failed" << endl;
      abort();
    }
  return sizeof(float);
}

int
StateInText::read(double&d)
{
  istream in(buf_);
  in >> d;
  if (in.fail()) {
      cerr << "StateInText::read(double&): failed" << endl;
      abort();
    }
  return sizeof(double);
}

void
StateInText::abort()
{
  cerr << "StateInText aborting at line " << _newlines+1 << " in the input"
       << endl;
  ::abort();
}


///////////////////////////////////////////////////////////////////////

int StateOutText::put(const ClassDesc*cd)
{
  ostream out(buf_);
  //
  // write out parent info
  if (!_classidmap->contains((ClassDesc*)cd)) {
      putparents(cd);
      out << " version of class " << cd->name()
          << " is " << cd->version() << endl;
      out.flush();
      _classidmap->operator[]((ClassDesc*)cd) = _nextclassid++;
    }
  out << "object of class " << cd->name() << " being written" << endl;
  out.flush();
  return 0;
  }
void
StateOutText::putparents(const ClassDesc*cd)
{
  ostream out(buf_);
  const ParentClasses& parents = cd->parents();

  for (int i=0; i<parents.n(); i++) {
      // the cast is needed to de-const-ify the class descriptor
      ClassDesc*tmp = (ClassDesc*) parents[i].classdesc();
      if (!_classidmap->contains(tmp)) {
          putparents(tmp);
          out << " version of class " << tmp->name()
              << " is " << tmp->version() << endl;
          out.flush();
          _classidmap->operator[](tmp) = _nextclassid++;
        }
    }
}
int StateInText::get(const ClassDesc**cd)
{
  istream in(buf_);
  const int line_length = 512;
  char line[line_length];

  // if a list of class descriptors exists then read it in
  
  in.getline(line,line_length); _newlines++;
  while (strncmp(line,"object",6)) {
      char name[line_length];
      int version;
      sscanf(line," version of class %s is %d\n",
             name,
             &version);
      ClassDesc* tmp = ClassDesc::name_to_class_desc(name);
      // save the class descriptor and the version
      _cd.add(tmp);
      int position = _cd.iseek(tmp);
      if (_version.length() <= position) {
          _version.reset_length(position + 10);
        }
      _version[position] = version;
      in.getline(line,line_length); _newlines++;
    }

  // get the class name for the object
  char classname[line_length];
  sscanf(line,"object of class %s being written\n", classname);

  // convert the class id into the class descriptor
  *cd = ClassDesc::name_to_class_desc(classname);
  
  return 0;
}

int StateOutText::put(char r)
{
  no_array();
  return StateOut::put(r);
}
int StateInText::get(char&r)
{
  no_array();
  return StateIn::get(r);
}

int StateOutText::put(int r)
{
  no_array();
  return StateOut::put(r);
}
int StateInText::get(int&r)
{
  no_array();
  return StateIn::get(r);
}

int StateOutText::put(float r)
{
  no_array();
  return StateOut::put(r);
}
int StateInText::get(float&r)
{
  no_array();
  return StateIn::get(r);
}

int StateOutText::put(double r)
{
  no_array();
  return StateOut::put(r);
}
int StateInText::get(double&r)
{
  no_array();
  return StateIn::get(r);
}

int StateOutText::put(char*s,int size)
{
  if (!s) size = 0;
  if (putpointer((void*)s)) {
      no_newline(); put(size);
      int result = put_array_char(s,size);
      return result;
    }
  return 0;
}
int StateInText::get(char*&s)
{
  int objnum;
  if (objnum = getpointer((void**)&s)) {
    int size; no_newline(); get(size);
    if (size) {
      s = new char[size];
      havepointer(objnum,(void*)s);
      int result = get_array_char(s,size);
      return result;
      }
    else s = 0;
    }
  return 0;
}

int StateOutText::put(int*s,int size)
{
  if (!s) size = 0;
  if (putpointer((void*)s)) {
      no_newline(); put(size);
      int result = put_array_int(s,size);
      return result;
    }
  return 0;
}
int StateInText::get(int*&s)
{
  int objnum;
  if (objnum = getpointer((void**)&s)) {
    int size; no_newline(); get(size);
    if (size) {
      s = new int[size];
      havepointer(objnum,(void*)s);
      int result = get_array_int(s,size);
      return result;
      }
    else s = 0;
    }
  return 0;
}

int StateOutText::put(float*s,int size)
{
  if (!s) size = 0;
  if (putpointer((void*)s)) {
      no_newline(); put(size);
      int result = put_array_float(s,size);
      return result;
    }
  return 0;
}
int StateInText::get(float*&s)
{
  int objnum;
  if (objnum = getpointer((void**)&s)) {
    int size; no_newline(); get(size);
    if (size) {
      s = new float[size];
      havepointer(objnum,(void*)s);
      int result = get_array_float(s,size);
      return result;
      }
    else s = 0;
    }
  return 0;
}

int StateOutText::put(double*s,int size)
{
  if (!s) size = 0;
  if (putpointer((void*)s)) {
      no_newline(); put(size);
      int result = put_array_double(s,size);
      return result;
    }
  return 0;
}
int StateInText::get(double*&s)
{
  int objnum;
  if (objnum = getpointer((void**)&s)) {
    int size; no_newline(); get(size);
    if (size) {
      s = new double[size];
      havepointer(objnum,(void*)s);
      int result = get_array_double(s,size);
      return result;
      }
    else s = 0;
    }
  return 0;
}

int StateOutText::putpointer(void*p)
{
  ostream out(buf_);
  if (p == 0) {
      out << "reference to null" << endl;
      out.flush();
      return 0;
    }
  StateDataPtr dp(p);
  Pix ind = (ps_?ps_->seek(dp):0);
  if (ind == 0) {
      if (ps_) {
          dp.assign_num(next_pointer_number++);
          ps_->add(dp);
        }
      out << "writing object " << dp.num() << endl;
      out.flush();
      return 1;
    }
  else {
      out << "reference to object " << (*this->ps_)(ind).num() << endl;
      out.flush();
      return 0;
    }
}
int StateInText::getpointer(void**p)
{
  istream in(buf_);
  const int line_length = 512;
  char line[line_length];

  in.getline(line,line_length);
  _newlines++;

  if (!strcmp("reference to null",line)) {
      *p = 0;
      return 0;
    }
  else if (!strncmp("writing",line,7)) {
      int refnum;
      sscanf(line,"writing object %d",&refnum);
      StateDataNum num(refnum);
      *p = 0;
      return refnum;
    }
  else if (!strncmp("reference",line,9)) {
      int refnum;
      sscanf(line,"reference to object %d",&refnum);
      StateDataNum num(refnum);
      Pix ind = ps_->seek(num);
      *p = ((*this->ps_)(ind)).ptr();
      return 0;
    }
  else {
      cerr << "StateInText: couldn't find a reference object" << endl;
      abort();
    }

  return -1;
}

void
StateOutText::start_array()
{
  ostream out(buf_);
  if (!_no_array) { out.put(' '); out.put('<'); }
}
void
StateInText::start_array()
{
  istream in(buf_);
  if (!_no_array) {
      if (in.get() != ' ' || in.get() != '<') {
          cerr << "StateInText: expected a \" <\"" << endl;
          abort();
        }
    }
}

void
StateOutText::end_array()
{
  ostream out(buf_);
  if (!_no_array) {
      out.put(' '); out.put('>');
    }
  else {
      _no_array = 0;
    }
}
void
StateInText::end_array()
{
  istream in(buf_);
  if (!_no_array) {
      if (in.get() != ' ' || in.get() != '>') {
          cerr << "StateInText: expected a \"> \"" << endl;
          abort();
        }
    }
  else {
      _no_array = 0;
    }
}

void
StateOutText::newline()
{
  ostream out(buf_);
  if (_no_newline) {
      _no_newline = 0;
      return;
    }
  out << endl;
  out.flush();
}
void
StateInText::newline()
{
  istream in(buf_);
  if (_no_newline) {
      _no_newline = 0;
      return;
    }
  if (in.get() != '\n') {
      cerr << "StateInText: expected newline" << endl;
      abort();
    }
  _newlines++;
}

///////////////////////////////////////////////////////////////////////

int StateOutText::putstring(char*s)
{
  if (putpointer((void*)s)) {
      if (s) {
	  int size = strlen(s);
	  no_newline(); put(size);
          int result = 0;
          if (size) {
              result = put_array_char(s,size);
            }
          return result;
	}
      else {
	  put((int)0);
	}
    }
  return 0;
}
int StateInText::getstring(char*&s)
{
  int objnum;
  if (objnum = getpointer((void**)&s)) {
      int size;
      no_newline(); get(size);
      if (size) {
	  s = new char[size+1];
          s[size] = '\0';
	  havepointer(objnum,s);
          int result = 0;
          if (size) {
              result = get_array_char(s,size);
            }
          return result;
	}
      else {
	  s = 0;
	}
    }
  return 0;
}

int StateOutText::put_array_char(const char*d,int size)
{
  ostream out(buf_);
  start_array();
  int nwrit=size+1;
  for (int i=0; i<size; i++) { out.put(d[i]); }
  end_array();
  newline();
  return nwrit;
}
int StateInText::get_array_char(char*d,int size)
{
  istream in(buf_);
  start_array();
  int ch;
  for (int i=0; i<size; i++) {
      ch = in.get();
      if (ch == EOF) {
          cerr << "StateInText::get_array_char: EOF while reading array"
               << endl;
          abort();
        }
      d[i] = ch;
    }
  end_array();
  newline();
  return size+1;
}

int StateOutText::put_array_int(const int*d,int size)
{
  ostream out(buf_);
  start_array();
  int nwrit=0;
  for (int i=0; i<size; i++) { out << ' ' << d[i]; nwrit++; }
  out.flush();
  end_array();
  newline();
  return nwrit;
}
int StateInText::get_array_int(int*d,int size)
{
  start_array();
  int nread,tnread=0;
  for (int i=0; i<size; i++) {
      nread=read(d[i]);
      tnread += nread;
    }
  end_array();
  newline();
  return tnread;
}

int StateOutText::put_array_float(const float*d,int size)
{
  ostream out(buf_);
  start_array();
  int nwrit=0;
  for (int i=0; i<size; i++) {
      out.setf(ios::scientific);
      out.width(20);
      out.precision(15);
      out << ' ' << d[i];
      nwrit++;
    }
  out.flush();
  end_array();
  newline();
  return nwrit;
}
int StateInText::get_array_float(float*d,int size)
{
  start_array();
  int nread,tnread=0;
  for (int i=0; i<size; i++) {
      nread=read(d[i]);
      tnread += nread;
    }
  end_array();
  newline();
  return tnread;
}

int StateOutText::put_array_double(const double*d,int size)
{
  ostream out(buf_);
  start_array();
  int nwrit=0;
  for (int i=0; i<size; i++) {
      out.setf(ios::scientific);
      out.width(20);
      out.precision(15);
      out << ' ' << d[i];
      nwrit++;
    }
  out.flush();
  end_array();
  newline();
  return nwrit;
}
int StateInText::get_array_double(double*d,int size)
{
  start_array();
  int nread,tnread=0;
  for (int i=0; i<size; i++) {
      nread=read(d[i]);
      tnread += nread;
    }
  end_array();
  newline();
  return tnread;
}

/////////////////////////////////////////////////////////////////////////////

// Local Variables:
// mode: c++
// c-file-style: "CLJ"
// End:
