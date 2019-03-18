#ifndef _SNT_EXTERNAL_H_
#define _SNT_EXTERNAL_H_


namepace SNT
{


class AbstractExternalInterface
{
    public:
        virtual bool isActive() = 0;
        virtual void Init(EXTERNAL_Interface* iface, NodeInput* nodeInput) = 0;
        virtual void InitNodes(EXTERNAL_Interface* iface, NodeInput* nodeInput) = 0;
        virtual void ReadParameters(NodeInput* nodeInput) = 0;
        virtual void Receive(EXTERNAL_Interface* iface) = 0;
        virtual void ProcessEvent(Node* node, Message* msg) = 0;
        virtual void Finalize(EXTERNAL_Interface* iface) = 0;

};

#endif
