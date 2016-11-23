/*! \file list.h
    \brief Data structures to manage LISP-like lists.  
   
    As in LISP, a list can contain any type of data structure
    as an item on the list: thread control blocks, 
    pending interrupts, etc.  That is why each item is a "void *",
    or in other words, a "pointers to anything".
  
 Copyright (c) 1992-1993 The Regents of the University of California.
 All rights reserved.  See copyright.h for copyright notice and limitation 
 of liability and disclaimer of warranty provisions.
  
*/

#ifndef LIST_H
#define LIST_H

#include "kernel/copyright.h"
#include "utility/utility.h"

/*! \brief definition of a "list element"
   
    The class defines a "list element" which is used to keep track of
    one item on a list.  It is equivalent to a LISP cell, with a "car"
    ("next") pointing to the next element on the list, and a "cdr"
    ("item") pointing to the item on the list.
 
    Internal data structures kept public so that List operations can
    access them directly.
*/
template <class Priority>
class ListElement {
public:
  
  //! Next element on list, NULL if this is the last.
  ListElement <Priority> *next;
  
  //! Priority, for a sorted list.		
  Priority key;
  
  //! Pointer to item on the list.		    	
  void *item; 
  
  //----------------------------- ----------------------------------------
  // ListElement::ListElement
  /*! 	Initializes a list element, so it can be added somewhere on a list.
    
  \param itemPtr is the item to be put on the list.  It can be a pointer
  to anything.
  \param sortKey is the priority of the item, if any.
  */
  //----------------------------------------------------------------------
  ListElement(void *itemPtr, Priority sortKey){
    item = itemPtr;
    key = sortKey;
    next = NULL;	// assume we'll put it at the end of the list 
  };    	
};

/*! \brief Definition of a generic single-linked "list" 
//
// The following class defines a "list" -- a singly linked list of
// list elements, each of which points to a single item on the list.
// By using the "Sorted" functions, the list can be kept in sorted in
// increasing order by "key" in ListElement.
*/
template <class Priority>
class List {
  public:

  //----------------------------------------------------------------------
  // List::List
  /*!	Initialize a list, empty to start with.
  //	Elements can now be added to the list.
  */
  //----------------------------------------------------------------------
  List(){ 
    first = last = NULL; 
  };
  
  //----------------------------------------------------------------------
  // List::~List
  /*!	Prepare a list for deallocation.  If the list still contains any 
  //	ListElements, de-allocate them.  However, note that we do *not*
  //	de-allocate the "items" on the list -- this module allocates
  //	and de-allocates the ListElements to keep track of each item,
  //	but a given item may be on multiple lists, so we can't
  //	de-allocate them here.
  */
  //----------------------------------------------------------------------   
  ~List(){ 
    while (Remove() != NULL)
      ;	 // delete all the list elements
  };

  //----------------------------------------------------------------------
  // List::Prepend
  /*!      Put an "item" on the front of the list.
  //      
  //	Allocate a ListElement to keep track of the item.
  //      If the list is empty, then this will be the only element.
  //	Otherwise, put it at the beginning.
  //
  //	\param item is the thing to put on the list, it can be a pointer to 
  //		anything.
  */
  //----------------------------------------------------------------------
  void Prepend(void *item){
    ListElement<Priority> *element = new ListElement<Priority>(item, 0);
    
    if (IsEmpty()) {		// list is empty
      first = element;
      last = element;
    } else {			// else put it before first
      element->next = first;
      first = element;
    }
  };
    
 
  //----------------------------------------------------------------------
  // List::Append
  /*!      Append an "item" to the end of the list.
  //      
  //	Allocate a ListElement to keep track of the item.
  //      If the list is empty, then this will be the only element.
  //	Otherwise, put it at the end.
  //
  //	\param item is the thing to put on the list, it can be a pointer to 
  //		anything.
  */
  //---------------------------------------------------------------------- 
  void Append(void *item){
    
    ListElement<Priority> *element = new ListElement<Priority>(item,(Priority) 0);
    
    if (IsEmpty()) {		// list is empty
      first = element;
      last = element;
    } else {			// else put it after last
      last->next = element;
      last = element;
    }
  }; 
    
  //----------------------------------------------------------------------
  // List::Remove
  /*!      Remove the first "item" from the front of the list.
  // 
  // \return
  //	Pointer to removed item, NULL if nothing on the list.
  */
  //----------------------------------------------------------------------
  void *Remove(){
    return SortedRemove(NULL);  // Same as SortedRemove, but ignore the key
  }
  ;
 
  //----------------------------------------------------------------------
  /*! List::Mapcar
  //	Apply a function to each item on the list, by walking through  
  //	the list, one element at a time.
  //
  //	Unlike LISP, this mapcar does not return anything!
  //
  //	\param func is the procedure to apply to each element of the list.
  */
  //----------------------------------------------------------------------   
  void Mapcar(VoidFunctionPtr func){
    for (ListElement<Priority> *ptr = first; ptr != NULL; ptr = ptr->next) {
      DEBUG('l', (char*)"In mapcar, about to invoke %x(%x)\n", func, ptr->item);
      (*func)((int64_t)ptr->item);
    }
  };	
  
  //----------------------------------------------------------------------
  // List::IsEmpty
  //!      \return TRUE if the list is empty (has no items).
  //----------------------------------------------------------------------   
  bool IsEmpty(){ 
    if (first == NULL)
      return true;
    else
      return false; 
  };	
 
  //----------------------------------------------------------------------
  // List::SortedInsert
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
  */
  //----------------------------------------------------------------------  
  void SortedInsert(void *item, Priority sortKey){
    ListElement<Priority> *element = new ListElement<Priority>(item, sortKey);
    
    ListElement<Priority> *ptr;		// keep track
    
    if (IsEmpty()) {	// if list is empty, put
      first = element;
      last = element;
    } else if (sortKey < first->key) {	
      // item goes on front of list
      element->next = first;
      first = element;
    } else {		// look for first elt in list bigger than item
      for (ptr = first; ptr->next != NULL; ptr = ptr->next) {
	if (sortKey < ptr->next->key) {
	  element->next = ptr->next;
	  ptr->next = element;
	  return;
	}
      }
      last->next = element;		// item goes at end of list
      last = element;
    }
    
  };	
    
  
  //----------------------------------------------------------------------
  // List::SortedRemove
  /*!      Remove the first "item" from the front of a sorted list.
  // 
  // \return
  //	Pointer to removed item, NULL if nothing on the list.
  //	Sets *keyPtr to the priority value of the removed item
  //	(this is needed by interrupt.cc, for instance).
  //
  //  \param keyPtr is a pointer to the location in which to store the 
  //	priority of the removed item.
  */
  //----------------------------------------------------------------------    
  void *SortedRemove(Priority *keyPtr){
    ListElement<Priority> *element = first;
    void *thing;
    
    if (IsEmpty()) 
      return NULL;
    
    ASSERT(first != first->next);
    
    thing = first->item;
    if (first == last) {	// list had one item, now has none 
      first = NULL;
      last = NULL;
    } else {
      first = element->next;
    }
    if (keyPtr != NULL)
      *keyPtr = element->key;
    delete element;
    return thing;
  }; 	  

  //----------------------------------------------------------------------
  // List::Search
  /*!      Search for an element in the list
  // 
  // \return
  //	true if the elementn is found, false otherwise
  //
  // \param 
  //    item: pointer to the element we want to find
  */
  //----------------------------------------------------------------------    
  bool Search(void *item){
    ListElement<Priority> *element = first;
      
      if (IsEmpty()) {
	return false;
      }
      else {   
	while ((element != last) && (element->item != item)) {
	  element = element->next;
	}
	return (element->item == item);
      }
    } ;
    
  //----------------------------------------------------------------------
  // List::RemoveItem
  /*!      Remove the specified item from the list if present
  // 
  // \return
  //	none
  //
  // \param 
  //    item: pointer to the element we want to remove
  */
  //----------------------------------------------------------------------    
  void RemoveItem(void *item){
    void *temp;
    while ((temp=this->Remove()) != item) this->Append(temp);
  };

  //----------------------------------------------------------------------
  // List::getFirst
  /*!      Return the first element of a list
  // 
  // \return
  //	First element
  //
  // \param 
  //    none
  */
  //----------------------------------------------------------------------    
   ListElement<Priority> *getFirst(void) {return first;}

private:
  //! Head of the list, NULL if list is empty.
  ListElement<Priority> *first;
  //! Last element of list.  
  ListElement<Priority>  *last;		
};

typedef List<int> Listint;
typedef List<Time> ListTime;

#endif // LIST_H
