/*
 * MedianFilter.hpp
 *
 *  Created on: May 20, 2018
 *      Author: alexandru.bogdan
 */

#ifndef MEDIANFILTER_HPP_
#define MEDIANFILTER_HPP_

#include <type_traits>

template <typename type, unsigned int numElements>
class MedianFilter
{
	static_assert(std::is_arithmetic<type>::value, "type must be POD");
	static_assert(numElements % 2, "Number of elements must be ODD");
	static_assert(numElements > 2, "Minimum 3 elements required");

	typedef struct sMedianNode
	{
		type value;
		struct sMedianNode *nextAge;
		struct sMedianNode *nextValue;
		struct sMedianNode *prevValue;
	}sMedianNode_t;

	sMedianNode_t m_medianBuffer[numElements];
	sMedianNode_t *m_ageHead;
	sMedianNode_t *m_valueHead;
	sMedianNode_t *m_medianHead;

public:
	MedianFilter();
	virtual ~MedianFilter() { };

	type Insert(type value);
};

template<typename type, unsigned int numElements>
MedianFilter<type, numElements>::MedianFilter()
{
	for(unsigned int i = 0; i < numElements; i++)
	{
		m_medianBuffer[i].value = 0;
		m_medianBuffer[i].nextAge = &m_medianBuffer[(i + 1) % numElements];
		m_medianBuffer[i].nextValue = &m_medianBuffer[(i + 1) % numElements];
		m_medianBuffer[(i + 1) % numElements].prevValue = &m_medianBuffer[i];
	}

	m_ageHead = m_medianBuffer;
	m_valueHead = m_medianBuffer;
	m_medianHead = &m_medianBuffer[numElements / 2];
}

template<typename type, unsigned int numElements>
type MedianFilter<type, numElements>::Insert(type value)
{
	unsigned int i;
	sMedianNode_t *newNode, *it;

	if(m_ageHead == m_valueHead)
	{
		m_valueHead = m_valueHead->nextValue;
	}

	if((m_ageHead == m_medianHead) || (m_ageHead->value > m_medianHead->value))
	{
		m_medianHead = m_medianHead->prevValue;
	}

	newNode = m_ageHead;
	newNode->value = value;

	m_ageHead->nextValue->prevValue = m_ageHead->prevValue;
	m_ageHead->prevValue->nextValue = m_ageHead->nextValue;
	m_ageHead = m_ageHead->nextAge;

	it = m_valueHead;
	for(i = 0; i < numElements - 1; i++)
	{
		if(value < it->value)
		{
			if(i == 0)
			{
				m_valueHead = newNode;
			}
			break;
		}
		it = it->nextValue;
	}

	it->prevValue->nextValue = newNode;
	newNode->prevValue = it->prevValue;
	it->prevValue = newNode;
	newNode->nextValue = it;

	if(i >= (numElements / 2))
	{
		m_medianHead = m_medianHead->nextValue;
	}

	return m_medianHead->value;
}

#endif /* MEDIANFILTER_HPP_ */
