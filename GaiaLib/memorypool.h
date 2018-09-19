#pragma once
#include <stdio.h>
namespace Gaia
{
template <class T, int BLOCK_SIZE = 50>
class MemoryPool
{
  public:
	~MemoryPool() {}

	static void *operator new(size_t size)
	{
		if (!allocBuffer)
			allocBlock();
		char *pointer = allocBuffer;
		allocBuffer = *(char **)(pointer);
		printf("return: %p\n", pointer);
		return pointer;
	}

	static void operator delete(void *pointer)
	{
		*(char **)(pointer) = allocBuffer;
		allocBuffer = static_cast<char *>(pointer);
	}

  private:
	static void allocBlock()
	{
		allocBuffer = new char[sizeof(T) * BLOCK_SIZE];
		char **pCurrent = (char **)(allocBuffer);
		char *pNext = allocBuffer;
		for (int i = 0; i < BLOCK_SIZE - 1; i++)
		{
			pNext += sizeof(T);
			*pCurrent = pNext;
			pCurrent = (char **)pNext;
		}
		*pCurrent = 0;
	}

  private:
	static char *allocBuffer;
};

template <class T, int BLOCK_SIZE>
char *MemoryPool<T, BLOCK_SIZE>::allocBuffer;

} // namespace Gaia