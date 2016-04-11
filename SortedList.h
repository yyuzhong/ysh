#ifndef SORTEDLIST_H
#define SORTEDLIST_H

#define MAX_ITEMS 100


template <typename DataType>
class ListNode {
public:
    ListNode(const DataType& nodeData, ListNode* nextPtr, ListNode* prevPtr);
    
    DataType dataItem;
    ListNode* next;
    ListNode* prev;
};


template <typename DataType>
class SortedList{
public: 				//  8 public member functions
	SortedList();  // constructor
	~SortedList(); // destructor
    
	// transformers
    void insert(const DataType& newDataItem);  // insert an item according to the order
    void remove();  // remove the cursor node, and move the cursor to the next node. If the removed node is the last node, move the cursor to the beginning of the list
    void replace(const DataType& newDataItem);  // replace the cursor node value
    void clear();  // clear the list, remove all nodes
    
    bool isEmpty() const;
    bool isFull() const;
    
    void print(ListNode<DataType> *node) const;  // print the list recursively
    void print_rev(ListNode<DataType> *node) const;  // print the list in reverse order recursively
    
    void gotoBeginning();  // move cursor to the beginning of the list
    void gotoEnd();  // move cursor to the end of the list
    bool gotoNext();  // move cursor to the next node, return false if no next node is available
    bool gotoPrior();  // move cursor to the prior node, return false if no prior node is available
    
    DataType getCursor() const;  // return the value of the cursor node
    
    ListNode<DataType>* getHead() const;  // return the pointer of head

private:				//  3 private data members
	int 	length;  // the current number of items in the list
	ListNode<DataType>		*head;
	ListNode<DataType>		*cursor;
};


#endif