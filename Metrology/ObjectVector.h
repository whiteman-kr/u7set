#ifndef OBJECTARRAY_H
#define OBJECTARRAY_H

#include <QMutex>
#include <QVector>

#include "Database.h"

// ==============================================================================================
// ==============================================================================================
// ==============================================================================================

template <class TYPE>
class ObjectVector
{
public:

    explicit        ObjectVector();
    explicit        ObjectVector(const ObjectVector& from);

private:

    mutable QMutex  m_vectorMutex;

    QVector<TYPE>   m_vector;

public:

    int				count() const;
    bool            isEmpty() const;

    // TYPE         operator[](const int index);       don't append this method
    TYPE            at(const int index) const;
    QVector<TYPE>   toVector() const;

    int             set(const int index, const TYPE item);
    void            set(const QVector<TYPE>& fromVector);
    ObjectVector&   operator=(const ObjectVector& from);

    int				append(const TYPE item);
    int             insert(const int index, const TYPE item);
    bool			remove(const int index);
    void            clear();

    void            swap(const int i, const int j);

    bool            contains(const TYPE& item) const;
    int				find(const TYPE& item) const;

    virtual void    initEmptyData(QVector<TYPE>& data);
    bool            loadData(int table);
    bool            saveData(int table);
};

// ==============================================================================================

template <class TYPE>
ObjectVector<TYPE>::ObjectVector()
{
}

// ----------------------------------------------------------------------------------------------

template <class TYPE>
ObjectVector<TYPE>::ObjectVector(const ObjectVector& from)
{
    *this = from;
}

// ----------------------------------------------------------------------------------------------

template <class TYPE>
int ObjectVector<TYPE>::count() const
{
    int count = 0;

    m_vectorMutex.lock();

        count = m_vector.count();

    m_vectorMutex.unlock();

    return count;
}

// ----------------------------------------------------------------------------------------------

template <class TYPE>
bool ObjectVector<TYPE>::isEmpty() const
{
    bool empty = false;

    m_vectorMutex.lock();

        empty = m_vector.isEmpty();

    m_vectorMutex.unlock();

    return empty;
}

// ----------------------------------------------------------------------------------------------

template <class TYPE>
TYPE ObjectVector<TYPE>::at(const int index) const
{
    TYPE item;

    if (index < 0 || index >= count())
    {
        return item;
    }

    m_vectorMutex.lock();

        item = m_vector[index];

    m_vectorMutex.unlock();

    return item;
}

// ----------------------------------------------------------------------------------------------

template <class TYPE>
QVector<TYPE> ObjectVector<TYPE>::toVector() const
{
    return m_vector;
}

// ----------------------------------------------------------------------------------------------

template <class TYPE>
int ObjectVector<TYPE>::set(const int index, const TYPE item)
{
    if (index < 0 || index >= count())
    {
        return -1;
    }

    m_vectorMutex.lock();

        m_vector[index] = item;

    m_vectorMutex.unlock();

    return index;
}


// ----------------------------------------------------------------------------------------------

template <class TYPE>
void ObjectVector<TYPE>::set(const QVector<TYPE>& fromVector)
{
    clear();

    m_vectorMutex.lock();

        m_vector = fromVector;

    m_vectorMutex.unlock();
}

// ----------------------------------------------------------------------------------------------

template <class TYPE>
ObjectVector<TYPE>& ObjectVector<TYPE>::operator=(const ObjectVector& from)
{
    clear();

    m_vectorMutex.lock();

        m_vector = from.toVector();

    m_vectorMutex.unlock();

    return *this;
}

// ----------------------------------------------------------------------------------------------

template <class TYPE>
int ObjectVector<TYPE>::append(const TYPE item)
{
    int index = -1;

    m_vectorMutex.lock();

        m_vector.append(item);
        index = m_vector.count() - 1;

    m_vectorMutex.unlock();

    return index;
}

// -------------------------------------------------------------------------------------------------------------------

template <class TYPE>
int ObjectVector<TYPE>::insert(const int index, const TYPE item)
{
    if (index < 0 || index > count())
    {
        return -1;
    }

    m_vectorMutex.lock();

        m_vector.insert(index, item);

    m_vectorMutex.unlock();

    return index;
}

// ----------------------------------------------------------------------------------------------

template <class TYPE>
bool ObjectVector<TYPE>::remove(const int index)
{
    if (index < 0 || index >= count())
    {
        return false;
    }

    m_vectorMutex.lock();

        m_vector.remove(index);

    m_vectorMutex.unlock();

    return true;
}


// ----------------------------------------------------------------------------------------------

template <class TYPE>
void ObjectVector<TYPE>::clear()
{
    m_vectorMutex.lock();

        m_vector.clear();

    m_vectorMutex.unlock();
}

// ----------------------------------------------------------------------------------------------

template <class TYPE>
void ObjectVector<TYPE>::swap(const int i, const int j)
{
    if ( (i < 0 || i >= count()) || (j < 0 || j >= count()) )
    {
        return;
    }

    m_vectorMutex.lock();

        TYPE item   = m_vector[j];
        m_vector[j] = m_vector[i];
        m_vector[i] = item;

    m_vectorMutex.unlock();

}

// ----------------------------------------------------------------------------------------------

template <class TYPE>
bool ObjectVector<TYPE>::contains(const TYPE& item) const
{
    bool isContain = false;

    m_vectorMutex.lock();

        isContain = m_vector.contains(item);

    m_vectorMutex.unlock();

    return isContain;
}


// ----------------------------------------------------------------------------------------------

template <class TYPE>
int ObjectVector<TYPE>::find(const TYPE& item) const
{
    int index = -1;

    m_vectorMutex.lock();

        int count = m_vector.count();
        for(int i = 0; i < count; i++)
        {
            if (m_vector[i] == item)
            {
                index = i;

                break;
            }
        }

    m_vectorMutex.unlock();

    return index;
}

// ----------------------------------------------------------------------------------------------

template <class TYPE>
void ObjectVector<TYPE>::initEmptyData(QVector<TYPE>& data)
{
    data.empty();
}

// ----------------------------------------------------------------------------------------------

template <class TYPE>
bool ObjectVector<TYPE>::loadData(int table)
{
    if (thePtrDB == nullptr)
    {
        return false;
    }

    if (thePtrDB->isOpen() == false)
    {
        return false;
    }

    if (table < 0 || table >= SQL_TABLE_COUNT)
    {
        return false;
    }

    SqlTable* pTable = thePtrDB->openTable(table);
    if (pTable == nullptr)
    {
        return false;
    }

    m_vectorMutex.lock();

        QVector<TYPE> dataVector;

        int recordCount = pTable->recordCount();
        if (recordCount == 0)
        {
            // if table is empty then initialize data
            //
            initEmptyData(dataVector);

            // update table
            //
            pTable->write(dataVector.data(), dataVector.count());
        }
        else
        {
            // if table is not empty then read data from table
            //
            dataVector.resize(recordCount);
            pTable->read(dataVector.data());
        }

    m_vectorMutex.unlock();

    set(dataVector);

    pTable->close();

    return true;
}

// ----------------------------------------------------------------------------------------------

template <class TYPE>
bool ObjectVector<TYPE>::saveData(int table)
{
    if (thePtrDB == nullptr)
    {
        return false;
    }

    if (thePtrDB->isOpen() == false)
    {
        return false;
    }

    if (table < 0 || table >= SQL_TABLE_COUNT)
    {
        return false;
    }

    SqlTable* pTable = thePtrDB->openTable(table);
    if (pTable == nullptr)
    {
        return false;
    }

    m_vectorMutex.lock();

        if (pTable->clear() == true)
        {
            pTable->write(toVector().data(), toVector().count());
        }

    m_vectorMutex.unlock();

    pTable->close();

    return true;
}

// ==============================================================================================
// ==============================================================================================
// ==============================================================================================

template <class TYPE>
class PtrObjectVector
{
public:
    explicit            PtrObjectVector();
    explicit            PtrObjectVector(const PtrObjectVector& from);
                        ~PtrObjectVector();

protected:

    mutable QMutex      m_vectorMutex;

    QVector<TYPE*>      m_vector;

public:

    int                 count() const;
    bool                isEmpty() const;

    // TYPE*            operator[](const int index);       don't append this method
    TYPE*               at(const int index) const;
    QVector<TYPE*>      toVector() const;

    int                 set(const int index, TYPE *ptr);
    void                set(const QVector<TYPE*>& fromVector);
    PtrObjectVector&    operator=(const PtrObjectVector& from);

    int                 append(TYPE *ptr);
    int                 insert(const int index, TYPE *ptr);
    bool                remove(const int index, const bool removeData = true);
    void                clear(const bool removeData = true);

    void                swap(const int i, const int j);

    bool                contains(TYPE *ptr) const;
    int                 find(TYPE *ptr) const;
};

// ==============================================================================================

template <class TYPE>
PtrObjectVector<TYPE>::PtrObjectVector()
{
}

// ----------------------------------------------------------------------------------------------

template <class TYPE>
PtrObjectVector<TYPE>::PtrObjectVector(const PtrObjectVector& from)
{
    *this = from;
}

// ----------------------------------------------------------------------------------------------

template <class TYPE>
PtrObjectVector<TYPE>::~PtrObjectVector()
{
    clear();
}

// ----------------------------------------------------------------------------------------------

template <class TYPE>
int PtrObjectVector<TYPE>::count() const
{
    int count = 0;

    m_vectorMutex.lock();

        count = m_vector.count();

    m_vectorMutex.unlock();

    return count;
}

// ----------------------------------------------------------------------------------------------

template <class TYPE>
bool PtrObjectVector<TYPE>::isEmpty() const
{
    bool empty = false;

    m_vectorMutex.lock();

        empty = m_vector.isEmpty();

    m_vectorMutex.unlock();

    return empty;
}

// ----------------------------------------------------------------------------------------------

template <class TYPE>
TYPE* PtrObjectVector<TYPE>::at(const int index) const
{
    if (index < 0 || index >= count())
    {
        return nullptr;
    }

    TYPE *ptr = nullptr;

    m_vectorMutex.lock();

        ptr = m_vector[index];

    m_vectorMutex.unlock();

    return ptr;
}

// ----------------------------------------------------------------------------------------------

template <class TYPE>
QVector<TYPE*> PtrObjectVector<TYPE>::toVector() const
{
    return m_vector;
}

// ----------------------------------------------------------------------------------------------

template <class TYPE>
int PtrObjectVector<TYPE>::set(const int index, TYPE *ptr)
{
    if (index < 0 || index >= count())
    {
        return -1;
    }

    if (ptr == nullptr)
    {
        return -1;
    }

    m_vectorMutex.lock();

        m_vector[index] = ptr;

    m_vectorMutex.unlock();

    return index;
}

// ----------------------------------------------------------------------------------------------

template <class TYPE>
void PtrObjectVector<TYPE>::set(const QVector<TYPE*>& fromVector)
{
    clear();

    m_vectorMutex.lock();

        m_vector = fromVector;

    m_vectorMutex.unlock();
}

// ----------------------------------------------------------------------------------------------

template <class TYPE>
PtrObjectVector<TYPE>& PtrObjectVector<TYPE>::operator=(const PtrObjectVector& from)
{
    clear();

    m_vectorMutex.lock();

        m_vector = from.toVector();

    m_vectorMutex.unlock();

    return *this;
}

// ----------------------------------------------------------------------------------------------

template <class TYPE>
int PtrObjectVector<TYPE>::append(TYPE *ptr)
{
    if (ptr == nullptr)
    {
        return -1;
    }

    int index = -1;

    m_vectorMutex.lock();

        m_vector.append(ptr);
        index = m_vector.count() - 1;

    m_vectorMutex.unlock();

    return index;
}

// -------------------------------------------------------------------------------------------------------------------

template <class TYPE>
int PtrObjectVector<TYPE>::insert(const int index, TYPE *ptr)
{
    if (index < 0 || index > count())
    {
        return -1;
    }

    if (ptr == nullptr)
    {
        return -1;
    }

    m_vectorMutex.lock();

        m_vector.insert(index, ptr);

    m_vectorMutex.unlock();

    return index;
}

// ----------------------------------------------------------------------------------------------

template <class TYPE>
bool PtrObjectVector<TYPE>::remove(const int index, bool removeData)
{
    if (index < 0 || index >= count())
    {
        return false;
    }

    m_vectorMutex.lock();

        if (removeData == true)
        {
            TYPE *ptr = m_vector[index];
            if (ptr != nullptr)
            {
                delete ptr;
            }
        }

        m_vector.remove(index);

    m_vectorMutex.unlock();

    return true;
}


// ----------------------------------------------------------------------------------------------

template <class TYPE>
void PtrObjectVector<TYPE>::clear(const bool removeData)
{
    m_vectorMutex.lock();

        if (removeData == true)
        {
            int count = m_vector.count();
            for(int i = count - 1; i >= 0; i--)
            {
                TYPE *ptr = m_vector[i];
                if (ptr == nullptr)
                {
                    continue;
                }

                delete ptr;
            }
        }

        m_vector.clear();

    m_vectorMutex.unlock();
}

// ----------------------------------------------------------------------------------------------

template <class TYPE>
void PtrObjectVector<TYPE>::swap(const int i, const int j)
{
    if ( (i < 0 || i >= count()) || (j < 0 || j >= count()) )
    {
        return;
    }

    m_vectorMutex.lock();

        TYPE *ptr   = m_vector[j];
        m_vector[j] = m_vector[i];
        m_vector[i] = ptr;

    m_vectorMutex.unlock();

}

// ----------------------------------------------------------------------------------------------

template <class TYPE>
bool PtrObjectVector<TYPE>::contains(TYPE *ptr) const
{
    if (ptr == nullptr)
    {
        return false;
    }

    bool isContain = false;

    m_vectorMutex.lock();

        isContain = m_vector.contains(ptr);

    m_vectorMutex.unlock();

    return isContain;
}


// ----------------------------------------------------------------------------------------------

template <class TYPE>
int PtrObjectVector<TYPE>::find(TYPE *ptr) const
{
    if (ptr == nullptr)
    {
        return -1;
    }

    int index = -1;

    m_vectorMutex.lock();

        int count = m_vector.count();
        for(int i = 0; i < count; i++)
        {
            if (m_vector[i] == ptr)
            {
                index = i;

                break;
            }
        }

    m_vectorMutex.unlock();

    return index;
}

// ----------------------------------------------------------------------------------------------

#endif // OBJECTARRAY_H

