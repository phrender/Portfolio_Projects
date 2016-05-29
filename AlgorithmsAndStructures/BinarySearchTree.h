#ifndef BINARYSEARCHTREE_H_INCLUDED
#define BINARYSEARCHTREE_H_INCLUDED
#pragma once

#include <algorithm>

template <typename Comparable>
class BinarySearchTree
{

	public:

		BinarySearchTree() : m_pxRoot{ nullptr }
		{
		};

		BinarySearchTree(BinarySearchTree&& kBinarySearchTree) : m_pxRoot{ nullptr }
		{
			kBinarySearchTree.m_pxRoot = nullptr;
		};

		~BinarySearchTree()
		{
			MakeEmpty();
		};

		BinarySearchTree& operator=(const BinarySearchTree& kBinarySearchTree)
		{
			BinarySearchTree copyBST = kBinarySearchTree;
			std::swap(*this, copyBST);

			return  *this;
		};

		BinarySearchTree& operator=(BinarySearchTree&& binarySearchTree)
		{
			std::swap(m_pxRoot, binarySearchTree.m_pxRoot);
			return *this;
		};

		const Comparable& FindMin() const
		{
			return FindMin(m_pxRoot)->m_element;
		};

		const Comparable& FindMax() const
		{
			return FindMax(m_pxRoot)->m_element;
		};

		bool Contains(const Comparable& kElement) const
		{
			return Contains(kElement, m_pxRoot);
		};

		bool IsEmpty() const
		{
			return m_pxRoot == nullptr;
		};

		void PrintTree(std::ostream& out = std::cout)
		{
			if (IsEmpty())
			{
				out << "Empty tree" << std::endl;
			}
			else
			{
				PrintTree(m_pxRoot, out);
			};
		};

		void MakeEmpty()
		{
			MakeEmpty(m_pxRoot);
		};

		void Insert(const Comparable& kElement)
		{
			Insert(std::move(kElement), m_pxRoot);
		};

		void Remove(Comparable&& element)
		{
			Remove(element, m_pxRoot);
		};

	private:

		struct BinaryNode
		{

			Comparable m_element;
			BinaryNode* m_pxLeft;
			BinaryNode* m_pxRight;

			BinaryNode(const Comparable& element, BinaryNode* pLeftTree, BinaryNode* pRightTree) : m_element{ element }, m_pxLeft{ pLeftTree }, m_pxRight{ pRightTree }
			{
			}

			BinaryNode(Comparable&& element, BinaryNode* pLeftTree, BinaryNode* pRightTree) : m_element{ std::move(element) }, m_pxLeft{ pLeftTree }, m_pxRight{ pRightTree }
			{
			}

		}; // BinaryNode

		BinaryNode* m_pxRoot;

	private:

		void Insert(const Comparable& kElement, BinaryNode*& pTree)
		{
			if (pTree == nullptr)
			{
				pTree = new BinarySearchTree{ kElement, nullptr, nullptr };
			}
			else if (kElement < pTree->m_element)
			{
				Insert(std::move(kElement), pTree->m_pxLeft);
			}
			else if (pTree->m_element < kElement)
			{
				Insert(std::move(kElement), pTree->m_pxRight);
			};
		};

		void Insert(Comparable&& element, BinaryNode*& pTree)
		{
			if (pTree == nullptr)
			{
				pTree = new BinaryNode{std::move(element), nullptr, nullptr};
			}
			else if (element < pTree->m_element)
			{
				Insert(std::move(element), pTree->m_pxLeft);
			}
			else if (pTree->m_element < element)
			{
				Insert(std::move(element), pTree->m_pxRight);
			}
		}

		void Remove(const Comparable& kElement, BinaryNode*& pTree)
		{
			if (pTree == nullptr)
			{
				return;
			}

			if (kElement < pTree->m_element)
			{
				Remove(kElement, pTree->m_pxLeft);
			}
			else if (t->element < x)
			{
				Remove(kElement, pTree->m_pxRight);
			}
			else if (pTree->m_pxLeft != nullptr && pTree->m_pxRight != nullptr)
			{
				pTree->m_element = FindMin(t->m_pxRight)->m_element;
				Remove(pTree->m_element, pTree->m_pxRight);
			}
			else
			{
				BinaryNode *pOldNode = pTree;
				pTree = (pTree->m_pxLeft != nullptr) ? pTree->m_pxLeft : pTree->m_pxRight;
				delete pOldNode;
			};
		};

		BinaryNode* FindMin(BinaryNode* pTree) const
		{
			if (pTree == nullptr)
			{
				return nullptr;
			}

			if (pTree->m_pxLeft == null)
			{
				return pTree;
			}

			return FindMin(pTree->m_pxLeft)
		};

		BinaryNode* FindMAx(BinaryNode* pTree) const
		{
			if (pTree != nullptr)
			{
				while (pTree->m_pxRight != nullptr)
				{
					pTree = pTree->m_pxRight;
				};
			};

			return pTree;
		};

		bool Contains(const Comparable& kElement, BinaryNode* pTree)
		{
			if (pTree == nullptr)
			{
				return false;
			}
			else if (kElement < pTree->m_element)
			{
				return Contains(kElement, pTree->m_pxLeft);
			}
			else if (pTree->m_element < kElement)
			{
				return Contains(kElement, pTree->m_pxRight);
			}
			else
			{
				return true;
			};
		};

		bool Contains(const Comparable& kElement, BinaryNode* pTree) const
		{
			while (pTree != nullptr)
			{
				if (kElement < pTree->m_element)
				{
					pTree = pTree->m_pxLeft;
				}
				else if (pTree->m_element < kElement)
				{
					pTree = pTree->m_pxRight;
				}
				else
				{
					return true;
				};
			};

			return false;
		};

		void MakeEmpty(BinaryNode*& pTree)
		{
			if (pTree != nullptr)
			{
				MakeEmpty(pTree->m_pxLeft);
				MakeEmpty(pTree->m_pxRight);
				delete pTree;
			};

			pTree = nullptr;
		};

		void PrintTree(BinaryNode* pTree, std::ostream& out)
		{
			if (pTree != nullptr)
			{
				PrintTree(pTree->m_pxLeft, out);
				out << pTree->m_element << std::endl;
				PrintTree(pTree->m_pxRight, out);
			};
		};

		BinaryNode* Clone(BinaryNode* pTree) const
		{
			if (pTree == nullptr)
			{
				return nullptr;
			}
			else
			{
				return new BinaryNode{ pTree->m_element, Clone(pTree->m_pxLeft), Clone(pTree->m_pxRight) };
			};
		};
};

#endif // BINARYSEARCHTREE_H_INCLUDED