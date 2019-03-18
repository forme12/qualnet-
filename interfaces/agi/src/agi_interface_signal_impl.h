#pragma once

template<class T>
class CAgiInterfaceSignalImpl : public T
{
public:
    CAgiInterfaceSignalImpl();
    CAgiInterfaceSignalImpl(const CAgiInterfaceSignalImpl& rhs);
    virtual ~CAgiInterfaceSignalImpl();

    STDMETHOD(get_FrequencyHertz)(double* pFrequencyHertz);
    STDMETHOD(put_FrequencyHertz)(double frequencyHertz);
    STDMETHOD(get_PowerWatts)(double* pPowerWatts);
    STDMETHOD(put_PowerWatts)(double powerWatts);

    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject);

private:
    double m_powerWatts;
    double m_frequencyHertz;

    unsigned int m_cRefs;
};

template<class T>
CAgiInterfaceSignalImpl<T>::CAgiInterfaceSignalImpl() :
m_cRefs(0),
m_frequencyHertz(0.0),
m_powerWatts(0.0)
{
}

template<class T>
CAgiInterfaceSignalImpl<T>::CAgiInterfaceSignalImpl(const CAgiInterfaceSignalImpl<T>& rhs) :
m_cRefs(0),
m_frequencyHertz(rhs.m_frequencyHertz),
m_powerWatts(rhs.m_powerWatts)
{
}

template<class T>
CAgiInterfaceSignalImpl<T>::~CAgiInterfaceSignalImpl()
{
}

template<class T>
STDMETHODIMP_(ULONG) CAgiInterfaceSignalImpl<T>::AddRef()
{
    m_cRefs++;
    return m_cRefs;
}

template<class T>
STDMETHODIMP_(ULONG) CAgiInterfaceSignalImpl<T>::Release()
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
HRESULT STDMETHODCALLTYPE CAgiInterfaceSignalImpl<T>::QueryInterface(
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
{
    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObject = static_cast<IUnknown*>(this);
        AddRef();
        return S_OK;
    }

    if (IsEqualIID(riid, __uuidof(IAgStkCommUtilSignal)))
    {
        *ppvObject = static_cast<AgStkCommUtilLib::IAgStkCommUtilSignal*>(this);
        AddRef();
        return S_OK;
    }

    *ppvObject = NULL;
    return E_NOINTERFACE;
}

template<class T>
STDMETHODIMP CAgiInterfaceSignalImpl<T>::get_FrequencyHertz(double* pFrequencyHertz)
{
    *pFrequencyHertz = m_frequencyHertz;
    return S_OK;
}

template<class T>
STDMETHODIMP CAgiInterfaceSignalImpl<T>::put_FrequencyHertz(double frequencyHertz)
{
    m_frequencyHertz = frequencyHertz;
    return S_OK;
}

template<class T>
STDMETHODIMP CAgiInterfaceSignalImpl<T>::get_PowerWatts(double* pPowerWatts)
{
    *pPowerWatts = m_powerWatts;
    return S_OK;
}

template<class T>
STDMETHODIMP CAgiInterfaceSignalImpl<T>::put_PowerWatts(double powerWatts)
{
    m_powerWatts = powerWatts;
    return S_OK;
}