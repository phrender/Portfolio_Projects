#ifndef VECTOR_H_INCLUDED
#define VECTOR_H_INCLUDED
#pragma once

#include <algorithm>
#include <iostream>

template <typename Type>
class Vector
{

	public:

		explicit Vector(int iInitSize = 0) : m_iSize(iInitSize), m_iCapacity(iInitSize + s_kiSpareCapacity)
		{
			m_pxObjects = new Type[m_iCapacity];
		};

		Vector(const Vector& kVectorCopy) : m_iSize(kVectorCopy.m_iSize), m_iCapacity(kVectorCopy.m_iCapacity), m_pxObjects(nullptr)
		{
			m_pxObjects = new Type[m_iCapacity];
			for (int i = 0; i < m_iSize; ++i)
			{
				m_pxObjects[i] = kVectorCopy.m_pxObjects[i];
			};
		};

		Vector& operator=(const Vector& kVector)
		{
			Vector copyVector = kVector;
			std::swap(*this, copyVector);

			return *this;
		};

		~Vector()
		{
			delete[] m_pxObjects;
		};

		Vector(Vector&& kVector) : m_iSize(kVector.m_iSize), m_iCapacity(kVector.m_iCapacity), m_pxObjects(kVector.m_pxObject)
		{
			kVector.m_pxObjects = nullptr;
			kVector.m_iSize = 0;
			kVector.m_iCapacity = 0;
		};

		Vector& operator=(Vector&& kVector)
		{
			std::swap(m_iSize, kVector.m_iSize);
			std::swap(m_iCapacity, kVector.m_iCapacity);
			std::swap(m_pxObjects, kVector.m_pxObjects);

			return *this;
		};

		bool Empty()
		{
			return m_iSize == 0;
		};

		int Size()
		{
			return m_iSize;
		};

		int Capacity()
		{
			return m_iCapacity;
		};

		Type& operator[](int iPosition)
		{
			return m_pxObjects[iPosition];
		};

		const Type& operator[](int iPosition) const
		{
			return m_pxObjects[iPosition];
		};

		void Resize(int iNewSize)
		{
			if (iNewSize > m_iCapacity)
			{
				Reserve(iNewSize * 2);
			};
			m_iSize = iNewSize;
		};

		void Reserve(int iNewCapacity)
		{
			if (iNewCapacity < m_iSize)
			{
				return;
			};

			Type* pNewArray = new Type[iNewCapacity];
			for (int i = 0; i < m_iSize; ++i)
			{
				pNewArray[i] = std::move(m_pxObjects[i]);
			};

			m_iCapacity = iNewCapacity;
			std::swap(m_pxObjects, pNewArray);

			delete[] pNewArray;
		};

		void PushBack(const Type& kObject)
		{
			if (m_iSize == m_iCapacity)
			{
				Reserve(2 * m_iCapacity + 1);
			};

			m_pxObjects[m_iSize++] = kObject;
		};

		void PushBack(Type&& object)
		{
			if (m_iSize == m_iCapacity)
			{
				Reserve(2 * m_iCapacity + 1);
			};
			m_pxObjects[m_iSize++] = object;
		};

		void PopBack()
		{
			--m_iSize;
		};

		const Type& Back() const
		{
			return m_pxObjects[m_iSize];
		};

		typedef Type* pIterator;
		typedef const Type* pConstIterator;

		pIterator Begin()
		{
			return &m_pxObjects[0];
		};

		pConstIterator Begin() const
		{
			return &m_pxObjects[0];
		};

		pIterator End()
		{
			return &m_pxObjects[Size()];
		};

		pConstIterator End() const
		{
			return &m_pxObjects[Size()];
		};

	public:

		static const int s_kiSpareCapacity = 2;

	private:

		int m_iSize;
		int m_iCapacity;
		Type* m_pxObjects;

}; // Vector

#endif // VECTOR_H_INCLUDED