/*
  The contents of this file are dedicated by all of its authors, including

    Michael S. Gashler,
    Eric Moyer,
    anonymous contributors,

  to the public domain (http://creativecommons.org/publicdomain/zero/1.0/).

  Note that some moral obligations still exist in the absence of legal ones.
  For example, it would still be dishonest to deliberately misrepresent the
  origin of a work. Although we impose no legal requirements to obtain a
  license, it is beseeming for those who build on the works of others to
  give back useful improvements, or find a way to pay it forward. If
  you would like to cite us, a published paper about Waffles can be found
  at http://jmlr.org/papers/volume12/gashler11a/gashler11a.pdf. If you find
  our code to be useful, the Waffles team would love to hear how you use it.
*/

#ifndef __GPRIORITYQUEUE_H__
#define __GPRIORITYQUEUE_H__

#include <cstddef>
#include <vector>

namespace GClasses {

typedef int (*PointerComparer)(void* pThis, void* pA, void* pB);

/// An internal class used by GPriorityQueue. You should not use this class directly.
class GPriorityQueueEntry
{
public:
	int m_minIndex;
	int m_maxIndex;
	void* m_pPayload;

	GPriorityQueueEntry(int minIndex = 0, int maxIndex = 0, void* pPayload = NULL)
	: m_minIndex(minIndex), m_maxIndex(maxIndex), m_pPayload(pPayload)
	{}

	GPriorityQueueEntry(const GPriorityQueueEntry& other)
	: m_minIndex(other.m_minIndex), m_maxIndex(other.m_maxIndex), m_pPayload(other.m_pPayload)
	{}

	~GPriorityQueueEntry() {}
};


/// An implementation of a double-ended heap-based priority queue. (Note that the
/// multimap STL class can also be used to implement a double-ended priority queue,
/// but the STL does not currently provide a heap-based double-ended priority queue,
/// which is asymptotically more efficient for insertions.)
class GPriorityQueue
{
protected:
	std::vector<GPriorityQueueEntry> m_entries;
	PointerComparer m_pCompareFunc;
	void* m_pThis;

public:
	GPriorityQueue(PointerComparer pCompareFunc, void* pThis);
	~GPriorityQueue();

#ifndef NO_TEST_CODE
	/// Performs unit tests for this class. Throws an exception if there is a failure.
	static void test();
#endif
	/// Returns the number of items in the queue
	int size();

	/// Adds an item to the queue
	void insert(void* pObj);

	/// Returns the min item in the queue
	void* minimum();

	/// Returns the max item in the queue
	void* maximum();

	/// Removes the min item from the queue
	void removeMin();

	/// Removes the max item from the queue
	void removeMax();

protected:
	void minHeapSwap(int a, int b);
	void maxHeapSwap(int a, int b);
	void minHeapBubbleUp(int index);
	void maxHeapBubbleUp(int index);
	void minHeapBubbleDown(int index);
	void maxHeapBubbleDown(int index);
};

} // namespace GClasses

#endif // __GPRIORITYQUEUE_H__
