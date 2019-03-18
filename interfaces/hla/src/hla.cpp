// Copyright (c) 2001-2009, Scalable Network Technologies, Inc.  All Rights Reserved.
//                          6100 Center Drive
//                          Suite 1250
//                          Los Angeles, CA 90045
//                          sales@scalable-networks.com
//
// This source code is licensed, not sold, and is subject to a written
// license agreement.  Among other things, no portion of this source
// code may be copied, transmitted, disclosed, displayed, distributed,
// translated, used as the basis for a derivative work, or used, in
// whole or in part, for any program or purpose other than its intended
// use in compliance with the license agreement as part of the QualNet
// software.  This source code and certain of the algorithms contained
// within it are confidential trade secrets of Scalable Network
// Technologies, Inc. and may not be used as the basis for any other
// software, hardware, product or service.

// iostream.h is needed to support DMSO RTI 1.3v6 (non-NG).

#ifdef NOT_RTI_NG
#include <iostream.h>
#else /* NOT_RTI_NG */
#include <iostream>
using namespace std;
#endif /* NOT_RTI_NG */

#include <cassert>

#include "hla_shared.h"
#include "hla_main.h"

bool g_hlaActive;

void
HlaReadHlaParameter(NodeInput* nodeInput)
{
    BOOL retVal;
    char buf[MAX_STRING_LENGTH];

    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "HLA",
        &retVal,
        buf);

    if (retVal && strcmp(buf, "YES") == 0)
    {
        g_hlaActive = true;

        cout << endl
             << "HLA support enabled." << endl;
    }
    else
    {
        g_hlaActive = false;
    }
}

bool
HlaIsActive()
{
    return g_hlaActive;
}

void
HlaInit(EXTERNAL_Interface* iface, NodeInput* nodeInput)
{
    // This function is called before nodes and protocols are initialized.
    // Protocol state-data is not available (memory not allocated).

    // This creates an empty HLA interface data (and associates it
    // to the EXTERNAL_Interface data struct.
    HlaInterfaceData* ifaceData;
    ifaceData = HlaMallocHla(iface->partition);
    iface->data = ifaceData;
    ifaceData->m_interface = iface;

    if (iface->partition->partitionId == 0)
    {
        HlaValidateConstants();
        HlaInitHlaVariable(iface, ifaceData);

        HlaMallocFederateAmbassador(ifaceData);
        HlaMallocRtiAmbassador(ifaceData);
        HlaReadParameters(iface->partition, ifaceData, nodeInput);
    }
}

void
HlaInitNodes(EXTERNAL_Interface* iface, NodeInput* nodeInput)
{
    if (iface->partition->partitionId != 0)
    {
        // For parallel we create messenger applications.
        // All the other hla init work is only done on partition 0.
        HlaInitMessenger(iface->partition, nodeInput);
        return;
    }
    // This function is called after nodes and protocols are initialized.
    // Protocol state-data is available.
    // (in parallel) This part is only invoked on partition 0.
    HlaInterfaceData* ifaceData = (HlaInterfaceData*) iface->data;
    PartitionData* partitionData = iface->partition;

    cout << endl;

    // The order of the three function calls below should NOT be changed.

    HlaReadEntitiesFile(partitionData, ifaceData, nodeInput);
    HlaReadRadiosFile(partitionData, ifaceData, nodeInput);
    HlaReadNetworksFile(partitionData, ifaceData, nodeInput);

    HlaMapDefaultDstRadioPtrsToRadios(ifaceData);

    HlaMapHierarchyIds(ifaceData);

    HlaInitMessenger(partitionData, nodeInput);

    HlaCreateFederation(ifaceData);
    HlaSleep(1);
    HlaJoinFederation(ifaceData);
    HlaRegisterCtrlCHandler();
    HlaGetObjectAndInteractionClassHandles(ifaceData);
    HlaSubscribeAndPublish(ifaceData);

    HlaWaitForFirstObjectDiscovery(ifaceData);
    cout.precision(6);

    if (ifaceData->m_hla->hlaDynamicStats)
    {
        if (ifaceData->m_hla->sendNodeIdDescriptions)
        {
            HlaSendNodeIdDescriptionNotifications(ifaceData);
        }

        if (ifaceData->m_hla->sendMetricDefinitions)
        {
            HlaSendMetricDefinitionNotifications(ifaceData);
        }

        HlaScheduleCheckMetricUpdate(ifaceData);
    }
}

void
HlaReceive(EXTERNAL_Interface* iface)
{
    HlaInterfaceData* ifaceData = (HlaInterfaceData*) iface->data;
    HlaCheckRequestAttributeUpdates(ifaceData);

    ifaceData->m_hlaRtiAmb->tick();
}

void
HlaProcessEvent(Node* node, Message* msg)
{
    HlaInterfaceData* ifaceData = HlaGetIfaceDataFromNode(node);
    assert(ifaceData->m_hla != NULL);

    switch (MESSAGE_GetEvent(msg))
    {
        case MSG_EXTERNAL_HLA_HierarchyMobility:
            HlaAppProcessMoveHierarchy(node, msg);
            break;
        case MSG_EXTERNAL_HLA_CheckMetricUpdate:
            HlaAppProcessCheckMetricUpdateEvent(node, msg);
            break;
        case MSG_EXTERNAL_HLA_AckTimeout:
            HlaAppProcessTimeoutEvent(node, msg);
            break;
#ifdef MILITARY_RADIOS_LIB
        case MSG_EXTERNAL_HLA_SendRtss:
            // The HLA Federation has created this message
            // in the method ScheduleRtssNotification by way of
            // ReflectRadioAttributes ()
            HlaProcessSendRtssEvent(node, msg);
            break;
        case MSG_EXTERNAL_HLA_SendRtssForwarded:
        {
            // See HlaProcessSendRtssEvent() for sender...
            // This messages was MSG_EXTERNAL_HLA_SendRtss, but
            // we are receiving this notification from a remote
            // partition and thus have to locate the node by nodeId.
            Node* rtssNode;
            RtssForwardedInfo* rtssForwardedInfo = (RtssForwardedInfo*)
                MESSAGE_ReturnInfo (msg);
            rtssNode = MAPPING_GetNodePtrFromHash(
                    node->partitionData->remoteNodeIdHash,
                    rtssForwardedInfo->nodeId);
            HlaSendRtssNotification(rtssNode);
            break;
        }
#endif /* MILITARY_RADIOS_LIB */
        case MSG_EXTERNAL_HLA_StartMessengerForwarded:
        {
            // See method SendSimulatedMsgUsingMessenger()
            // for sender...
            EXTERNAL_HlaStartMessenegerForwardedInfo* startInfo =
                (EXTERNAL_HlaStartMessenegerForwardedInfo*)
                MESSAGE_ReturnInfo (msg);
            Node* srcNode;
            srcNode = MAPPING_GetNodePtrFromHash(
                    node->partitionData->nodeIdHash,
                    startInfo->srcNodeId);

            StartSendSimulatedMsgUsingMessenger(
                srcNode,
                startInfo->srcNetworkAddress,
                startInfo->destNodeId,
                startInfo->smInfo,
                startInfo->requestedDataRate,
                startInfo->dataMsgSize,
                startInfo->voiceMsgDuration,
                startInfo->isVoice,
                startInfo->timeoutDelay,
                startInfo->unicast,
                startInfo->sendDelay);
            break;
        }
        case MSG_EXTERNAL_HLA_CompletedMessengerForwarded:
        {
            EXTERNAL_HlaCompletedMessenegerForwardedInfo* completedInfo =
                (EXTERNAL_HlaCompletedMessenegerForwardedInfo*)
                MESSAGE_ReturnInfo (msg);
            Node* destNode;
            destNode = MAPPING_GetNodePtrFromHash(
                    node->partitionData->remoteNodeIdHash,
                    completedInfo->destNodeId);
            HlaAppMessengerResultCompleted (destNode, completedInfo->smInfo,
                completedInfo->success);
            break;
        }
        case MSG_EXTERNAL_HLA_ChangeMaxTxPower:
            HlaAppProcessChangeMaxTxPowerEvent(node, msg);
            break;
        default:
            assert(0);

        // Sent from HlaProcessSendRtssEvent of non-0 partitions
    }

    MESSAGE_Free(node, msg);
}

void
HlaFinalize(EXTERNAL_Interface* iface)
{
    HlaInterfaceData* ifaceData = (HlaInterfaceData*) iface->data;
    if (iface->partition->partitionId == 0)
    {
        HlaLeaveFederation(ifaceData);
        HlaDestroyFederation(ifaceData);

        HlaFreeFederateAmbassador(ifaceData);
        HlaFreeRtiAmbassador(ifaceData);
    }

    HlaFreeEntities(ifaceData);
    HlaFreeRadios(ifaceData);
    HlaFreeNetworks(ifaceData);

    HlaFreeHla(ifaceData);
}

HlaInterfaceData*
HlaGetIfaceDataFromNode(Node* node)
{
    EXTERNAL_Interface* iface
        = node->partitionData->interfaceTable[EXTERNAL_HLA];
    return (HlaInterfaceData*) iface->data;
}

HlaInterfaceData*
HlaGetIfaceDataFromNodeId(NodeId nodeId)
{
    // Use global ifaceData to get nodeIdHash.
    // Then use nodeIdHash to get the node pointer for nodeId.

    Node* node = MAPPING_GetNodePtrFromHash(
                     g_ifaceData->m_hla->partitionData->nodeIdHash, nodeId);

    HlaVerify(node != NULL,
              "Can't get node pointer associated with nodeId",
              __FILE__, __LINE__);

    // Now we have the node pointer, return node's ifaceData.
    // TODO:  Check whether this makes sense.

    EXTERNAL_Interface* iface
        = node->partitionData->interfaceTable[EXTERNAL_HLA];
    return (HlaInterfaceData*) iface->data;
}

void
HlaUpdateMetric(
    unsigned nodeId,
    const MetricData& metric,
    void* metricValue,
    clocktype updateTime)
{
    // HlaUpdateMetric is called from gui.cpp as it sends stats updates
    // to the gui.
    // To obtain the hla context we have to find a node and from the node
    // partitionData to interface DS for HLA.
    HlaInterfaceData*   ifaceData;
    Hla*                theHla;

    ifaceData = HlaGetIfaceDataFromNodeId(nodeId);
    theHla = ifaceData->m_hla;

    assert(theHla->metricUpdateSize
           <= sizeof(theHla->metricUpdateBuf));
    assert(updateTime >= 0);

    char metricValueString[g_hlaMetricUpdateBufSize];

    switch (metric.metricDataType)
    {
        case GUI_INTEGER_TYPE:
            sprintf(metricValueString, "%d", *((int*) metricValue));
            break;
        case GUI_DOUBLE_TYPE:
            sprintf(metricValueString, "%f", *((double*) metricValue));
            break;
        case GUI_UNSIGNED_TYPE:
            sprintf(metricValueString, "%u", *((unsigned*) metricValue));
            break;
        default:
            assert(0);
    }

    char updateTimeString[g_hlaClocktypeStringBufSize];
    ctoa(updateTime, updateTimeString);

    char line[g_hlaMetricUpdateBufSize];

    if (theHla->verboseMetricUpdates)
    {
        assert(theHla->partitionData != NULL);

        Node* node
            = MAPPING_GetNodePtrFromHash(
                  theHla->partitionData->nodeIdHash, nodeId);

        if (node == NULL)
        {
            HlaReportWarning("HLA PARALLEL doesn't yet support GUI updates"
                " from nodes on other partitions.");
            return;
        }

        HlaNodeIdToPerNodeDataMap::iterator it
            = theHla->nodeIdToPerNodeData.find(node->nodeId);
        HlaData& hlaData = it->second;

        if (hlaData.radioPtr != NULL)
        {
            assert(hlaData.radioPtr->entityPtr != NULL);

            const HlaRadio&  radio  = *hlaData.radioPtr;
            const HlaEntity& entity = *hlaData.radioPtr->entityPtr;

            if (!entity.mappedToHandle) { return; }

            sprintf(
                line,
                "%u \"%s\" %s %u %d \"%s\" %d %d %s %s\n",
                nodeId,
                entity.markingData,
                entity.entityIdString,
                radio.radioIndex,
                metric.metricID,
                metric.metricName,
                metric.metricLayerID,
                metric.metricDataType,
                metricValueString,
                updateTimeString);
        }
        else
        {
            sprintf(
                line,
                "%u \"\" -1 -1 %d \"%s\" %d %d %s %s\n",
                nodeId,
                metric.metricID,
                metric.metricName,
                metric.metricLayerID,
                metric.metricDataType,
                metricValueString,
                updateTimeString);
        }
    }
    else
    {
        sprintf(
            line,
            "%u %d %s %s\n",
            nodeId,
            metric.metricID,
            metricValueString,
            updateTimeString);
    }

    unsigned lineLength = strlen(line);
    assert(lineLength < sizeof(line));

    unsigned potentialMetricUpdateSize
        = theHla->metricUpdateSize + lineLength;

    if (theHla->metricUpdateSize == 1)
    {
        assert(potentialMetricUpdateSize
               <= sizeof(theHla->metricUpdateBuf));
    }

    if (potentialMetricUpdateSize <= sizeof(theHla->metricUpdateBuf))
    {
        strcat(theHla->metricUpdateBuf, line);
        theHla->metricUpdateSize = potentialMetricUpdateSize;

        return;
    }

    HlaSendMetricUpdateNotification(ifaceData);

    potentialMetricUpdateSize
        = theHla->metricUpdateSize + lineLength;

    assert(potentialMetricUpdateSize
           <= sizeof(theHla->metricUpdateBuf));

    strcat(theHla->metricUpdateBuf, line);
    theHla->metricUpdateSize = potentialMetricUpdateSize;
}

#ifdef MILITARY_RADIOS_LIB
void
HlaSendRtssNotificationIfNodeIsHlaEnabled(Node* node)
{
    if (HlaIsActive())
    {
        // Various radio macs will invoke this to indicate ready to send.
        // If this node is on a partition other than 0 (and thus
        // has no actually HLA federation) a message will be created
        // to forward the rtss to p0.
        HlaProcessSendRtssEvent(node, NULL);
        return;
    }
}
#endif /* MILITARY_RADIOS_LIB */
