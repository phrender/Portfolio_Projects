#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED
#pragma once

#include <algorithm>

template <typename Type>
class List
{
	private:

		struct Node
		{
			Type m_data;
			Node* m_pxPrevious;
			Node* m_pxNext;

			Node(const Type & data = Type{}, Node* pPrev = nullptr, Node* pNext = nullptr) : m_data{data}, m_pxPrevious{pPrev}, m_pxNext{pNext}
			{
			};

			Node(Type&& data, Node* pPrev = nullptr, Node* pNext = nullptr) : m_data{std::move(data)}, m_pxPrevious{pPrev}, m_pxNext{pNext} 
			{
			};

		}; // Node

	public:

		class ConstIterator
		{
			public:

				ConstIterator() : m_pxCurrent(nullptr)
				{
				};

				const Type& operator*() const
				{
					return Retrive();
				};

				ConstIterator& operator++()
				{
					m_pxCurrent = m_pxCurrent->m_pxNext;
					return *this;
				}

				ConstIterator operator++(int)
				{
					ConstIterator old = *this;
					++(*this);
					return old;
				};

				ConstIterator& operator--()
				{
					m_pxCurrent = m_pxCurrent->m_pxPrevious;
					return *this;
				};

				ConstIterator& operator--(int)
				{
					ConstIterator old = *this:
					--(*this);
					return old:
				};

				bool operator==(const ConstIterator& kConstIterator)
				{
					return m_pxCurrent == kConstIterator.m_pxCurrent;
				};

				bool operator!=(const ConstIterator& kConstIterator)
				{
					return !(*this == kConstIterator);
				};

			protected:

				Type& Retrive() const
				{
					return m_pxCurrent->m_data;
				};

				ConstIterator(Node* pNode) : m_pxCurrent(pNode)
				{
				};

			protected:

				Node* m_pxCurrent;

				friend class List<Type>;

		}; // ConstIterator

		class Iterator : public ConstIterator
		{

			public:

				Iterator()
				{
				};

				Type& operator*()
				{
					return ConstIterator::Retrive();
				};

				Iterator& operator++()
				{
					this->m_pxCurrent = this->m_pxCurrent->m_pxNext;
					return *this;
				}

				const Type& operator++(int)
				{
					Iterator old = *this;
					++(*this);
					return old;
				};

				Iterator& operator--()
				{
					this->m_pxCurrent = this->m_pxCurrent->m_pPrev;
					return *this;
				};

				const Type& operator--(int)
				{
					Iterator old = *this;
					--(*this);
					return old;
				};

			protected:

				Iterator(Node* pNode) : ConstIterator(pNode)
				{
				};

				friend class List<Type>;

		}; // Iterator

	public:

		List()
		{
			Initialize();
		};

		List(List&& kList) : m_iSize(kList.m_iSize), m_pxHead(kList.m_pxHead), m_pxTail(kList.m_pxTail)
		{
			kList.m_iSize = 0;
			kList.m_pxHead = nullptr;
			kList.m_pxTail = nullptr;
		}

		~List()
		{
			Clear();
			delete m_pxHead;
			delete m_pxTail;
		};

		List& operator=(const List& kList)
		{
			List copy = kList;
			std::swap(*this, copy);
			return *this;
		};

		List& operator=(List&& kList)
		{
			std::swap(m_iSize, kList.m_iSize);
			std::swap(m_pxHead, kList.m_pxHead);
			std::swap(m_pxTail, kList.m_pxTail);

			return *this;
		};

		Iterator Begin()
		{
			return Iterator(m_pxHead->m_pxNext);
		};

		ConstIterator Begin() const
		{
			return ConstIterator(m_pxHead->m_pxNext);
		};

		Iterator End()
		{
			return Iterator(m_pxTail);
		};

		ConstIterator End() const
		{
			return ConstIterator(m_pxTail);
		};

		int Size() const
		{
			return m_iSize;
		};

		bool Empty() const
		{
			return Size() == 0;
		};

		void Clear()
		{
			while (!Empty())
			{
				PopFront();
			};
		};

		Type& Front()
		{
			return *Begin();
		};

		const Type& Front() const
		{
			return *Begin();
		};

		Type& Back()
		{
			return *--End();
		};

		const Type& Back() const
		{
			return *--End();
		};

		void PushFront(const Type& x)
		{
			Insert(Begin(), x);
		};

		void PushBack(const Type& x)
		{
			Insert(End(), x);
		};

		void PushFront(Type&& x)
		{
			Insert(Begin(), std::move(x));
		};

		void PushBack(Type&& x)
		{
			Insert(End(), std::move(x));
		};

		void PopFront()
		{
			Erase(Begin());
		};

		void PopBack()
		{
			Erase(--End());
		};

		Iterator Insert(Iterator itr, const Type& x)
		{
			Node *p = itr.m_pxCurrent;
			++m_iSize;
			return Iterator(p->m_pxPrevious = p->m_pxPrevious->m_pxNext = new Node{ x, p->m_pxPrevious, p });
		};

		Iterator Insert(Iterator itr, Type&& x)
		{
			Node *p = itr.m_pxCurrent;
			++m_iSize;
			return Iterator(p->m_pxPrevious = p->m_pxPrevious->m_pxNext = new Node{ std::move(x), p->m_pxPrevious, p });
		};

		Iterator Erase(Iterator itr)
		{
			Node *p = itr.m_pxCurrent;
			Iterator retVal(p->m_pxNext);
			p->m_pxPrevious->m_pxNext = p->m_pxNext;
			p->m_pxNext->m_pxPrevious = p->m_pxPrevious;
			delete p;
			--m_iSize;

			return retVal;
		};

		Iterator Erase(Iterator from, Iterator to)
		{
			for (Iterator itr = from; itr != to; )
				itr = Erase(itr);

			return to;
		};

	private:

		int m_iSize;
		Node* m_pxHead;
		Node* m_pxTail;

		void Initialize()
		{
			m_iSize = 0;
			m_pxHead = new Node;
			m_pxTail = new Node;
			m_pxHead->m_pxNext = m_pxTail;
			m_pxTail->m_pxPrevious = m_pxHead;
		}

}; // List

#endif // LIST_H_INCLUDED