#pragma once

#include <assert.h>

template <typename KEY, typename VALUE>
class OrderedHash
{
private:
	QVector<VALUE> m_valueVector;
	QVector<KEY> m_keyVector;
	QHash<KEY, int> m_hash;

	void recalcHash();

public:
	OrderedHash();

	virtual void clear();

	bool isEmpty() const;
	int count() const;
	bool contains(const KEY& key) const;

	virtual void append(const KEY& key, const VALUE& value);
	virtual void remove(const KEY& key);
	virtual void removeAt(const int index);

	const VALUE value(const KEY& key) const;

	const KEY key(const int index) const;
	int keyIndex(const KEY& key) const;

	VALUE& operator[](int index);
	const VALUE& operator[](int index) const;

	QList<VALUE> toList() const;
};


template <typename KEY, typename VALUE>
void OrderedHash<KEY, VALUE>::recalcHash()
{
	m_hash.clear();
	for (int i = 0; i < m_keyVector.count(); i++)
	{
		m_hash[m_keyVector[i]] = i;
	}
}


template <typename KEY, typename VALUE>
OrderedHash<KEY, VALUE>::OrderedHash()
{
}


template <typename KEY, typename VALUE>
void OrderedHash<KEY, VALUE>::clear()
{
	m_valueVector.clear();
	m_keyVector.clear();
	m_hash.clear();
}


template <typename KEY, typename VALUE>
bool OrderedHash<KEY, VALUE>::isEmpty() const
{
	return m_valueVector.isEmpty();
}


template <typename KEY, typename VALUE>
int OrderedHash<KEY, VALUE>::count() const
{
	return m_valueVector.count();
}


template <typename KEY, typename VALUE>
bool OrderedHash<KEY, VALUE>::contains(const KEY& key) const
{
	return m_hash.contains(key);
}


template <typename KEY, typename VALUE>
void OrderedHash<KEY, VALUE>::append(const KEY& key, const VALUE& value)
{
	if (m_hash.contains(key))
	{
		int valueIndex = m_hash[key];

		m_valueVector[valueIndex] = value;
		m_keyVector[valueIndex] = key;
	}
	else
	{
		int newValueIndex = m_valueVector.count();

		m_valueVector.append(value);
		m_keyVector.append(key);
		m_hash.insert(key, newValueIndex);
	}
}


template <typename KEY, typename VALUE>
void OrderedHash<KEY, VALUE>::remove(const KEY &key)
{
	int index = m_hash[key];

	m_hash.remove(key);
	m_valueVector.remove(index);
	m_keyVector.remove(index);

	recalcHash();
}


template <typename KEY, typename VALUE>
void OrderedHash<KEY, VALUE>::removeAt(const int index)
{
	m_hash.remove(m_keyVector[index]);
	m_valueVector.removeAt(index);
	m_keyVector.removeAt(index);

	recalcHash();
}


template <typename KEY, typename VALUE>
const VALUE OrderedHash<KEY, VALUE>::value(const KEY& key) const
{
	if (m_hash.contains(key))
	{
		int valueIndex = m_hash[key];
		return m_valueVector[valueIndex];
	}

	assert(false);

	return VALUE();
}


template <typename KEY, typename VALUE>
const KEY OrderedHash<KEY, VALUE>::key(const int index) const
{
	return m_keyVector[index];
}


template <typename KEY, typename VALUE>
int OrderedHash<KEY, VALUE>::keyIndex(const KEY &key) const
{
	if (m_hash.contains(key))
	{
		return m_hash[key];
	}
	return -1;
}


template <typename KEY, typename VALUE>
VALUE& OrderedHash<KEY, VALUE>::operator[](int index)
{
	return m_valueVector[index];
}


template <typename KEY, typename VALUE>
const VALUE& OrderedHash<KEY, VALUE>::operator[](int index) const
{
	return m_valueVector[index];
}


template <typename KEY, typename VALUE>
QList<VALUE> OrderedHash<KEY, VALUE>::toList() const
{
	return m_valueVector.toList();
}
