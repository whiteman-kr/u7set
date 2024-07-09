#pragma once

#include <vector>
#include <map>
#include <cassert>

// -------------------------------------------------------------------------------------------------
//
// HashedVector class
//
// work only with unique keys
//
// -------------------------------------------------------------------------------------------------

template <typename KEY, typename VALUE>
class HashedVector
{
public:
	bool contains(const KEY& key) const { return m_map.contains(key); }
	void insert(const KEY& key, const VALUE& value);

	typename std::vector<VALUE>::iterator begin() { return m_vector.begin(); }
	typename std::vector<VALUE>::const_iterator begin() const { return m_vector.cbegin(); }

	typename std::vector<VALUE>::iterator end() { return m_vector.end(); }
	typename std::vector<VALUE>::const_iterator end() const { return m_vector.cend(); }

	VALUE& operator[](qsizetype i);
	const VALUE& operator[](qsizetype i) const;

	VALUE& value(const KEY& key);
	const VALUE& value(const KEY& key) const;

	const VALUE& value(const KEY& key, const VALUE& defaultValue) const;

	qsizetype indexOf(const KEY& key) const;

	void clear() { m_map.clear(); m_vector.clear(); }

	bool isEmpty() const { return m_vector.empty(); }

	qsizetype size() const { return m_vector.size(); }
	qsizetype count() const { return m_vector.size(); }

	void reserve(qsizetype size) { m_vector.reserve(size); }

private:
	std::vector<VALUE> m_vector;
	std::map<KEY, qsizetype> m_map;		// key => index in m_vector

	VALUE m_notValidValue;
};

template <typename KEY, typename VALUE>
void HashedVector<KEY, VALUE>::insert(const KEY& key, const VALUE& value)
{
	if (contains(key) == true)
	{
		assert(false);		// duplicate key
		return;
	}

	qsizetype index = m_vector.size();
	m_vector.emplace_back(value);
	m_map.emplace(key, index);
}

template <typename KEY, typename VALUE>
VALUE& HashedVector<KEY, VALUE>::operator[](qsizetype i)
{
	assert(i >= 0 && i < count());
	return m_vector[i];
}

template <typename KEY, typename VALUE>
const VALUE& HashedVector<KEY, VALUE>::operator[](qsizetype i) const
{
	assert(i >= 0 && i < count());
	return m_vector[i];
}

template <typename KEY, typename VALUE>
VALUE& HashedVector<KEY, VALUE>::value(const KEY& key)
{
	auto it = m_map.find(key);

	if (it == m_map.end())
	{
		assert(false);
		return m_notValidValue;
	}

	return m_vector[it->second];
}

template <typename KEY, typename VALUE>
const VALUE& HashedVector<KEY, VALUE>::value(const KEY& key) const
{
	auto it = m_map.find(key);

	if (it == m_map.end())
	{
		assert(false);
		return m_notValidValue;
	}

	return m_vector[it->second];
}

template <typename KEY, typename VALUE>
const VALUE& HashedVector<KEY, VALUE>::value(const KEY& key, const VALUE& defaultValue) const
{
	auto it = m_map.find(key);

	if (it == m_map.end())
	{
		return defaultValue;
	}

	return m_vector[it->second];
}

template <typename KEY, typename VALUE>
qsizetype HashedVector<KEY, VALUE>::indexOf(const KEY& key) const
{
	auto it = m_map.find(key);

	if (it == m_map.end())
	{
		return -1;
	}

	return it->second;
}

