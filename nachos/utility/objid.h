/*! \file objid.h
    \brief Object identifier data structure
   
    Nachos stores a data structure associating object ids with
    their pointers in the kernel address space. The ObjId class
    allows to maintain this data structure.
 
 Copyright (c) 2010-2011 university of Rennes 1.
  
*/

#ifndef OBJID_H
#define OBJID_H

#include "kernel/copyright.h"
#include "utility/utility.h"
#include "utility/list.h"
#include <map>

using namespace std;

/*! \brief Definition of object identifiers
//
// The ObjId class defines a set of object identifiers.  By object, we
// mean every Nachos object (Process, Thread, Semaphore, Lock; etc).
// The class stores the list of created objects and for each of them
// associates an object identifier than can be passed to subsequent
// system calls on the object.
// 
// A method allows to detect of an object corresponding to a given
// identifier exists; this is used to check the parameters of system
// calls.
*/
class ObjId {
 private:
  int last_id;
  map<const int32_t,void *> ids;
 public:
  //----------------------------------------------------------------------
  // ObjId::ObjId
  /*!      Insert an "item" into a list, so that the list elements are
  //	sorted in increasing order by "sortKey".
  //      
  //	Allocate a ListElement to keep track of the item.
  //      If the list is empty, then this will be the only element.
  //	Otherwise, walk through the list, one element at a time,
  //	to find where the new item should be placed.
  //
  //	\param item is the thing to put on the list, it can be a pointer to 
  //		anything.
  //	\param sortKey is the priority of the item.
  //    \ 
  */
  //----------------------------------------------------------------------  
  ObjId() {last_id = 3; /* 0, 1 and 2 used for file descriptors */}
  ~ObjId() {ids.clear();};
  int32_t AddObject(void *ptr) {
    int32_t res = last_id++;
    ids[res] = ptr;
    return res;
  }
  void *SearchObject(int32_t id) {
    return ids[id];
  }
  void RemoveObject(int32_t id) {
    ids.erase(id);
  }
};

#endif // OBJID_H
