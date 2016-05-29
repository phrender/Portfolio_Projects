#include <iostream>

#include "Vector.h"
#include "List.h"
#include "BinarySearchTree.h"

int main()
{
	BinarySearchTree<int> myTree = BinarySearchTree<int>();

	myTree.Insert(5);
	myTree.Insert(7);
	myTree.Insert(3);
	myTree.Insert(2);
	myTree.Insert(1);

	myTree.PrintTree();

	return 0;
};