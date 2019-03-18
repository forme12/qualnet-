#pragma once

#include <unknwn.h>
#include "agi_interface_smart_ptr.h"

class __declspec(novtable) DECLSPEC_UUID("F99D3F0F-D494-4b9a-AE7E-400675AB9FFA") IAgiInterfaceID : public IUnknown
{
    public:
        virtual void SetNodeID(const long& id) = 0;
        virtual long GetNodeID() const = 0;
        virtual void SetInterfaceID(const int& id) = 0;
        virtual int GetInterfaceID() const = 0;
        virtual void SetChannelIndex(const int& index) = 0;
        virtual int GetChannelIndex() const = 0;
};

template<class T>
class CAgiInterfaceIDImpl : public T,
                            public IAgiInterfaceID
{
public:
    CAgiInterfaceIDImpl();
    CAgiInterfaceIDImpl(const CAgiInterfaceIDImpl& rhs);
    virtual ~CAgiInterfaceIDImpl();

    void SetInfo(const long& nodeID,
                 const int& interfaceID,
                 const int& channelIndex,
                 const bool& isSatellite)
    {
        m_nodeID = nodeID;
        m_interfaceID = interfaceID;
        m_channelIndex = channelIndex;
        m_isSatellite = isSatellite;
    }

    void SetNodeID(const long& id) { m_nodeID = id; }
    long GetNodeID() const { return m_nodeID; }
    void SetInterfaceID(const int& id) { m_interfaceID = id; }
    int GetInterfaceID() const { return m_interfaceID; }
    void SetChannelIndex(const int& index) { m_channelIndex = index; }
    int GetChannelIndex() const { return m_channelIndex; }

    STDMETHOD(LessThan)(AgStkCommUtilLib::IAgStkCommUtilId* pRHS, VARIANT_BOOL* pVal);
    STDMETHOD(ToString)(BSTR* pStringRep);

    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject);

private:
    long m_nodeID;
    int m_interfaceID;
    int m_channelIndex;
    bool m_isSatellite;

    unsigned int m_cRefs;
};

template<class T>
CAgiInterfaceIDImpl<T>::CAgiInterfaceIDImpl() :
m_cRefs(0),
m_nodeID(0),
m_interfaceID(0),
m_channelIndex(0),
m_isSatellite(false)
{
}

template<class T>
CAgiInterfaceIDImpl<T>::CAgiInterfaceIDImpl(const CAgiInterfaceIDImpl<T>& rhs) :
m_cRefs(0),
m_nodeID(rhs.m_nodeID),
m_interfaceID(rhs.m_interfaceID),
m_channelIndex(rhs.m_channelIndex),
m_isSatellite(rhs.m_isSatellite)
{
}

template<class T>
CAgiInterfaceIDImpl<T>::~CAgiInterfaceIDImpl()
{
}

template<class T>
STDMETHODIMP_(ULONG) CAgiInterfaceIDImpl<T>::AddRef()
{
    m_cRefs++;
    return m_cRefs;
}

template<class T>
STDMETHODIMP_(ULONG) CAgiInterfaceIDImpl<T>::Release()
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
HRESULT STDMETHODCALLTYPE CAgiInterfaceIDImpl<T>::QueryInterface(
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
{
    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObject = static_cast<IUnknown*>(static_cast<IAgiInterfaceID*>(this));
        AddRef();
        return S_OK;
    }

    if (IsEqualIID(riid, __uuidof(AgStkCommUtilLib::IAgStkCommUtilId)))
    {
        *ppvObject = static_cast<AgStkCommUtilLib::IAgStkCommUtilId*>(this);
        AddRef();
        return S_OK;
    }

    if (IsEqualIID(riid, __uuidof(IAgiInterfaceID)))
    {
        *ppvObject = static_cast<IAgiInterfaceID*>(this);
        AddRef();
        return S_OK;
    }

    *ppvObject = NULL;
    return E_NOINTERFACE;
}

template<class T>
STDMETHODIMP CAgiInterfaceIDImpl<T>::LessThan(AgStkCommUtilLib::IAgStkCommUtilId* pRHS, VARIANT_BOOL* pVal)
{
    CAgiInterfaceSmartPtr<IAgiInterfaceID> pId;
    if (SUCCEEDED(pRHS->QueryInterface(__uuidof(IAgiInterfaceID), (void**)&pId)))
    {
        long rhsNodeId = pId->GetNodeID();
        int rhsInterfaceId = pId->GetInterfaceID();
        int rhsChannelIndex = pId->GetChannelIndex();

        if (m_isSatellite)
        {
            if (m_nodeID < rhsNodeId)
            {
                *pVal = VARIANT_TRUE;
            }
            else if (m_nodeID > rhsNodeId)
            {
                *pVal = VARIANT_FALSE;
            }
            else if (m_interfaceID < rhsInterfaceId)
            {
                *pVal = VARIANT_TRUE;
            }
            else if (m_interfaceID > rhsInterfaceId)
            {
                *pVal = VARIANT_FALSE;
            }
            else if (m_channelIndex < rhsChannelIndex)
            {
                *pVal = VARIANT_TRUE;
            }
            else
            {
                *pVal = VARIANT_FALSE;
            }
        }
        else
        {
        if (m_nodeID < rhsNodeId)
        {
            *pVal = VARIANT_TRUE;
        }
        else if (m_nodeID > rhsNodeId)
        {
            *pVal = VARIANT_FALSE;
        }
        else if (m_interfaceID < rhsInterfaceId)
        {
            *pVal = VARIANT_TRUE;
        }
        else
        {
            *pVal = VARIANT_FALSE;
        }
    }
    }
    else
    {
        *pVal = VARIANT_FALSE;
    }

    return S_OK;
}

template<class T>
STDMETHODIMP CAgiInterfaceIDImpl<T>::ToString(BSTR* pStringRep)
{
    char strRep[100];
    sprintf(strRep, "Node = %d, Interface = %d, Channel Index = %d", m_nodeID, m_interfaceID, m_channelIndex);
    _bstr_t bstrRep = strRep;
    *pStringRep = bstrRep.Detach();
    return S_OK;
}
