#pragma once


template <typename KEY, typename VALUE>
class OrderedHash
{
private:
	QVector<VALUE> m_valueVector;
	QVector<KEY> m_keyVector;
	QHash<KEY, int> m_hash;

public:
	OrderedHash();

	void clear();

	bool isEmpty() const;
	int count() const;
	bool contains(const KEY& key) const;

	void append(const KEY& key, const VALUE& value);

	const VALUE value(const KEY& key) const;

	const KEY key(const int index) const;

	VALUE& operator[](int index);
	const VALUE& operator[](int index) const;

};


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
VALUE& OrderedHash<KEY, VALUE>::operator[](int index)
{
	return m_valueVector[index];
}


template <typename KEY, typename VALUE>
const VALUE& OrderedHash<KEY, VALUE>::operator[](int index) const
{
	return m_valueVector[index];
}
