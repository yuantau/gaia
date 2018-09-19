#pragma once
#include "def.h"

namespace Gaia
{
	class ICode;

	template<class T>
	class List;

	/************************************************************************/
	/* 链表节点                                                             */
	/* 节点元素为堆上分配,节点删除时同时删除元素                            */
	/************************************************************************/
	template<class NodeType>
	class ListNode
	{
	public:
		ListNode():pData_(NULL), pNext_(NULL){
		
		};
		~ListNode()
		{
			
			if (pData_)
			{
				delete pData_;
			}
		};
		NodeType getData()
		{
			return pData_;
		}
		ListNode *getNext()
		{
			return pNext_;
		}
		friend class List<NodeType>;
	private:
		int id;
		NodeType pData_;
		ListNode *pNext_;
	};

	/************************************************************************/
	/* 定制的单向链表类                                                     */
	/* 添加的节点元素都是堆上的，由链表维护，负责删除                       */
	/************************************************************************/
	template<class T>
	class List
	{
	public:
		List();
		~List();
	public:
		//在链表末尾增加一个节点
		int push(T pData);		

		//在链表末尾增加一个节点
		//节点元素为参数对象的副本
		int push(T pData, size_t size);

		//删除链表末尾节点
		void pop();

		//获取末尾节点元素
		T peek();

		//获取任意索引的节点元素
		T getAt(int index);

		//删除指定的节点
		void del(ListNode<T> *pNode);

		//获取头节点
		ListNode<T> *getHead();

		//获取尾节点
		ListNode<T> *getTail();

		//判断链表是否为空
		BOOL isEmpty();

		//获取链表长度
		int getLength();

		//删除所有节点
		void clear();

	private:
		ListNode<T> *pHead_;
		ListNode<T> *pTail_;
		int length_;		//链表长度
	};

	template<class T>
	void List<T>::clear()
	{
		ListNode<T> *pNode = pHead_;
		ListNode<T> *pNext;
		while(pNode)
		{
			pNext = pNode->pNext_;
			delete pNode;
			pNode = pNext;
			length_--;
		}
		pHead_ = pTail_ = NULL;
	}

	template<class T>
	List<T>::List():pHead_(NULL), pTail_(NULL), length_(0)
	{

	}

	template<class T>
	List<T>::~List(void)
	{
		ListNode<T> *pNode = pHead_;
		ListNode<T> *pNext;
		while (pNode)
		{
			
			pNext = pNode->pNext_;
			delete pNode;
			pNode = pNext;
		}
	}

	/************************************************************************/
	/* 获取任意索引位置元素                                                 */
	/************************************************************************/
	template<class T>
	T List<T>::getAt(int index)
	{
		ListNode<T> *pNode = pHead_;
		int i=0;
		while(pNode)
		{
			if (i == index)
				return pNode->pData_;
			i++;
			pNode = pNode->pNext_;
		}
		return NULL;
	}

	/************************************************************************/
	/* 在链表末尾增加一个节点                                               */
	/* T pData:要添加的对象                                                 */
	/* Return:链表长度                                                      */
	/************************************************************************/
	template<class T>
	int List<T>::push(T pData)
	{
		ListNode<T> *pNode = new ListNode<T>();
		
		pNode->pData_ = pData;
		pNode->pNext_ = NULL;
		if (!length_)
			pHead_ = pTail_ = pNode;
		else 
		{
			pTail_->pNext_ = pNode;
			pTail_ = pNode;
		}
		return length_++;
	}

	/************************************************************************/
	/* 在链表末尾增加一个节点,节点元素pData的副本                           */
	/* T pData:要添加的对象                                                 */
	/* size_t size:对象长度                                                 */
	/* Return:链表长度                                                      */
	/************************************************************************/
	template<class T>
	int List<T>::push(T pData, size_t size)
	{
		//复制原对象
		T pNewData = (T)new char[size];
		memcpy(pNewData, pData, size);

		ListNode<T> *pNode = new ListNode<T>();
		pNode->pData_ = pNewData;
		pNode->pNext_ = NULL;
		if (!length_)
			pHead_ = pTail_ = pNode;
		else 
		{
			pTail_->pNext_ = pNode;
			pTail_ = pNode;
		}
		return length_++;
	}

	/************************************************************************/
	/* 删除链表末尾元素                                                     */
	/************************************************************************/
	template<class T>
	void List<T>::pop()
	{
		if (pTail_)
			del(pTail_);
	}

	/************************************************************************/
	/* 获取链表尾节点元素                                                   */
	/************************************************************************/
	template<class T>
	T List<T>::peek()
	{
		if (pTail_)
			return pTail_->pData_;
		return NULL;
	}

	/************************************************************************/
	/* 删除指定的节点                                                       */
	/************************************************************************/
	template<class T>
	void List<T>::del(ListNode<T> *pNode)
	{
		if (!length_) return;
		BOOL bDeled = FALSE;
		if (pNode == pHead_)
		{
			pHead_ = pHead_->pNext_;
			bDeled = TRUE;
		}
		else 
		{
			ListNode<T> *pTravNode = pHead_;
			for (int i=0; i<length_; i++)
			{
				if (pTravNode->pNext_ == pNode)
				{
					if (pTail_ == pNode)
					{
						pTravNode->pNext_ = NULL;
						pTail_ = pTravNode;
					}
					else 
					{
						pTravNode->pNext_ = pNode->pNext_;
					}
					bDeled = TRUE;
					break;
				}
				pTravNode = pTravNode->pNext_;
			}
		}
		length_--;
		delete pNode;
	}

	/************************************************************************/
	/* 获取链表是否为空                                                     */
	/************************************************************************/
	template<class T>
	BOOL List<T>::isEmpty()
	{
		return length_ == 0;
	}

	/************************************************************************/
	/* 获取链表头                                                           */
	/************************************************************************/
	template<class T>
	ListNode<T> *List<T>::getHead()
	{
		return pHead_;
	}

	/************************************************************************/
	/* 获取链表尾                                                           */
	/************************************************************************/
	template<class T>
	ListNode<T> *List<T>::getTail()
	{
		return pTail_;
	}

	/************************************************************************/
	/* 获取链表长度                                                         */
	/************************************************************************/
	template<class T>
	int List<T>::getLength()
	{
		return length_;
	}
}

