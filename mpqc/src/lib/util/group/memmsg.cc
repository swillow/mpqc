//
// memmsg.cc
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

#ifndef _util_group_memmsg_cc
#define _util_group_memmsg_cc

#ifdef __GNUC__
#pragma implementation
#endif

#include <util/group/memmsg.h>

#define CLASSNAME MsgMemoryGrp
#define PARENTS public MemoryGrp
#include <util/class/classia.h>
void *
MsgMemoryGrp::_castdown(const ClassDesc*cd)
{
  void* casts[1];
  casts[0] =  MemoryGrp::_castdown(cd);
  return do_castdowns(casts,cd);
}

MsgMemoryGrp::MsgMemoryGrp(const RefMessageGrp &msg)
{
  msg_ = msg;
  n_ = msg->n();
  me_ = msg->me();
}

MsgMemoryGrp::MsgMemoryGrp(const RefKeyVal &keyval):
  MemoryGrp(keyval)
{
  RefMessageGrp msg = keyval->describedclassvalue("message");
  if (msg.null()) {
      msg = MessageGrp::get_default_messagegrp();
    }
  if (msg.null()) {
      cerr << "MsgMemoryGrp(const RefKeyVal&): couldn't find MessageGrp"
           << endl;
      abort();
    }

  msg_ = msg;
  n_ = msg->n();
  me_ = msg->me();
}

MsgMemoryGrp::~MsgMemoryGrp()
{
}

void
MsgMemoryGrp::set_localsize(int localsize)
{
  delete[] offsets_;

  offsets_ = new distsize_t[n_ + 1];
  int *sizes = new int[n_];

  int i;
  for (i=0; i<n_; i++) sizes[i] = 0;
  sizes[me_] = localsize;

  msg_->sum(sizes, n_);

  offsets_[0] = 0;
  for (i=1; i<=n_; i++) {
      offsets_[i] = sizes[i-1] + offsets_[i-1];
      if (offsets_[i] < offsets_[i-1]) {
          ExEnv::out() << "MsgMemoryGrp::set_localsize: distsize_t cannot handle biggest size" << endl;
          abort();
        }
    }

  delete[] sizes;
}

void
MsgMemoryGrp::sync()
{
  msg_->sync();
}

#endif

/////////////////////////////////////////////////////////////////////////////

// Local Variables:
// mode: c++
// c-file-style: "CLJ"
// End:
