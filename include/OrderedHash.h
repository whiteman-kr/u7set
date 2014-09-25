#pragma once


template <typename KEY, typename VALUE>
class OrderedHash
{
private:
	QVector<KEY> m_vector;
	QHash<KEY, VALUE> m_hash;

public:
	OrderedHash();

	void clear();

	bool isEmpty() const;
	int count() const;
	bool contains(const KEY& key) const;

	void append(const KEY& key, const VALUE& value);
	const VALUE value(const KEY& key) const;
	const KEY key(const int index) const;
	const VALUE& operator[](const int index) const;
};


template <typename KEY, typename VALUE>
OrderedHash<KEY, VALUE>::OrderedHash()
{
}


template <typename KEY, typename VALUE>
void OrderedHash<KEY, VALUE>::clear()
{
	m_vector.clear();
	m_hash.clear();
}


template <typename KEY, typename VALUE>
bool OrderedHash<KEY, VALUE>::isEmpty() const
{
	return m_vector.isEmpty();
}


template <typename KEY, typename VALUE>
int OrderedHash<KEY, VALUE>::count() const
{
	return m_vector.count();
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
	}
	else
	{
		m_hash.insert(key, value);
		m_vector.append(key);
	}
}


template <typename KEY, typename VALUE>
const VALUE OrderedHash<KEY, VALUE>::value(const KEY& key) const
{
	return m_hash.value(key);
}


template <typename KEY, typename VALUE>
const KEY OrderedHash<KEY, VALUE>::key(const int index) const
{
	return m_vector[index];
}


template <typename KEY, typename VALUE>
const VALUE& OrderedHash<KEY, VALUE>::operator[](const int index) const
{
	return m_hash.value(m_vector[index]);
}
