#pragma once

#include <QVector>
#include <QHash>


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

	void reserve(int n);
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


template <typename KEY, typename VALUE>
void OrderedHash<KEY, VALUE>::reserve(int n)
{
	m_valueVector.reserve(n);
	m_valueVector.reserve(n);
	m_hash.reserve(n);
}


// -------------------------------------------------------------------------------------------------
// PtrOrderedHash class
//


template <typename KEY, typename VALUE>
class PtrOrderedHash
{
private:
	QVector<VALUE*> m_valueVector;
	QVector<KEY> m_keyVector;
	QHash<KEY, int> m_hash;

	void recalcHash();

public:
	PtrOrderedHash();
	~PtrOrderedHash();

	virtual void clear();

	bool isEmpty() const;
	int count() const;
	bool contains(const KEY& key) const;

	virtual void append(const KEY& key, VALUE* value);
	virtual void remove(const KEY& key);
	virtual void removeAt(const int index);

	const VALUE value(const KEY& key) const;

	const KEY key(const int index) const;
	int keyIndex(const KEY& key) const;

	VALUE& operator[](int index);
	const VALUE& operator[](int index) const;

	QList<VALUE*> toList() const;

	void reserve(int n);
};


template <typename KEY, typename VALUE>
void PtrOrderedHash<KEY, VALUE>::recalcHash()
{
	m_hash.clear();

	for (int i = 0; i < m_keyVector.count(); i++)
	{
		m_hash[m_keyVector[i]] = i;
	}
}


template <typename KEY, typename VALUE>
PtrOrderedHash<KEY, VALUE>::PtrOrderedHash()
{
}

template <typename KEY, typename VALUE>
PtrOrderedHash<KEY, VALUE>::~PtrOrderedHash()
{
	clear();
}



template <typename KEY, typename VALUE>
void PtrOrderedHash<KEY, VALUE>::clear()
{
	for(auto value : m_valueVector)
	{
		delete value;
	}

	m_valueVector.clear();
	m_keyVector.clear();
	m_hash.clear();
}


template <typename KEY, typename VALUE>
bool PtrOrderedHash<KEY, VALUE>::isEmpty() const
{
	return m_valueVector.isEmpty();
}


template <typename KEY, typename VALUE>
int PtrOrderedHash<KEY, VALUE>::count() const
{
	return m_valueVector.count();
}


template <typename KEY, typename VALUE>
bool PtrOrderedHash<KEY, VALUE>::contains(const KEY& key) const
{
	return m_hash.contains(key);
}


template <typename KEY, typename VALUE>
void PtrOrderedHash<KEY, VALUE>::append(const KEY& key, VALUE* value)
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
void PtrOrderedHash<KEY, VALUE>::remove(const KEY &key)
{
	int index = m_hash[key];

	m_hash.remove(key);

	VALUE* value = m_valueVector[index];
	delete value;

	m_valueVector.remove(index);

	m_keyVector.remove(index);

	recalcHash();
}


template <typename KEY, typename VALUE>
void PtrOrderedHash<KEY, VALUE>::removeAt(const int index)
{
	m_hash.remove(m_keyVector[index]);

	VALUE* value = m_valueVector[index];
	delete value;

	m_valueVector.removeAt(index);

	m_keyVector.removeAt(index);

	recalcHash();
}


template <typename KEY, typename VALUE>
const VALUE PtrOrderedHash<KEY, VALUE>::value(const KEY& key) const
{
	if (m_hash.contains(key))
	{
		int valueIndex = m_hash[key];
		return *m_valueVector[valueIndex];
	}

	assert(false);

	return VALUE();
}


template <typename KEY, typename VALUE>
const KEY PtrOrderedHash<KEY, VALUE>::key(const int index) const
{
	return m_keyVector[index];
}


template <typename KEY, typename VALUE>
int PtrOrderedHash<KEY, VALUE>::keyIndex(const KEY &key) const
{
	if (m_hash.contains(key))
	{
		return m_hash[key];
	}
	return -1;
}


template <typename KEY, typename VALUE>
VALUE& PtrOrderedHash<KEY, VALUE>::operator[](int index)
{
	return *m_valueVector[index];
}


template <typename KEY, typename VALUE>
const VALUE& PtrOrderedHash<KEY, VALUE>::operator[](int index) const
{
	return *m_valueVector[index];
}


template <typename KEY, typename VALUE>
QList<VALUE*> PtrOrderedHash<KEY, VALUE>::toList() const
{
	return m_valueVector.toList();
}


template <typename KEY, typename VALUE>
void PtrOrderedHash<KEY, VALUE>::reserve(int n)
{
	m_valueVector.reserve(n);
	m_valueVector.reserve(n);
	m_hash.reserve(n);
}


// -------------------------------------------------------------------------------------------------
// HashedVector class
//
// work only with unique keys

template <typename KEY, typename VALUE>
class HashedVector : private QVector<VALUE>
{
private:
	QHash<KEY, int> m_map;

public:
	bool contains(const KEY& key) const { return m_map.contains(key); }
	void insert(const KEY& key, const VALUE& value);

	typename QVector<VALUE>::iterator begin() { return QVector<VALUE>::begin(); }
	typename QVector<VALUE>::const_iterator begin() const { return QVector<VALUE>::begin(); }

	typename QVector<VALUE>::iterator end() { return QVector<VALUE>::end(); }
	typename QVector<VALUE>::const_iterator end() const { return QVector<VALUE>::end(); }

	VALUE& operator[](int i) { return QVector<VALUE>::operator [](i); }
	const VALUE& operator[](int i) const { return QVector<VALUE>::operator [](i); }

	VALUE& operator[](const KEY& key) { return (*this)[m_map.value(key)]; }
	const VALUE& operator[](const KEY& key) const {  return (*this)[m_map[key]]; }

	void clear() { m_map.clear(); QVector<VALUE>::clear(); }

	bool isEmpty() const { return QVector<VALUE>::isEmpty(); }

	int size() const { return QVector<VALUE>::size(); }
	int count() const { return QVector<VALUE>::size(); }
};


template <typename KEY, typename VALUE>
void HashedVector<KEY, VALUE>::insert(const KEY& key, const VALUE& value)
{
	if (contains(key))
	{
		assert(false);		// duplicate key
		return;
	}

	this->append(value);
	m_map.insert(key, this->size() - 1);
}
