// Copyright (c) 2001-2008, Scalable Network Technologies, Inc.  All Rights Reserved.
//                          6100 Center Drive West
//                          Suite 1250 
//                          Los Angeles, CA 90045
//                          sales@scalable-networks.com
//
// This source code is licensed, not sold, and is subject to a written
// license agreement.  Among other things, no portion of this source
// code may be copied, transmitted, disclosed, displayed, distributed,
// translated, used as the basis for a derivative work, or used, in
// whole or in part, for any program or purpose other than its intended
// use in compliance with the license agreement as part of the EXata
// software.  This source code and certain of the algorithms contained
// within it are confidential trade secrets of Scalable Network
// Technologies, Inc. and may not be used as the basis for any other
// software, hardware, product or service.

#ifndef _SIM_CLASS_H_
#define _SIM_CLASS_H_

#include <vector>
#include <string>
#include <map>
#include <set>
#include "hla_types.h"

namespace SNT_HLA
{

class Interaction
{
    private:
        RTI::InteractionClassHandle myHandle;
        bool on;
    public:
        Interaction(RTI::InteractionClassHandle theHandle) : on(false), myHandle(theHandle) {}
        virtual ~Interaction() {}
        virtual void turnOn() { on = true; }
        virtual void turnOff() { on = false; }
        bool isOn() { return on; }
};

class HLAClass;

class HLAObject
{
    protected:
    public:
        RTI::ObjectHandle myHandle;
        std::string myName;
        HLAClass* myClass;
        std::map<RTI::AttributeHandle, abstractAttribute*> attributes;
        bool updated;

    public:
        HLAObject(RTI::ObjectHandle theObject, const char * theObjectName, HLAClass* cls) :
          myHandle(theObject), myName(theObjectName), myClass(cls), updated(false) {}
        virtual ~HLAObject() {}
        virtual void reflectAttributeValues(const RTI::AttributeHandleValuePairSet& theAttributes, const char *theTag) throw (RTI::AttributeNotKnown, RTI::FederateOwnsAttributes, RTI::FederateInternalError)
        {
            for(unsigned int i = 0; i < theAttributes.size(); i++)
            {
                std::map<RTI::AttributeHandle, abstractAttribute*>::iterator it = attributes.find(theAttributes.getHandle(i));
                if( it != attributes.end() )
                {
                    abstractAttribute* attr = it->second;
                    if(attr)
                        attr->reflectAttribute(theAttributes, i);
                }
            }
            updated = true;
        }
};

class HLAObjectFactory
{
    protected:
        HLAClass* theClass;
    public:
        virtual HLAObject* newObject(RTI::ObjectHandle theObject, const char * theObjectName) throw (RTI::CouldNotDiscover) = 0;
        HLAClass* getClass() { return theClass; }
};
class HLAClass
{
    private:
    public:
        HLAClass* baseClass;
        std::string className;
        RTI::ObjectClassHandle classHandle;
        std::map<std::string,RTI::AttributeHandle> attributes;
        HLAObjectFactory& theFactory;
        bool subscribed;
        std::set<HLAObject*> allObjects;
        int numUpdated;
    public:
        HLAClass(const char* class_name, const char** attribute_names, HLAObjectFactory& factory, HLAClass* base = 0) :
          baseClass(base), className(class_name), theFactory(factory), subscribed(false), numUpdated(0)
        {
            const char **p = attribute_names;
            while(*p)
            {
                attributes[std::string(*p)] = 0;
                p++;
            }
        }
        RTI::ObjectClassHandle handle() { return classHandle; }
        void Subscribe(RTI::RTIambassador* rti) throw (RTI::Exception)
        {
            if(baseClass && !baseClass->subscribed)
                baseClass->Subscribe(rti);
            classHandle = rti->getObjectClassHandle(className.c_str());
            std::map<std::string,RTI::AttributeHandle>::iterator it;
            for(it=attributes.begin(); it!=attributes.end(); it++)
            {
                it->second = rti->getAttributeHandle(it->first.c_str(), classHandle);
            }
            RTI::AttributeHandleSet* set = getAttributeHandleSet();
            rti->subscribeObjectClassAttributes(classHandle, *set);
            delete set;
            subscribed = true;
            allObjects.clear();
            numUpdated = 0;
        }
        RTI::AttributeHandleSet* getAttributeHandleSet()
        {
            RTI::AttributeHandleSet* baseSet = 0;
            RTI::ULong size = (RTI::ULong) attributes.size();
            if(baseClass)
            {
                baseSet = baseClass->getAttributeHandleSet();
                size += baseSet->size();
            }
            RTI::AttributeHandleSet* set = RTI::AttributeHandleSetFactory::create(size);
            if(baseSet)
            {
                for(unsigned int i=0; i<baseSet->size(); i++)
                    set->add(baseSet->getHandle(i));
                delete baseSet;
            }
            std::map<std::string,RTI::AttributeHandle>::iterator it;
            for(it=attributes.begin(); it!=attributes.end(); it++)
                set->add(it->second);
            return set;
        }
        virtual HLAObject* newObject(RTI::ObjectHandle theObject, const char * theObjectName) throw (RTI::CouldNotDiscover)
        {
            HLAObject* obj = theFactory.newObject(theObject, theObjectName);
            allObjects.insert(obj);
            return obj;
        }
};

template <class T> class Factory : public HLAObjectFactory
{
    private:
        Factory()
        {
            defineClass();
        }
        void defineClass();
    public:
        static Factory& instance()
        {
            static Factory factory;
            return factory;
        }
        virtual T* newObject(RTI::ObjectHandle theObject, const char * theObjectName) throw (RTI::CouldNotDiscover)
        {
            T* obj = new T(theObject, theObjectName, theClass);
            return obj;
        }
};

};
#endif
