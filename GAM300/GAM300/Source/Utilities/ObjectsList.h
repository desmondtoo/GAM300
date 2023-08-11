#ifndef OBJECTS_LIST_H
#define OBJECTS_LIST_H
#include "SparseSet.h"
#include <bitset>

using ObjectIndex = size_t;

template <typename T, ObjectIndex N>
class ObjectsList
{
    struct Node
    {
        SparseSet<T, N> sparseSet;
        std::bitset<N> activeObjectsBitset;
        Node* next = nullptr;
    };
    class Iterator
    {

        ObjectsList<T, N>& arr;
        size_t index;
        friend class ObjectsList;
    public:
        /***************************************************************************/
        /*!
            \brief
                Constructor for iterator
            \param _sparseIndex
                Index of element
            \param _arr
                Referenced sparse set
        */
        /**************************************************************************/
        Iterator(size_t _index, ObjectsList<T, N>& _arr) : index(_index), arr{ _arr } {}

        /***************************************************************************/
        /*!
            \brief
                Dereferencing operator to get value in it
            \return
                Reference to object stored
        */
        /**************************************************************************/
        T& operator*() const
        {
            size_t whichNode = index / N;
            size_t sparseIndex = index % N;
            Node* start = head;
            for (size_t i = 0; i < whichNode; ++i, head = head->next);
            return head->sparseSet[sparseIndex];
        }

        /***************************************************************************/
         /*!
            \brief
                Gets the next iterator
            \return
                Next iterator
        */
        /**************************************************************************/
        Iterator operator++() {
            ++index;
            return *this;
        }

        /***************************************************************************/
        /*!
            \brief
                Increments this iterator and but returns the current iteration
            \return
                this iterator
        */
        /**************************************************************************/
        Iterator operator++(int) {
            Iterator tmp(*this);
            operator++();
            return tmp;
        }

        /***************************************************************************/
        /*!
            \brief
                Checks if two iterators are the same
            \param other
                Other iterator to compare with
            \return
                True if both iterators are of the same sparse index
        */
        /**************************************************************************/
        bool operator==(const Iterator& other) const {
            return index == other.index;
        }

        /***************************************************************************/
        /*!
            \brief
                Checks if two iterators are NOT the same
            \param other
                Other iterator to compare with
            \return
                True if both iterators are NOT of the same sparse index
        */
        /**************************************************************************/
        bool operator!=(const Iterator& other) const {
            return index != other.index;
        }
    };
public:
	//bool GetActive(ObjectIndex index)
	//{
	//	return activeObjectsBitset.test(index);
	//}

	//void SetActive(ObjectIndex index, bool val)
	//{
	//	activeObjectsBitset.set(index, val);
	//}
    template <typename... Args>
    T& emplace_back(Args&&... args);
    template <typename... Args>
    T& emplace_back(DenseIndex index, Args&&... args);
    void clear();
    void erase(T& val);
    ~ObjectsList();
    Iterator begin() {return Iterator(0, *this);}
    Iterator end() {return Iterator(size_, *this);}
private:
    Node* head = nullptr;
    Node* emptyNodesPool = nullptr;
    size_t size_;
    Node* CreateNode();
    void DeleteNode(Node* prev, Node* pNode);
};	

#include "ObjectsList.cpp"

#endif // !OBJECTS_LIST_H
