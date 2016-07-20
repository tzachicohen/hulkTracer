

#ifndef __MEMORYUTILS_VECTOR_HPP__
#define __MEMORYUTILS_VECTOR_HPP__

#include "util.hpp"
/* 
A template Stack.
it is recommended for class T to be a simple class 
(IE no complex "default constructor"/"copy constructor"/"destructor"/"operator=")
the Stack uses an expanding array for implementation, 
therefore you may not use pointers to the array after changing the elements
*/
template<class T> class Stack 
{
public:
	static const unsigned int  INITIAL_SIZE = 16;
	Stack(unsigned int  initiailSize = INITIAL_SIZE);
	Stack(const Stack& other);
	Stack& operator=(const Stack& other);
	~Stack();

	__forceinline unsigned int  size() const {return m_size;}
	__forceinline unsigned int  capacity() const {return m_capacity;}
	__forceinline T* data() {return m_array;}
	__forceinline const T* data() const {return m_array;}
	__forceinline T& operator[](unsigned int  index) {return m_array[index];}
	__forceinline const T& operator[](unsigned int  index) const {return m_array[index];}
	__forceinline void pop_back() {m_size--;}
	__forceinline void push_back(const T& obj) {*push_back() = obj;}
	__forceinline T* getBackObject() ;
	__forceinline void clear() {m_size = 0;}
	__forceinline bool isEmpty() const {return m_size == 0;}
	//expand the size of the buffer .
	void resize(unsigned int newSize);
	void setZeros() { memset(m_array,0,sizeof(T)*m_size);}
	//expand the capacity of the array
	void reserve(unsigned int newCapacity);


	class iterator 
	{
	public:
		iterator():m_vec(NULL), m_pos(NULL) {}
		iterator(Stack& vec,unsigned int place = 0): m_vec(&vec), m_pos(m_vec->m_array + place) {}
		iterator(iterator & other): m_vec(toher.m_vec), m_pos(other.m_pos){}
		~iterator() {}
		iterator& operator++() {m_pos++; return *this;}
		iterator& operator=(const iterator & other)
		{ m_vec = other.m_vec; m_pos = other.m_pos; return *this;}
		bool operator==(const iterator& other) {return m_pos == other.m_pos;}
		bool operator!=(const iterator& other) {return m_pos != other.m_pos;}
		T& operator*() {return *m_pos;}
		T* operator->() {return m_pos;}
        unsigned int getIndex() { return m_pos - m_vec->m_array; }
	private:
		Stack* m_vec;
		T* m_pos;
	};

	class const_iterator 
	{
	public:
		const_iterator(const Stack& vec,unsigned int place = 0): m_vec(vec), m_pos(m_vec.m_array + place) {}
		~const_iterator() {}
		const_iterator& operator++() {m_pos++; return *this;}
		bool operator==(const const_iterator& other) {return m_pos == other.m_pos;}
		bool operator!=(const const_iterator& other) {return m_pos != other.m_pos;}
		const T& operator*() const {return *m_pos;}
		const T* operator->() const {return m_pos;}
        unsigned int getIndex() { return m_pos - m_vec.m_array; }
	private:
		const Stack& m_vec;
		T*  m_pos;
	};

	__forceinline const_iterator begin() const {return const_iterator(*this);}
	__forceinline const_iterator end() const {return const_iterator(*this,m_size);}
	__forceinline iterator begin() {return iterator(*this);}
	__forceinline iterator end() {return iterator(*this,m_size);}
	__forceinline iterator getIteratorAtIndex(unsigned int  index) {return iterator(*this,index);}

private:
	__forceinline T* push_back();
	T* allocArray(unsigned int  size);
	void expandCapacity();

	T* m_array;
	unsigned int  m_size;
	unsigned int  m_capacity;
};

template<class T> Stack<T>::Stack(unsigned int  initiailSize) 
{
	m_capacity = initiailSize; 
	m_array = allocArray(m_capacity);
	m_size = 0;
}

template<class T> Stack<T>::Stack(const Stack<T>& other) 
{
	if (other.m_size != 0) 
	{		
		m_capacity = other.m_size;
	}
	else 
	{
		m_capacity = INITIAL_SIZE;
	}
	m_array = allocArray(m_capacity);
	m_size = other.m_size;
	for (unsigned int  i = 0; i < other.m_size; i++) 
	{
		m_array[i] = other.m_array[i];
	}
}

template<class T> Stack<T>& Stack<T>::operator=(const Stack<T>& other) 
{
	if (this == &other) return *this;

	if (m_capacity < other.m_size)
	{
		SAFE_ARRAY_DELETE(m_array);
		m_capacity = other.m_size;
		m_array = allocArray(m_capacity);
	}	
	m_size = other.m_size;
	for (unsigned int  i = 0; i < other.m_size; i++) 
	{
		m_array[i] = other.m_array[i];
	}
	return *this;
}

template<class T> Stack<T>::~Stack()
{
	SAFE_ARRAY_DELETE(m_array);
}

template<class T> T*  Stack<T>::getBackObject() 
{
	if (0 == m_size) 
		return NULL;
	else
		return &m_array[m_size-1];
}

template<class T> T* Stack<T>::push_back()
{
	if (m_size == m_capacity) {
		expandCapacity();			
	}
	T* res = &m_array[m_size];
	m_size++;
	return res;
}

template<class T> void Stack<T>::resize(unsigned int newSize)
{
	if (newSize > m_size) {
		T* newArray = allocArray(newSize);
		for (unsigned int  i = 0; i < min(m_size,newSize); i++) {
			newArray[i] = m_array[i];
		}
		SAFE_ARRAY_DELETE(m_array);
		m_array = newArray;
		m_capacity = newSize;
	}
	m_size = newSize;
}

template<class T> void Stack<T>::reserve(unsigned int newCapacity)
{
	if (newCapacity > m_capacity) {
		T* newArray = allocArray(newCapacity);
		for (unsigned int  i = 0; i < m_size; i++) {
			newArray[i] = m_array[i];
		}
		SAFE_ARRAY_DELETE(m_array);
		m_array = newArray;
		m_capacity = newCapacity;
	}

}
template<class T> void Stack<T>::expandCapacity()
{
	unsigned int  newCapacity = m_capacity == 0 ? INITIAL_SIZE : m_capacity * 2;
	T* newArray = allocArray(newCapacity);
	for (unsigned int  i = 0; i < m_size; i++) {
		newArray[i] = m_array[i];
	}
	SAFE_ARRAY_DELETE(m_array);
	m_array = newArray;
	m_capacity = newCapacity;
}

template <class T> T* Stack<T>::allocArray(unsigned int  size) 
{
	if (size == 0) return NULL;
	return new T[size];
}


#endif // __MEMORYUTILS_VECTOR_HPP__
