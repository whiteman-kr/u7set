#include "SignalBase.h"


// -------------------------------------------------------------------------------------------------------------------

SignalBase theSignalBase;

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SignalBase::SignalBase(QObject *parent) :
    QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

 SignalBase::~SignalBase()
 {
 }

 // -------------------------------------------------------------------------------------------------------------------

void SignalBase::clear()
{
    m_paramMutex.lock();

        m_hashMap.clear();

        m_signalList.clear();

    m_paramMutex.unlock();

    m_unitMutex.lock();

        m_unitList.clear();

    m_unitMutex.unlock();

    m_stateMutex.lock();

        m_requestStateList.clear();

    m_stateMutex.unlock();

}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::size() const
{
    int size = 0;

    m_paramMutex.lock();

        size = m_signalList.size();

    m_paramMutex.unlock();

    return size;
}
// -------------------------------------------------------------------------------------------------------------------

bool SignalBase::hashIsValid(const Hash& hash)
{
    if (hash == 0)
    {
        assert(hash != 0);
        return false;
    }

    bool valid = false;

    m_paramMutex.lock();

        if (m_hashMap.contains(hash) == true)
        {
            valid = true;
        }

    m_paramMutex.unlock();

    return valid;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::appendSignal(const Signal& param)
{
    if (param.appSignalID().isEmpty() == true || param.hash() == 0)
    {
        assert(false);
        return -1;
    }

    int index = -1;

    m_paramMutex.lock();

        if (m_hashMap.contains(param.hash()) == false)
        {

            MEASURE_SIGNAL signal(param);

            m_signalList.append(signal);
            index = m_signalList.size() - 1;

            m_hashMap.insert(param.hash(), index);
        }

     m_paramMutex.unlock();

     return index;
}

// -------------------------------------------------------------------------------------------------------------------

bool SignalBase::signalParam(const Hash& hash, Signal& param)
{
    if (hashIsValid(hash) == false)
    {
        assert(false);
        return false;
    }

    int index = -1;

    m_paramMutex.lock();

        index = m_hashMap[hash];

    m_paramMutex.unlock();

    return signalParam(index, param);
}

// -------------------------------------------------------------------------------------------------------------------

bool SignalBase::signalParam(const int& index, Signal& param)
{
    bool found = false;

    m_paramMutex.lock();

        if (index >= 0 && index < m_signalList.size())
        {
            param = m_signalList[index].m_param;

            found = true;
        }

    m_paramMutex.unlock();

    return found;
}

// -------------------------------------------------------------------------------------------------------------------

bool SignalBase::signalState(const Hash& hash, AppSignalState& state)
{
    if (hashIsValid(hash) == false)
    {
        assert(false);
        return false;
    }

    int index = -1;

    m_paramMutex.lock();

        index = m_hashMap[hash];

    m_paramMutex.unlock();

    return signalState(index, state);
}

// -------------------------------------------------------------------------------------------------------------------

bool SignalBase::signalState(const int& index, AppSignalState& state)
{
    bool found = false;

    m_paramMutex.lock();

        if (index >= 0 && index < m_signalList.size())
        {
            state = m_signalList[index].m_state;

            found = true;
        }

    m_paramMutex.unlock();

    return found;
}

// -------------------------------------------------------------------------------------------------------------------

bool SignalBase::setSignalState(const Hash& hash, const AppSignalState &state)
{
    if (hashIsValid(hash) == false)
    {
        assert(false);
        return false;
    }

    int index = -1;

    m_paramMutex.lock();

        index = m_hashMap[hash];

    m_paramMutex.unlock();

    return setSignalState(index, state);
}

// -------------------------------------------------------------------------------------------------------------------

bool SignalBase::setSignalState(const int& index, const AppSignalState &state)
{
    m_paramMutex.lock();

        if (index >= 0 && index < m_signalList.size())
        {
            m_signalList[index].m_state = state;
        }

    m_paramMutex.unlock();

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::appendUnit(const int& id, const QString& unit)
{
    m_unitMutex.lock();

        m_unitList.append(id, unit);

    m_unitMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalBase::signalUnit(const Hash& hash)
{
    if (hashIsValid(hash) == false)
    {
        assert(false);
        return false;
    }

    int index = -1;

    m_paramMutex.lock();

        index = m_hashMap[hash];

    m_paramMutex.unlock();

    return signalUnit(index);
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalBase::signalUnit(const int& index)
{
    Signal param;

    m_paramMutex.lock();

        if (index >= 0 && index < m_signalList.size())
        {
            param = m_signalList[index].m_param;
        }

    m_paramMutex.unlock();

    return unit(param.unitID());
}


// -------------------------------------------------------------------------------------------------------------------

QString SignalBase::unit(const int& id)
{
    QString strUnit;

    m_unitMutex.lock();

        if (m_unitList.contains(id) == true)
        {
            strUnit = m_unitList[id];
        }

    m_unitMutex.unlock();

    return strUnit;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::hashForRequestStateCount() const
{
    int count = 0;

    m_stateMutex.lock();

        count = m_requestStateList.size();

    m_stateMutex.unlock();

    return count;
}

// -------------------------------------------------------------------------------------------------------------------

Hash SignalBase::hashForRequestState(int index)
{
    Hash hash;

    m_stateMutex.lock();

        if (index >= 0 && index < m_requestStateList.size())
        {
            hash = m_requestStateList[index];
        }

    m_stateMutex.unlock();

    return hash;
}

// -------------------------------------------------------------------------------------------------------------------
