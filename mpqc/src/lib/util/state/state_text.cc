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

#ifdef __GNUC__
#pragma implementation
#endif

#include <stdarg.h>
#include <stdio.h>

#include <util/state/state_text.h>

#define CLASSNAME StateOutText
#define PARENTS public StateOutFile
#include <util/class/classi.h>
void *
StateOutText::_castdown(const ClassDesc*cd)
{
  void* casts[1];
  casts[0] = StateOutFile::_castdown(cd);
  return do_castdowns(casts,cd);
}

StateOutText::StateOutText() :
  StateOutFile(),
  no_newline_(0),
  no_array_(0)
{
}

StateOutText::StateOutText(ostream&s) :
  StateOutFile(s),
  no_newline_(0),
  no_array_(0)
{
}

StateOutText::StateOutText(const char *path) :
  StateOutFile(path),
  no_newline_(0),
  no_array_(0)
{
}

StateOutText::~StateOutText()
{
}

#define CLASSNAME StateInText
#define PARENTS public StateInFile
#define HAVE_KEYVAL_CTOR
#include <util/class/classi.h>
void *
StateInText::_castdown(const ClassDesc*cd)
{
  void* casts[1];
  casts[0] =  StateInFile::_castdown(cd);
  return do_castdowns(casts,cd);
}

StateInText::StateInText() :
  StateInFile(),
  newlines_(0),
  no_newline_(0),
  no_array_(0)
{
}

StateInText::StateInText(istream& s) :
  StateInFile(s),
  newlines_(0),
  no_newline_(0),
  no_array_(0)
{
}

StateInText::StateInText(const char *path) :
  StateInFile(path),
  newlines_(0),
  no_newline_(0),
  no_array_(0)
{
}

StateInText::StateInText(const RefKeyVal &keyval):
  newlines_(0),
  no_newline_(0),
  no_array_(0)
{
  char *path = keyval->pcharvalue("file");
  if (!path) {
      cerr << "StateInText(const RefKeyVal&): no path given" << endl;
    }
  open(path);
  delete[] path;
}

StateInText::~StateInText()
{
}

///////////////////////////////////////////////////////////////////////

// no_newline() and its associated no_newline_ variable
// are used to omit the next newline generated by newline() in
// the input/output stream
void
StateOutText::no_newline()
{
  no_newline_ = 1;
}
void
StateOutText::no_array()
{
  no_array_ = 1;
}
void
StateInText::no_newline()
{
  no_newline_ = 1;
}
void
StateInText::no_array()
{
  no_array_ = 1;
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
  cerr << "StateInText aborting at line " << newlines_+1 << " in the input"
       << endl;
  ::abort();
}


///////////////////////////////////////////////////////////////////////

int StateOutText::put(const ClassDesc*cd)
{
  ostream out(buf_);
  //
  // write out parent info
  if (!classidmap_.contains((ClassDesc*)cd)) {
      putparents(cd);
      out << " version of class " << cd->name()
          << " is " << cd->version() << endl;
      out.flush();
      classidmap_[(ClassDesc*)cd] = nextclassid_++;
    }
  out << "object of class " << cd->name() << " being written" << endl;
  out.flush();
  return 0;
  }
int
StateOutText::putparents(const ClassDesc*cd)
{
  ostream out(buf_);
  const ParentClasses& parents = cd->parents();

  for (int i=0; i<parents.n(); i++) {
      // the cast is needed to de-const-ify the class descriptor
      ClassDesc*tmp = (ClassDesc*) parents[i].classdesc();
      if (!classidmap_.contains(tmp)) {
          putparents(tmp);
          out << " version of class " << tmp->name()
              << " is " << tmp->version() << endl;
          out.flush();
          classidmap_[tmp] = nextclassid_++;
        }
    }
  return 0;
}
int StateInText::get(const ClassDesc**cd)
{
  istream in(buf_);
  const int line_length = 512;
  char line[line_length];

  // if a list of class descriptors exists then read it in
  
  in.getline(line,line_length); newlines_++;
  while (strncmp(line,"object",6)) {
      char name[line_length];
      int version;
      sscanf(line," version of class %s is %d\n",
             name,
             &version);
      ClassDesc* tmp = ClassDesc::name_to_class_desc(name);
      // save the class descriptor and the version
      int classid = nextclassid_++;
      classidmap_[tmp] = classid;
      StateClassData classdat(version,tmp);
      classdatamap_[classid] = classdat;
      in.getline(line,line_length); newlines_++;
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
int StateInText::get(char&r, const char *key)
{
  no_array();
  return StateIn::get(r,key);
}

int StateOutText::put(int r)
{
  no_array();
  return StateOut::put(r);
}
int StateInText::get(int&r, const char *key)
{
  no_array();
  return StateIn::get(r,key);
}

int StateOutText::put(float r)
{
  no_array();
  return StateOut::put(r);
}
int StateInText::get(float&r, const char *key)
{
  no_array();
  return StateIn::get(r,key);
}

int StateOutText::put(double r)
{
  no_array();
  return StateOut::put(r);
}
int StateInText::get(double&r, const char *key)
{
  no_array();
  return StateIn::get(r,key);
}

int StateOutText::put(const char*d,int n)
{
  return StateOut::put(d,n);
}
int StateInText::get(char*&r)
{
  return StateIn::get(r);
}
int StateOutText::put(const int*d,int n)
{
  return StateOut::put(d,n);
}
int StateInText::get(int*&r)
{
  return StateIn::get(r);
}
int StateOutText::put(const float*d,int n)
{
  return StateOut::put(d,n);
}
int StateInText::get(float*&r)
{
  return StateIn::get(r);
}
int StateOutText::put(const double*d,int n)
{
  return StateOut::put(d,n);
}
int StateInText::get(double*&r)
{
  return StateIn::get(r);
}

int StateOutText::putobject(const RefSavableState &p)
{
  ostream out(buf_);
  int r=0;
  if (p.null()) {
      out << "reference to null" << endl;
      out.flush();
    }
  else {
      AVLMap<RefSavableState,StateOutData>::iterator ind = ps_.find(p);
      if (ind == ps_.end() || copy_references_) {
          // object has not been written yet
          StateOutData dp;
          dp.num = next_object_number_++;
          out << "writing object " << dp.num << endl;
          out.flush();
          const ClassDesc *cd = p->class_desc();
          put(cd);
          out.flush();
          dp.type = classidmap_[(ClassDesc*)cd];
          if (!copy_references_) ps_[p] = dp;
          have_classdesc();
          p->save_vbase_state(*this);
          p->save_data_state(*this);
        }
      else {
          out << "reference to object " << ind.data().num << endl;
          out.flush();
        }
    }
  return r;
}
int StateInText::getobject(RefSavableState &p)
{
  istream in(buf_);
  const int line_length = 512;
  char line[line_length];

  in.getline(line,line_length);
  newlines_++;

  if (!strcmp("reference to null",line)) {
      p = 0;
    }
  else if (!strncmp("writing",line,7)) {
      int refnum;
      sscanf(line,"writing object %d",&refnum);
      const ClassDesc *cd;
      get(&cd);
      have_classdesc();
      nextobject(refnum);
      DescribedClass *dc = cd->create(*this);
      p = SavableState::castdown(dc);
    }
  else if (!strncmp("reference",line,9)) {
      int refnum;
      sscanf(line,"reference to object %d",&refnum);
      p = ps_[refnum].ptr;
    }
  else {
      cerr << "StateInText: couldn't find a reference object" << endl;
      abort();
    }

  return 0;
}

void
StateOutText::start_array()
{
  ostream out(buf_);
  if (!no_array_) { out.put(' '); out.put('<'); }
}
void
StateInText::start_array()
{
  istream in(buf_);
  if (!no_array_) {
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
  if (!no_array_) {
      out.put(' '); out.put('>');
    }
  else {
      no_array_ = 0;
    }
}
void
StateInText::end_array()
{
  istream in(buf_);
  if (!no_array_) {
      if (in.get() != ' ' || in.get() != '>') {
          cerr << "StateInText: expected a \"> \"" << endl;
          abort();
        }
    }
  else {
      no_array_ = 0;
    }
}

void
StateOutText::newline()
{
  ostream out(buf_);
  if (no_newline_) {
      no_newline_ = 0;
      return;
    }
  out << endl;
  out.flush();
}
void
StateInText::newline()
{
  istream in(buf_);
  if (no_newline_) {
      no_newline_ = 0;
      return;
    }
  if (in.get() != '\n') {
      cerr << "StateInText: expected newline" << endl;
      abort();
    }
  newlines_++;
}

///////////////////////////////////////////////////////////////////////

int StateOutText::putstring(const char*s)
{
  int r = 0;
  if (s) {
      int size = strlen(s);
      no_newline(); r += put(size);
      if (size) {
          r += put_array_char(s,size);
        }
    }
  else {
      r += put((int)0);
    }
  return r;
}
int StateInText::getstring(char*&s)
{
  int r = 0;
  int size;
  no_newline(); r += get(size);
  if (size) {
      s = new char[size+1];
      s[size] = '\0';
      if (size) {
          r += get_array_char(s,size);
        }
    }
  else {
      s = 0;
    }
  return r;
}

///////////////////////////////////////////////////////////////////////

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
