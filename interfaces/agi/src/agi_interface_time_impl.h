#pragma once

#include "agi_interface_smart_ptr.h"
#include <unknwn.h>
#include "clock.h"

class __declspec(novtable) DECLSPEC_UUID("5E3BE5C8-BBF1-45af-B97E-CA3DCADB50E7") IAgiInterfaceTime : public IUnknown
{
    public:
        virtual void SetTime(const clocktype& time) = 0;
        virtual clocktype GetTime() const = 0;
};

template<class T>
class CAgiInterfaceTimeImpl : public T,
                              public IAgiInterfaceTime
{
public:
    CAgiInterfaceTimeImpl();
    CAgiInterfaceTimeImpl(clocktype time);
    CAgiInterfaceTimeImpl(const CAgiInterfaceTimeImpl& rhs);
    virtual ~CAgiInterfaceTimeImpl();

    void SetTime(const clocktype& time) { m_time = time; }
    clocktype GetTime() const { return m_time; }
    STDMETHOD(get_TimeSeconds)(double* pTimeInSeconds);
    STDMETHOD(Equals)(AgStkCommUtilLib::IAgStkCommUtilTime* pRHS, VARIANT_BOOL* pVal);
    STDMETHOD(CopyFrom)(AgStkCommUtilLib::IAgStkCommUtilTime* pTime);
    STDMETHOD(SubtractSeconds)(double seconds, AgStkCommUtilLib::IAgStkCommUtilTime** ppTime);
    STDMETHOD(SubtractSecondsCopyTo)(double seconds, AgStkCommUtilLib::IAgStkCommUtilTime* pTime);

    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject);

protected:
    virtual AgStkCommUtilLib::IAgStkCommUtilTime* ConstructNew(clocktype time) = 0;

private:
    clocktype m_time;

    unsigned int m_cRefs;
};

template<class T>
CAgiInterfaceTimeImpl<T>::CAgiInterfaceTimeImpl() :
m_cRefs(0),
m_time(0)
{
}

template<class T>
CAgiInterfaceTimeImpl<T>::CAgiInterfaceTimeImpl(clocktype time) :
m_cRefs(0),
m_time(time)
{
}

template<class T>
CAgiInterfaceTimeImpl<T>::CAgiInterfaceTimeImpl(const CAgiInterfaceTimeImpl<T>& rhs) :
m_cRefs(0),
m_time(rhs.m_time)
{
}

template<class T>
CAgiInterfaceTimeImpl<T>::~CAgiInterfaceTimeImpl()
{
}

template<class T>
STDMETHODIMP_(ULONG) CAgiInterfaceTimeImpl<T>::AddRef()
{
    m_cRefs++;
    return m_cRefs;
}

template<class T>
STDMETHODIMP_(ULONG) CAgiInterfaceTimeImpl<T>::Release()
{
    ULONG ulCount = --m_cRefs;
    if (!ulCount)
    {
        m_cRefs = (unsigned int)(-(LONG_MAX/2));
        delete this;
    }
    return ulCount;
}

template<class T>
HRESULT STDMETHODCALLTYPE CAgiInterfaceTimeImpl<T>::QueryInterface(
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
{
    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObject = static_cast<IUnknown*>(static_cast<IAgiInterfaceTime*>(this));
        AddRef();
        return S_OK;
    }

    if (IsEqualIID(riid, __uuidof(AgStkCommUtilLib::IAgStkCommUtilTime)))
    {
        *ppvObject = static_cast<AgStkCommUtilLib::IAgStkCommUtilTime*>(this);
        AddRef();
        return S_OK;
    }

    if (IsEqualIID(riid, __uuidof(IAgiInterfaceTime)))
    {
        *ppvObject = static_cast<IAgiInterfaceTime*>(this);
        AddRef();
        return S_OK;
    }

    *ppvObject = NULL;
    return E_NOINTERFACE;
}

template<class T>
STDMETHODIMP CAgiInterfaceTimeImpl<T>::Equals(AgStkCommUtilLib::IAgStkCommUtilTime* pRHS, VARIANT_BOOL* pVal)
{
    CAgiInterfaceSmartPtr<IAgiInterfaceTime> pTime;
    if (SUCCEEDED(pRHS->QueryInterface(__uuidof(IAgiInterfaceTime), (void**)&pTime)))
    {
        *pVal = pTime->GetTime() == m_time ? VARIANT_TRUE : VARIANT_FALSE;
    }
    else
    {
        *pVal = VARIANT_FALSE;
    }

    return S_OK;
}

template<class T>
STDMETHODIMP CAgiInterfaceTimeImpl<T>::get_TimeSeconds(double* pTimeInSeconds)
{
    *pTimeInSeconds = (double)m_time * 1.0e-9;
    return S_OK;
}

template<class T>
STDMETHODIMP CAgiInterfaceTimeImpl<T>::CopyFrom(AgStkCommUtilLib::IAgStkCommUtilTime* pTime)
{
    HRESULT hr = E_FAIL;

    CAgiInterfaceSmartPtr<IAgiInterfaceTime> pBase;
    if (SUCCEEDED(pTime->QueryInterface(__uuidof(IAgiInterfaceTime), (void**)&pBase)))
    {
        m_time = pBase->GetTime();
        hr = S_OK;
    }

    return hr;
}

template<class T>
STDMETHODIMP CAgiInterfaceTimeImpl<T>::SubtractSeconds(double seconds, AgStkCommUtilLib::IAgStkCommUtilTime** ppTime)
{
    CAgiInterfaceSmartPtr<AgStkCommUtilLib::IAgStkCommUtilTime> newTime = ConstructNew(m_time - (clocktype)(seconds * 1.0e9));
    return newTime.CopyTo(ppTime);
}

template<class T>
STDMETHODIMP CAgiInterfaceTimeImpl<T>::SubtractSecondsCopyTo(double seconds, AgStkCommUtilLib::IAgStkCommUtilTime* pTime)
{
    HRESULT hr = E_FAIL;

    CAgiInterfaceSmartPtr<IAgiInterfaceTime> pBase;
    if (SUCCEEDED(pTime->QueryInterface(__uuidof(IAgiInterfaceTime), (void**)&pBase)))
    {
        pBase->SetTime(m_time - (clocktype)(seconds * 1.0e9));
        hr = S_OK;
    }

    return hr;
}