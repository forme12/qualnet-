/******************************************************************************/
/*****         Copyright 2001-2004, Analytical Graphics, Inc.             *****/
/******************************************************************************/
#ifndef CAgiInterfaceSmartPtr_H
#define CAgiInterfaceSmartPtr_H

template<class T> 
class CAgiInterfaceSmartPtr
{
public:

    typedef T* pointer_type;

    CAgiInterfaceSmartPtr() : m_pObj(NULL) 
    { 
    }
    CAgiInterfaceSmartPtr(const CAgiInterfaceSmartPtr<T> &rhs) : m_pObj(rhs.m_pObj) 
    {
        if (m_pObj != NULL)
        {
            m_pObj->AddRef(); 
        }
    }
    CAgiInterfaceSmartPtr(T* pObj) : m_pObj(pObj) 
    { 
        if (m_pObj != NULL)
        {
            m_pObj->AddRef(); 
        }
    }
    ~CAgiInterfaceSmartPtr() 
    { 
        if (m_pObj != NULL)
        {
            m_pObj->Release();
        }
    }
    operator T*() 
    { 
        return m_pObj; 
    }
    operator T*() const
    { 
        return m_pObj; 
    }
    T& operator*() 
    { 
        return *m_pObj; 
    }
    T& operator*() const
    { 
        return *m_pObj; 
    }
    T* operator->() 
    { 
        return m_pObj; 
    }
    T** operator&()
    {
        return &m_pObj;
    }
    const T* operator->() const
    { 
        return m_pObj; 
    }
    CAgiInterfaceSmartPtr<T>& operator=(const CAgiInterfaceSmartPtr<T> &rhs)
    {
        if (m_pObj != rhs.m_pObj)
        {
            if (m_pObj != NULL) 
            {
                m_pObj->Release(); 
            }
            m_pObj = rhs.m_pObj; 
            if (m_pObj != NULL) 
            {
                m_pObj->AddRef(); 
            }
        }
        return *this;
    }
    CAgiInterfaceSmartPtr& operator=(T* pObj) 
    {
        if (m_pObj != NULL) 
        {
            m_pObj->Release(); 
        }
        m_pObj = pObj; 
        if (m_pObj != NULL) 
        {
            m_pObj->AddRef(); 
        }
        return *this;
    }
    bool operator==(const CAgiInterfaceSmartPtr<T> &rhs) const
    {
        return m_pObj == rhs.m_pObj;
    }
    bool operator==(const T* pObj) const
    {
        return m_pObj == pObj;
    }
    bool operator!=(const CAgiInterfaceSmartPtr<T> &rhs) const
    {
        return m_pObj != rhs.m_pObj;
    }
    bool operator!=(const T* pObj) const
    {
        return m_pObj != pObj;
    }
    bool operator<(const CAgiInterfaceSmartPtr<T> &rhs) const
    {
        return m_pObj < rhs.m_pObj;
    }
    HRESULT CopyTo(T** ppT)
    {
        if (ppT == NULL)
            return E_POINTER;
        *ppT = m_pObj;
        if (m_pObj)
            m_pObj->AddRef();
        return S_OK;
    }

private:
    T *m_pObj;
};

#endif
