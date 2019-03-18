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

// /**
// PACKAGE :: MAPPING
// DESCRIPTION :: This file describes data structures and functions for mapping between node pointers,
//                node identifiers, and node addresses.
// **/

#ifndef MAPPING_H
#define MAPPING_H

// /**
// CONSTANT :: INVALID_MAPPING : 0xffffffff
// DESCRIPTION :: Indicates Invalid Mapping
// **/
#define INVALID_MAPPING 0xffffffff

// /**
// CONSTANT :: MAX_INTERFACE_ADDRESSES : 2
// DESCRIPTION :: max no of addressees assigned to an interface
// **/
#define MAX_INTERFACE_ADDRESSES 2


// /**
// CONSTANT :: NODE_HASH_SIZE : 32
// DESCRIPTION ::
//      Defines node hash size. Hashes the nodeIds using a mod
//      NODE_HASH_SIZE hash.
// **/
#define NODE_HASH_SIZE  32

// /**
// STRUCT :: NetworkProperty
// DESCRIPTION :: Describes the property of a network.
//
typedef union network_property_struct
{
    struct
    {
        NodeAddress u_numHostBits;
        NodeAddress u_subnetMask;
        NodeAddress u_subnetAddress;
    } ipv4;
    struct
    {
        unsigned int u_subnetPrefixLen;
        in6_addr     u_subnetAddress;
    } ipv6;

    // Here we introduce ATM related property. User specify ATM network
    // Address by the following string:
    // 'ATM-LINK ICD-<value>.AID-<value>.PTP-<value> {node ids}'.
    struct
    {
        unsigned int u_icd; // store the ICD value.
        unsigned int u_aid; // store the AID value.
        unsigned int u_ptp; // store the PTP value.
    } atm;

} NetworkProperty;

//
// ENUM         :: NetworkProtocolType
// DESCRIPTION  :: Types of various nodes
//
typedef enum
{
    INVALID_NETWORK_TYPE,
    IPV4_ONLY,
    IPV6_ONLY,
    DUAL_IP,
    ATM_NODE,
    GSM_LAYER3,
    CELLULAR,
    NETWORK_VIRTUAL
}
NetworkProtocolType;


// /**
// STRUCT :: AddressMappingType
// DESCRIPTION :: Describes the type of address mapping.
// **/
typedef
struct
{
    Address address;     // key
    unsigned int nodeId;
    NetworkProperty networkProp;
    int interfaceIndex;
    int oppositeMappingIndex;
}
AddressMappingType;

//
// STRUCT :: AddressInfo
// DESCRIPTION :: information of all the addresses assigned to the interface.
//
typedef
struct
{
    Address address;
    NetworkProperty networkProp;
    int oppositeMappingIndex;
}
AddressInfo;

// /**
// STRUCT :: AddressReverseMappingType
// DESCRIPTION :: Describes the type of reverse address mapping.
// **/
typedef
struct
{
    unsigned int nodeId;    // key
    int interfaceIndex;
    NetworkProtocolType netProtoType;
    int noOfAddresses;
    AddressInfo addressInfo[MAX_INTERFACE_ADDRESSES];
}
AddressReverseMappingType;


// /**
// STRUCT :: SubnetListType
// DESCRIPTION ::
//      Used to determine what the next address counter should be for each
//      subnet address. This is needed to allow different SUBNET/LINK
//      statements to declare the same subnet address.
// **/

typedef struct
{

    //Subnet network type.
    NetworkType networkType;

    union
    {
        NodeAddress subnetAddr;
        struct
        {
            unsigned int u_subnetPrefixLen;
            in6_addr     u_subnetAddress;
        } ipv6;

        // Here we introduce ATM related network property. User specify
        // ATM network by the following string:
        // 'ATM-LINK ICD-<value>.AID-<value>.PTP-<value> {node ids}'.
        struct
        {
            unsigned int u_icd; // store the ICD value.
            unsigned int u_aid; // store the AID value.
            unsigned int u_ptp; // store the PTP value.
        } atm;

    } addrQual;
    unsigned int addressCounter;
} SubnetListType;

#define HOST_BITS   networkProp.ipv4.u_numHostBits
#define SUBNET_MASK networkProp.ipv4.u_subnetMask
#define SUBNET_ADDR networkProp.ipv4.u_subnetAddress

#define list_subnet_addr addrQual.subnetAddr
#define list_TLA addrQual.ipv6.u_tla
#define list_NLA addrQual.ipv6.u_nla
#define list_SLA addrQual.ipv6.u_sla

#define list_IPV6_SUBNET_ADDR addrQual.ipv6.u_subnetAddress
#define list_IPV6_PREFIX_LEN addrQual.ipv6.u_subnetPrefixLen

#define list_ICD addrQual.atm.u_icd
#define list_AID addrQual.atm.u_aid
#define list_PTP addrQual.atm.u_ptp

#define TLA networkProp.ipv6.u_tla
#define NLA networkProp.ipv6.u_nla
#define SLA networkProp.ipv6.u_sla

#define IPV6_SUBNET_ADDR networkProp.ipv6.u_subnetAddress
#define IPV6_PREFIX_LEN networkProp.ipv6.u_subnetPrefixLen

#define IPV4_ADDR    address.interfaceAddr.ipv4
#define IPV6_ADDR    address.interfaceAddr.ipv6
#define NETWORK_TYPE address.networkType

// /**
// STRUCT :: AddressMapType
// DESCRIPTION ::
//      Describes the detailed information of Node ID <--> IP address
//      mappings.
// **/
typedef
struct
{
    int numMappings;
    int numAllocatedMappings;

    int numReverseMappings;
    int numAllocatedReverseMappings;

    AddressMappingType *mappings;
    AddressReverseMappingType *reverseMappings;

    int numSubnets;
    int numAllocatedSubnets;

    SubnetListType *subnetList;
}
AddressMapType;

typedef struct nodeIdToNodePtr * IdToNodePtrMap;

// /**
// STRUCT :: nodeIdToNodePtr
// DESCRIPTION ::
//      Describes the nodeId and corresponding nodePtr.
// **/
struct nodeIdToNodePtr
{
    NodeAddress nodeId;
    Node* nodePtr;
    IdToNodePtrMap next;
};

//---------------------------------------------------------------------------
// Some address related macros and functions.
//---------------------------------------------------------------------------

// /**
// MACRO    :: MADDR6_SCOPE(a)
// DESCRIPTION :: Multicast Address Scope.
// **/
#ifndef MADDR6_SCOPE
#define MADDR6_SCOPE(a) \
        MADDR6_SCP_LINK
#endif

// /**
// MACRO    :: IS_MULTIADDR6(a)
// DESCRIPTION :: Checks whether an address is multicast address.
// **/
#define IS_MULTIADDR6(a)    ((a).s6_addr8[0] == 0xff)

// /**
// MACRO    :: CLR_ADDR6(a)
// DESCRIPTION :: Set an address with 0 values.
// **/
#ifndef CLR_ADDR6
#define CLR_ADDR6(a) \
    do { \
        (a).s6_addr32[0] = 0; \
        (a).s6_addr32[1] = 0; \
        (a).s6_addr32[2] = 0; \
        (a).s6_addr32[3] = 0; \
    } while (0)
#endif

// /**
// MACRO    :: IS_CLR_ADDR6(a)
// DESCRIPTION :: Does an address have the value of 0 (Cleared).
// **/
#ifndef IS_CLR_ADDR6
#define IS_CLR_ADDR6(a) \
        ((a).s6_addr32[0] == 0 && \
         (a).s6_addr32[1] == 0 && \
         (a).s6_addr32[2] == 0 && \
         (a).s6_addr32[3] == 0 )
#endif

// /**
// MACRO    :: COPY_ADDR6(from, to)
// DESCRIPTION :: Copies from-ipv6 address to to-ipv6 address.
// **/
#ifndef COPY_ADDR6
#define COPY_ADDR6(from, to) \
    do { \
        (to).s6_addr32[0] = (from).s6_addr32[0]; \
        (to).s6_addr32[1] = (from).s6_addr32[1]; \
        (to).s6_addr32[2] = (from).s6_addr32[2]; \
        (to).s6_addr32[3] = (from).s6_addr32[3]; \
    } while (0)
#endif

// /**
// MACRO    :: SAME_ADDR6(a, b)
// DESCRIPTION :: Checks if a and b address is same address.
// **/
#define SAME_ADDR6(a, b) \
    (((a).s6_addr32[0] == (b).s6_addr32[0]) && \
     ((a).s6_addr32[1] == (b).s6_addr32[1]) && \
     ((a).s6_addr32[2] == (b).s6_addr32[2]) && \
     ((a).s6_addr32[3] == (b).s6_addr32[3]))

// /**
// MACRO    :: IS_ANYADDR6(a)
// DESCRIPTION :: Checks whether the address is any address or not.
// **/
#define IS_ANYADDR6(a) \
    (((a).s6_addr32[0] == 0) && \
     ((a).s6_addr32[1] == 0) && \
     ((a).s6_addr32[2] == 0) && \
     ((a).s6_addr32[3] == 0))

// /**
// MACRO    :: IS_LOOPADDR6(a)
// DESCRIPTION :: Checks whether it is loopback address.
// **/
#define IS_LOOPADDR6(a) \
    (((a).s6_addr32[0] == 0) && \
     ((a).s6_addr32[1] == 0) && \
     ((a).s6_addr32[2] == 0) && \
     ((a).s6_addr32[3] == 1))

// /**
// MACRO    :: CMP_ADDR6(a, b)
// DESCRIPTION :: Compaires two addresses.
// **/
#define CMP_ADDR6(a, b) Ipv6CompareAddr6(a, b)

// /**
// MACRO    :: IS_IPV4ADDR6(a)
// DESCRIPTION :: Checks whether it is ipv4 address.
// **/
#ifndef IS_IPV4ADDR6
#define IS_IPV4ADDR6(a) \
    (((a).s6_addr32[0] == 0) && \
     ((a).s6_addr32[1] == 0) && \
     ((a).s6_addr32[2] == 0))
#endif

// /**
// MACRO    :: IS_LOCALADDR6(a)
// DESCRIPTION :: Checks whether it is local address.
// **/
#define IS_LOCALADDR6(a)    ((a).s6_addr8[0] == 0xfe)

// /**
// MACRO    :: IS_LINKLADDR6(a)
// DESCRIPTION :: Checks whether it is link local address.
// **/
#define IS_LINKLADDR6(a) \
    (IS_LOCALADDR6(a) && ((a).s6_addr8[1] == 0x80))

// /**
// MACRO    :: IS_SITELADDR6(a)
// DESCRIPTION :: Checks whether it is site local address.
// **/
#define IS_SITELADDR6(a) \
    (IS_LOCALADDR6(a) && ((a).s6_addr8[1] == 0xc0))

// /**
// MACRO    :: SAME_ADDR4(a, b)
// DESCRIPTION :: Checks whether IPv4 addresses match.
// **/
#define SAME_ADDR4(a, b) ((a) == (b))

// /**
// MACRO    :: IS_ANYADDR4(a)
// DESCRIPTION :: Checks whether IPv4 address is ANY_DEST.
// **/
#define IS_ANYADDR4(a) ((a) == ANY_DEST)

// /**
// API                 :: Address_IsSameAddress
// LAYER               :: Network
// PURPOSE             :: Check whether both addresses(i.e. addr1 and addr2)
//                        are same.
// PARAMETERS          ::
// + addr1               : Address* addr1 : Pointer to 1st address
// + addr2               : Address* addr2 : Pointer to 2nd address
// RETURN              :: BOOL :
// **/
BOOL Address_IsSameAddress(Address* addr1, Address* addr2);

// /**
// API                 :: Address_IsAnyAddress
// LAYER               :: Network
// PURPOSE             :: Check whether addr is any address of the same type
// PARAMETERS          ::
// + addr               : Address* addr         : Pointer to address
// RETURN              :: BOOL :
// **/
BOOL Address_IsAnyAddress(Address* addr);

// /**
// API                 :: Address_IsMulticastAddress
// LAYER               :: Network
// PURPOSE             :: Check whether addr is a multicast address
// PARAMETERS          ::
// + addr               : Address* addr         : Pointer to address
// RETURN              :: BOOL :
// **/
BOOL Address_IsMulticastAddress(Address* addr);


// /**
// API                 :: Address_IsSubnetBroadcastAddress
// LAYER               :: Network
// PURPOSE             :: Check whether addr is a subnet broadcast address
// PARAMETERS          ::
// + node               : Node* node            : pointer to node
// + addr               : Address* addr         : Pointer to address
// RETURN              :: BOOL :
// **/
BOOL Address_IsSubnetBroadcastAddress(Node *node, Address* addr); 

// /**
// API                 :: Address_SetToAnyAddress
// LAYER               :: Network
// PURPOSE             :: Set addr to any address of the same type as refAddr.
// PARAMETERS          ::
// + addr               : Address* addr         : Pointer to address
// + refAddr            : Address* refAddr      : Pointer to refAddr
// RETURN              :: void :
// **/
void Address_SetToAnyAddress(Address* addr, Address* refAddr);

// /**
// API                 :: Address_AddressCoopy
// LAYER               :: Network
// PURPOSE             :: Copy srcAddress to dstAddress
// PARAMETERS          ::
// + dstAddress         : Address*  : Destination address
// + srcAddress         : Address*  : Source address
// RETURN              :: void : NULL
// **/
void MAPPING_AddressCopy(Address* dstAddress, Address* srcAddress);

// /**
// API                 :: Ipv6CompareAddr6
// LAYER               :: Network
// PURPOSE             :: Compairs to ipv6 address. if a is greater than
//                        b then returns positive, if equals then 0,
//                        a is smaller then b then negative.
// PARAMETERS          ::
// + a                  : in6_addr      : ipv6 address.
// + b                  : in6_addr      : ipv6 address.
// RETURN              :: int :
// **/
int Ipv6CompareAddr6(in6_addr a, in6_addr b);

// /**
// API                 :: Ipv6IsAddressInNetwork
// LAYER               :: Network
// PURPOSE             :: Checks whether the address is in the same network.
//                      : if in the same network then returns TRUE,
//                        otherwise FALSE.
// PARAMETERS          ::
// + globalAddr         : const in6_addr* : Pointer to ipv6 address.
// + tla                : unsigned int    : Top level ipv6 address.
// + vla                : unsigned int    : Next level ipv6 address.
// + sla                : unsigned int    : Site local ipv6 address.
// RETURN              :: BOOL :
// **/
BOOL
Ipv6IsAddressInNetwork(
    const in6_addr* globalAddr,
    unsigned int tla,
    unsigned int nla,
    unsigned int sla);


// /**
// API                 :: Ipv6IsAddressInNetwork
// LAYER               :: Network
// PURPOSE             :: Checks whether the address is in the same network.
//                      : if in the same network then returns TRUE,
//                        otherwise FALSE.
// PARAMETERS          ::
// + globalAddr         : const in6_addr* : Pointer to ipv6 address.
// + ipv6SubnetAddr     : const in6_addr* : Pointer to ipv6 subnet address.
// + prefixLenth        : unsigned int    : prefix length of the address.
// RETURN              :: BOOL            : TRUE if the address is in the
//                                          same network, FALSE otherwise
// **/
BOOL
Ipv6IsAddressInNetwork(
    const in6_addr* globalAddr,
    const in6_addr* ipv6SubnetAddr,
    unsigned int prefixLenth);


// /**
// FUNCTION            :: Ipv6CheckNetworkParams
// LAYER               :: Network
// PURPOSE             :: Checks network parameters (tla, nla, sla)
// PARAMETERS          ::
// + tla                : unsigned int tla: Top level aggregation.
// + nla                : unsigned int nla: Next level aggregation.
// + sla                : unsigned int sla: Site level aggregaton.
// RETURN              :: BOOL
// **/
BOOL
Ipv6CheckNetworkParams(
    unsigned int tla,
    unsigned int nla,
    unsigned int sla);

//---------------------------------------------------------------------------
// End of address related code.
//---------------------------------------------------------------------------

// /**
// API :: MAPPING_HashNodeId
// PURPOSE ::
//      Hashes the nodeIds using a mod NODE_HASH_SIZE hash. This
//      is not thread safe.
// PARAMETERS ::
// + hash    : IdToNodePtrMap* : IdToNodePtrMap pointer
// + nodeId  : NodeAddress     : Node id.
// + nodePtr : Node*           : Node poniter
// RETURN :: void :
// **/
void
MAPPING_HashNodeId(
    IdToNodePtrMap* hash,
    NodeAddress     nodeId,
    Node*           nodePtr);


// /**
// API :: MAPPING_GetNodePtrFromHash
// PURPOSE :: Retrieves the node pointer for nodeId from hash.
// PARAMETERS ::
// + hash   : IdToNodePtrMap* : IdToNodePtrMap pointer
// + nodeId : NodeAddress     : Node id.
// RETURN :: Node* : Node pointer for nodeId.
// **/
Node*
MAPPING_GetNodePtrFromHash(
    IdToNodePtrMap* hash,
    NodeAddress     nodeId);


// /**
// API :: MAPPING_MallocAddressMap
// PURPOSE :: Allocates memory block of size AddressMapType.
// PARAMETERS ::
// RETURN :: AddressMapType* : Pointer to a new AddressMapType structure.
// **/
AddressMapType*
MAPPING_MallocAddressMap();


// /**
// API :: MAPPING_InitAddressMap
// PURPOSE :: Initializes the AddressMapType structure.
// PARAMETERS ::
// + map : AddressMapType* : A pointer of type AddressMapType.
// RETURN :: void :
// **/
void
MAPPING_InitAddressMap(AddressMapType *map);


// /**
// API :: MAPPING_BuildAddressMap
// PURPOSE :: Builds the address map
// PARAMETERS ::
// + nodeInput      : const NodeInput* : A pointer to const NodeInput.
// + nodeIdArrayPtr : NodeAddress**    : A pointer to pointer of NodeAddress
// + map            : AddressMapType*  : A pointer of type AddressMapType.
// RETURN :: void :
// **/
void
MAPPING_BuildAddressMap(
    const NodeInput *nodeInput,
    int *numNodes,
    NodeAddress **nodeIdArrayPtr,
    AddressMapType *map);


// /**
// API :: MAPPING_GetInterfaceAddressForSubnet
// PURPOSE :: Gives Interface address for a Subnet.
// PARAMETERS ::
// + node          : Node*       : A pointer to node being initialized
// + nodeId        : NodeAddress : Node id
// + subnetAddress : NodeAddress : Subnet address
// + numHostBits   : int         : Number of host bits
// RETURN :: NodeAddress : Interface address for the subnet.
// **/
NodeAddress
MAPPING_GetInterfaceAddressForSubnet(
    Node *node,
    NodeId nodeId,
    NodeAddress subnetAddress,
    int numHostBits);


// /**
// API :: MAPPING_GetInterfaceAddressForSubnet
// PURPOSE :: Gives Interface address for a Subnet.
// PARAMETERS ::
// + map           : const AddressMapType* : A pointer to address map
// + nodeId        : NodeAddress : Node id
// + subnetAddress : NodeAddress : Subnet address
// + numHostBits   : int         : Number of host bits
// RETURN :: NodeAddress : Interface address for the subnet.
// **/
NodeAddress
MAPPING_GetInterfaceAddressForSubnet(
    const AddressMapType* map,
    NodeId nodeId,
    NodeAddress subnetAddress,
    int numHostBits);


// /**
// API :: MAPPING_GetSubnetAddressForInterface
// PURPOSE :: Gives the Subnet address for an interface.
// PARAMETERS ::
// + node           : Node*       : A pointer to node being initialized.
// + nodeId         : NodeAddress : Node id
// + interfaceIndex : int         : Interface index
// RETURN :: NodeAddress : Subnet address for an interface.
// **/
NodeAddress
MAPPING_GetSubnetAddressForInterface(
    Node *node,
    NodeAddress nodeId,
    int interfaceIndex);


// /**
// API :: MAPPING_GetSubnetMaskForInterface
// PURPOSE :: Gives the Subnet mask for an interface.
// PARAMETERS ::
// + node           : Node*       : A pointer to node being initialized.
// + nodeId         : NodeAddress : Node id
// + interfaceIndex : int         : Interface index
// RETURN :: NodeAddress : Subnet mask for an interface.
// **/
NodeAddress
MAPPING_GetSubnetMaskForInterface(
    Node *node,
    NodeAddress nodeId,
    int interfaceIndex);


// /**
// API :: MAPPING_GetNumHostBitsForInterface
// PURPOSE :: Gives the number of host bits for an interface.
// PARAMETERS ::
// + node           : Node*       : A pointer to node being initialized.
// + nodeId         : NodeAddress : Node id
// + interfaceIndex : int         : Interface index
// RETURN :: int : The number of host bits for an interface.
// **/
int
MAPPING_GetNumHostBitsForInterface(
    Node *node,
    NodeAddress nodeId,
    int interfaceIndex);


// /**
// API :: MAPPING_GetInterfaceInfoForInterface
// PURPOSE :: Gives the Interface information for an interface.
// PARAMETERS ::
// + node             : Node*        : A pointer to node being initialized.
// + nodeId           : NodeAddress  : Node id
// + interfaceIndex   : int          : Interface index
// + interfaceAddress : NodeAddress* : Interface address, int pointer.
// + subnetAddress    : NodeAddress* : Subnet address, NodeAddress pointer.
// + subnetMask       : NodeAddress* : Subnet mask, NodeAddress pointer.
// + numHostBits      : int*         : Number of host bits, int pointer.
// RETURN :: void :
// **/
void
MAPPING_GetInterfaceInfoForInterface(
    Node *node,
    NodeAddress nodeId,
    int interfaceIndex,
    NodeAddress *interfaceAddress,
    NodeAddress *subnetAddress,
    NodeAddress *subnetMask,
    int *numHostBits);


// /**
// API :: MAPPING_GetInterfaceAddressForInterface
// PURPOSE :: Gives the Interface address for an interface.
// PARAMETERS ::
// + node           : Node*       : A pointer to the node being initialized.
// + nodeId         : NodeAddress : Node id
// + interfaceIndex : int         : Interface index
// RETURN :: NodeAddress : Interface address for an interface.
// **/
NodeAddress
MAPPING_GetInterfaceAddressForInterface(
    Node *node,
    NodeAddress nodeId,
    int interfaceIndex);

// /**
// API          :: MAPPING_GetInterfaceAddressForInterface
// PURPOSE      :: Get node interface Address according to the network
//                 specific interface index.
//                 Overloaded function for ATM compatibility.
// PARAMETERS    ::
// + netType        : NetworkType : Network type of the interface.
// + relativeInfInx : int         : Inrerface index related to networkType.
// RETURN       :: Address : Return Address.
// **/
// NOTE         :: If relativeInfInx is not valid, networkType of return
//                 Address is NETWORK_INVALID.
Address
MAPPING_GetInterfaceAddressForInterface(
    Node *node,
    NodeAddress nodeId,
    NetworkType netType,
    int relativeInfInx);


// /**
// API :: MAPPING_GetNodeIdFromInterfaceAddress
// PURPOSE :: Gives Node id from an interface address.
// PARAMETERS ::
// + node             : Node*       : A pointer to node being initialized.
// + interfaceAddress : NodeAddress : Interface address
// RETURN :: NodeAddress :
// **/
NodeAddress
MAPPING_GetNodeIdFromInterfaceAddress(
    Node *node,
    NodeAddress interfaceAddress);


// /**
// API :: MAPPING_GetNodeIdFromInterfaceAddress
// PURPOSE :: Gives Node id from an interface address.
//            Overloaded for IPv6
// PARAMETERS ::
// + node             : Node*   : A pointer to node being initialized.
// + interfaceAddress : Address : Interface address
// RETURN :: NodeAddress :
// **/
NodeAddress
MAPPING_GetNodeIdFromInterfaceAddress(
    Node *node,
    Address interfaceAddress);

// /**
// API :: MAPPING_GetDefaultInterfaceAddressFromNodeId
// PURPOSE :: Gives default interface address from a node id.
// PARAMETERS ::
// + node   : Node*       : A pointer to node being initialized.
// + nodeId : NodeAddress : Node id
// RETURN :: NodeAddress : Default interface address from the node id.
// **/
NodeAddress
MAPPING_GetDefaultInterfaceAddressFromNodeId(
    Node *node,
    NodeAddress nodeId);


// /**
// API :: MAPPING_GetNumNodesInSubnet
// PURPOSE :: Gives the number of nodes in a subnet.
// PARAMETERS ::
// + node          : Node*       : A pointer to node being initialized.
// + subnetAddress : NodeAddress : Subnet address
// RETURN :: unsigned int : Number of nodes in a subnet.
// **/
unsigned int
MAPPING_GetNumNodesInSubnet(
    Node *node,
    NodeAddress subnetAddress);


// /**
// API :: MAPPING_GetSubnetAddressCounter
// PURPOSE :: Gives the subnet address counter.
// PARAMETERS ::
// + map           : AddressMapType* : A pointer to AddressMapType.
// + subnetAddress : NodeAddress     : Subnet address
// RETURN :: unsigned int : The subnet address counter.
// **/
unsigned int
MAPPING_GetSubnetAddressCounter(
    AddressMapType *map,
    NodeAddress subnetAddress);


// /**
// API :: MAPPING_UpdateSubnetAddressCounter
// PURPOSE :: Updates the subnet address counter.
// PARAMETERS ::
// + map            : AddressMapType* : A pointer to AddressMapType.
// + subnetAddress  : NodeAddress     : Subnet address
// + addressCounter : int             : Address counter
// RETURN :: void :
// **/
void
MAPPING_UpdateSubnetAddressCounter(
    AddressMapType *map,
    NodeAddress subnetAddress,
    int addressCounter);

// /**
// API :: MAPPING_GetInterfaceIndexFromInterfaceAddress
// PURPOSE :: Gets the node's interface index for the given address.
// PARAMETERS ::
// + node             : Node*       : A pointer to node being initialized.
// + interfaceAddress : NodeAddress : Interface address
// RETURN :: int : The interface index.
// **/
int
MAPPING_GetInterfaceIndexFromInterfaceAddress(
    Node *node,
    NodeAddress interfaceAddress);

// /**
// API          :: MAPPING_GetNodeInfoFromAtmNetInfo
// PURPOSE      :: Get node interface Address, generic interfaceIndex and
//                 Atm related interfaceIndex from ATM Network information.
// PARAMETERS    ::
// + index       : unsigned int* : return atm related interface index of a
//                                 node.
// + genIndex    : unsigned int* : return generic interface index of a node.
// RETURN       :: Address : Return valid ATM Address related to Network information
//                           if genIndex is not equal to -1.
// **/
Address
MAPPING_GetNodeInfoFromAtmNetInfo(
    Node *node,
    int nodeId,
    int* index,
    int* genIndex,
    unsigned int* u_atmVal,
    unsigned char afi = 0x47);

// /**
// API          :: MAPPING_GetInterfaceIdForDestAddress
// PURPOSE      :: For a given destination address find its interface index
// PARAMETERS   ::
// + node       : Node*  : A pointer to node being initialized.
// + nodeId     : NodeId : Node ID
// + destAddr   : NodeAddress : Destination address.
// RETURN       :: unsigned int :
// **/
unsigned int
MAPPING_GetInterfaceIdForDestAddress(
    Node *node,
    NodeId nodeId,
    NodeAddress destAddr);

// /**
// API          :: MAPPING_GetSubnetMaskForDestAddress
// PURPOSE      :: For a given nodeId & destination address
//                 find the subnet mask for the associated network
// PARAMETERS   ::
// + node       : Node*  : A pointer to node being initialized.
// + nodeId     : NodeId : Node ID
// + destAddr   : NodeAddress : Destination address.
// RETURN       :: NodeAddress :
// **/
NodeAddress
MAPPING_GetSubnetMaskForDestAddress(
    Node *node,
    NodeId nodeId,
    NodeAddress destAddr);

// /**
// API          :: MAPPING_GetInterfaceAddrForNodeIdAndIntfId
// PURPOSE      :: For a given nodeId & InterfaceId
//                 find the associated IP-Address
// PARAMETERS   ::
// + node       : Node*  : The pointer to the node.
// + nodeId     : NodeId : Node ID
// + intfId     : int   : Interface ID.
// RETURN       :: NodeAddress :
// **/
NodeAddress
MAPPING_GetInterfaceAddrForNodeIdAndIntfId(
    Node *node,
    NodeId nodeId,
    int intfId);

// /**
// API          :: MAPPING_GetIPv6NetworkAddressCounter
// PURPOSE      :: Get IPV6 network address counter.
// PARAMETERS   ::
// + map                : AddressMapType*: The address map.
// + subnetAddr         : in6_addr : The IPv6 address.
// + subnetPrefixLen    : unsigned int : The prefix length.
// RETURN       :: unsigned int : The current counter.
// **/
unsigned int
MAPPING_GetIPv6NetworkAddressCounter(
        AddressMapType *map,
        in6_addr subnetAddr,
        unsigned int subnetPrefixLen);

// /**
// API          :: MAPPING_UpdateIPv6NetworkAddressCounter
// PURPOSE      :: Update IPV6 network address counter.
// PARAMETERS   ::
// + map                : AddressMapType* : The address map.
// + subnetAddr         : in6_addr : The IPv6 address.
// + subnetPrefixLen    : unsigned int : The prefix length.
// + addressCounter     : int : The new counter value.
// RETURN       :: void :
// **/
void
MAPPING_UpdateIPv6NetworkAddressCounter(
    AddressMapType *map,
    in6_addr subnetAddr,
    unsigned int subnetPrefixLen,
    int addressCounter);

// /**
// API              :: MAPPING_GetNumNodesInIPv6Network
// PURPOSE          :: Get Num of nodes in IPV6 network.
// PARAMETERS       ::
// + node               : Node* :  The pointer to the node.
// + subnetAddr         : in6_addr : The IPv6 address.
// + subnetPrefixLen    : unsigned int : The prefix length.
// RETURN           :: unsigned int :
// **/
unsigned int
MAPPING_GetNumNodesInIPv6Network(
    Node *node,
    in6_addr subnetAddr,
    unsigned int subnetPrefixLen);

// /**
// API          :: MAPPING_GetNetworkIPVersion
// PURPOSE      :: Get Network version IPv4/IPv6.
// PARAMETERS   ::
// + addrString     : const char* : The address string
// RETURN       :: NetworkType :
// **/
NetworkType
MAPPING_GetNetworkIPVersion(const char* addrString);

// /**
// API         :: MAPPING_GetNetworkType
// DESCRIPTION  :: Identify network type from addrString.
// PARAMETERS   ::
// + addrString     : const char* : The address string
// RETURN       :: NetworkType :
// **/
NetworkType
MAPPING_GetNetworkType(const char* addrString);

// /**
// API              :: MAPPING_GetIpv6InterfaceInfoForInterface
// PURPOSE          :: Get IPV6 interface information for a interface.
// PARAMETERS       ::
// + node               : Node *node : The node.
// + nodeId             : NodeId nodeId: Node Id
// + interfaceIndex     : int : The interface index.
// + globalAddr         : in6_addr* : The global IPv6 address.
// + subnetAddr         : in6_addr* :  The subnet IPv6 address.
// + subnetPrefixLen    : unsigned int* : THe subnet prefex length.
// RETURN           :: void :
// **/
void
MAPPING_GetIpv6InterfaceInfoForInterface(
    Node *node,
    NodeId nodeId,
    int interfaceIndex,
    in6_addr* globalAddr,
    in6_addr* subnetAddr,
    unsigned int* subnetPrefixLen);

// /**
// API          :: MAPPING_GetIpv6GlobalAddress:
// PURPOSE      :: Get IPV6 global address.
// PARAMETERS   ::
// + node       : Node *node : The node
// + nodeId     : NodeId nodeId : The node's id
// + tla        : unsigned int : Top level aggregation
// + nla        : unsigned int : Next level aggregation
// + sla        : unsigned int : Site level aggregation
// + addr6      : in6_addr * : The global IPv6 address.
// RETURN       :: BOOL :
// **/
BOOL
MAPPING_GetIpv6GlobalAddress(
    Node *node,
    NodeId nodeId,
    unsigned int tla,
    unsigned int nla,
    unsigned int sla,
    in6_addr *addr6);

// /**
// API          :: MAPPING_GetIpv6GlobalAddressForInterface
// PURPOSE      :: Get IPV6 global address for a node's nth interface.
// PARAMETERS   ::
// + node           : Node * : The node
// + nodeId         : NodeId : The node's id
// + interfaceIndex : int : The interface index.
// + addr6          : in6_addr * : The global IPv6 address.
// + isDeprecated   : BOOL : Return deprecated address (if valid)
// RETURN       :: BOOL :
// **/
BOOL
MAPPING_GetIpv6GlobalAddressForInterface(
    Node *node,
    NodeAddress nodeId,
    int interfaceIndex,
    in6_addr *addr6);

// /**
// API          :: MAPPING_CreateIpv6GlobalUnicastAddr
// PURPOSE      :: Create IPv6 Global Unicast Address from tla nla sla:
// PARAMETERS   ::
// + tla                : unsigned int : Top level aggregation
// + nla                : unsigned int : Next level aggregation
// + sla                : unsigned int : Site level aggregation
// + addressCounter     : int : The address counter.
// + globalAddr         : in6_addr* : The global IPv6 address.
// RETURN   :: void :
// **/
void
MAPPING_CreateIpv6GlobalUnicastAddr(
    unsigned int tla,
    unsigned int nla,
    unsigned int sla,
    int addressCounter,
    in6_addr* globalAddr);

// /**
// FUNCTION             :: MAPPING_CreateIpv6GlobalUnicastAddr
// PURPOSE              :: Create IPv6 Global Unicast Address.
// PARAMETERS           ::
// + map                : AddressMapType * : The address map.
// + IPv6subnetAddress  : in6_addr : The subnet address.
// + IPv6subnetPrefixLen: uunsigned int : The prefix length.
// + addressCounter     : int : The address counter.
// + globalAddr         : in6_addr* : The global IPv6 address.
// RETURN               :: void  :
// **/
void
MAPPING_CreateIpv6GlobalUnicastAddr(
    AddressMapType *map,
    in6_addr IPv6sunnetAddress,
    unsigned int IPv6subnetPrefixLen,
    int addressCounter,
    in6_addr* globalAddr);

// /**
// FUNCTION             :: MAPPING_CreateIpv6LinkLocalAddr
// PURPOSE              :: Create IPv6 link local Address.
// PARAMETERS           ::
// + globalAddr         : in6_addr* : The global IPv6 address.
// + linkLocalAddr      : in6_addr* : The subnet IPv6 address.
// + subnetPrefixLen    : unsigned int : The subnet prefix length.
// RETURN               :: void :
// **/
void
MAPPING_CreateIpv6LinkLocalAddr(
    in6_addr* globalAddr,
    in6_addr* linkLocalAddr,
    unsigned int subnetPrefixLen);

// /**
// FUNCTION             :: MAPPING_CreateIpv6SiteLocalAddr
// PURPOSE              :: Create IPv6 site local Address.
// PARAMETERS           ::
// + globalAddr         : in6_addr* : The global IPv6 address.
// + siteLocalAddr      : in6_addr* : The subnet IPv6 address.
// + siteCounter        : unsigned short : The counter to use.
// + subnetPrefixLen    : unsigned int : The subnet prefix length.
// RETURN               :: void :
// **/
void
MAPPING_CreateIpv6SiteLocalAddr(
    in6_addr* globalAddr,
    in6_addr* siteLocalAddr,
    unsigned short siteCounter,
    unsigned int subnetPrefixLen);

// /**
// FUNCTION             :: MAPPING_CreateIpv6MulticastAddr
// PURPOSE              :: Create ipv6 multicast address.
// PARAMETERS           ::
// + globalAddr         : in6_addr* : The global IPv6 address.
// + multicastAddr      : in6_addr* : The multicast IPv6 address.
// RETURN               :: void :
// **/
void
MAPPING_CreateIpv6MulticastAddr(
        in6_addr* globalAddr,
        in6_addr* multicastAddr);

// /**
// FUNCTION             :: MAPPING_CreateIpv6SubnetAddr
// PURPOSE              :: create subnet addr for IPV6 address.
// PARAMETERS           ::
// + tla                : unsigned int : Top level aggregation.
// + nla                : unsigned int : Next level aggregation.
// + sla                : unsigned int : Site level aggregation.
// + IPv6subnetPrefixLen: unsigned int* : The IPv6 prefix length.
// + IPv6subnetAddress  : in6_addr* : The IPv6 subnet address.
// RETURN               :: void :
// **/
void
MAPPING_CreateIpv6SubnetAddr(
    unsigned int tla,
    unsigned int nla,
    unsigned int sla,
    unsigned int* IPv6subnetPrefixLen,
    in6_addr* IPv6sunnetAddress);

// /**
// FUNCTION             :: MAPPING_GetNodeIdFromGlobalAddr
// PURPOSE              :: Get node id from Global Address.
// PARAMETERS           ::
// + node               : Node *: The node.
// + globalAddr         : in6_addr* : The global IPv6 address.
// RETURN               :: NodeId :
// **/
NodeId
MAPPING_GetNodeIdFromGlobalAddr(
    Node *node,
    in6_addr globalAddr);

// /**
// FUNCTION             :: MAPPING_GetNodeIdFromLinkLayerAddr
// PURPOSE              :: Get node id from Link layer Address.
// PARAMETERS           ::
// + node               : Node * : The node.
// + linkLayerAddr      : NodeAddress : The link layer address.
// RETURN               :: NodeId :
// **/
NodeId
MAPPING_GetNodeIdFromLinkLayerAddr(
    Node* node,
    NodeAddress linkLayerAddr);

// /**
// FUNCTION             :: MAPPING_CreateIpv6LinkLayerAddr
// PURPOSE              :: Create IPv6 link layer Address.
// PARAMETERS           ::
// + nodeId             : unsigned int : The node's id.
// + interfaceId        : int : The interface id.
// RETURN               :: NodeAddress :
// **/
NodeAddress
MAPPING_CreateIpv6LinkLayerAddr(
    unsigned int nodeId,
    int interfaceId);

// /**
// FUNCTION             :: MAPPING_IsIpv6AddressOfThisNode
// PURPOSE              :: checks whether the ipv6 address is of this node.
// PARAMETERS           ::
// + node               : Node* : The node id.
// + nodeId             : const NodeAddress : The node's address.
// + globalAddr         : in6_addr* : The global IPv6 address.
// RETURN               :: BOOL :
// **/
BOOL
MAPPING_IsIpv6AddressOfThisNode(
    Node *node,
    const NodeAddress nodeId,
    in6_addr *globalAddr);

// /**
// FUNCTION             : MAPPING_IsNodeInThisIpRange:
// PURPOSE              : checks whether the node is in given range of
//                      : Addresses.
// PARAMETERS           ::
// + node               : Node* : The node.
// + nodeId             : NodeId : The node id.
// + startRange         : NodeAddress : The starting address.
// + endRange           : NodeAddress : The end address.
// RETURN               :: BOOL :
// **/
BOOL
MAPPING_IsNodeInThisIpRange(
    Node *node,
    NodeId nodeId,
    NodeAddress startRange,
    NodeAddress endRange);

// /**
// FUNCTION             :: MAPPING_IsIpAddressOfThisNode
// PURPOSE              :: checks whether the ipv4 address is of this node.
// PARAMETERS           ::
// + node               : Node* : The node.
// + nodeId             : const NodeAddress : The node id.
// + addr               : NodeAddress : The address.
// RETURN               :: BOOL :
// **/
BOOL
MAPPING_IsIpAddressOfThisNode(
    Node *node,
    const NodeAddress nodeId,
    NodeAddress addr);

// /**
// FUNCTION             :: MAPPING_GetInterfaceAddressForSubnet
// PURPOSE              :: Get interface address for subnet using ipv6 addr.
// PARAMETERS           ::
// + node               : Node* : The node.
// + nodeId             : NodeId : The node id.
// + ipv6SubnetAddr     : in6_addr* : The subnet address.
// + prefixLenth        : unsigned int : The subnet prefix length.
// + ipv6InterfaceAddr  : in6_addr* : The ipv6 interface address.
// + interfaceIndex     : int* : The interface index.
// RETURN               :: BOOL :
// **/
BOOL
MAPPING_GetInterfaceAddressForSubnet(
    Node *node,
    NodeId nodeId,
    in6_addr* ipv6SubnetAddr,
    unsigned int prefixLenth,
    in6_addr* ipv6InterfaceAddr,
    int* interfaceIndex);

// /**
// FUNCTION             :: MAPPING_GetInterfaceAddressForSubnet
// PURPOSE              :: Get interface address for subnet using ipv6 addr.
// PARAMETERS           ::
// + map                : const AddressMapType* : The address map.
// + nodeId             : NodeId : The node id.
// + ipv6SubnetAddr     : in6_addr* : The subnet address.
// + prefixLenth        : unsigned int : The subnet prefix length.
// + ipv6InterfaceAddr  : in6_addr* : The ipv6 interface address.
// + interfaceIndex     : int* : The interface index.
// RETURN               : BOOL :
// **/
BOOL
MAPPING_GetInterfaceAddressForSubnet(
    const AddressMapType* map,
    NodeId nodeId,
    in6_addr* ipv6SubnetAddr,
    unsigned int prefixLenth,
    in6_addr* ipv6InterfaceAddr,
    int* interfaceIndex);

// /**
// FUNCTION             :: MAPPING_GetInterfaceAddressForSubnet
// PURPOSE              :: Get interface address for subnet using tla nla sla.
// PARAMETERS           ::
// + node               : Node* node : The node.
// + nodeId             : NodeId nodeId : The node id.
// + tla                : unsigned int : Top level aggregation.
// + nla                : unsigned int : Next level aggregation.
// + sla                : unsigned int : Site level aggregation.
// + ipv6Addr           : in6_addr* : The ipv6 interface address.
// + interfaceIndex     : int* : The interface index.
// RETURN               :: BOOL :
// **/
BOOL
MAPPING_GetInterfaceAddressForSubnet(
    Node *node,
    NodeId nodeId,
    unsigned int tla,
    unsigned int nla,
    unsigned int sla,
    in6_addr* ipv6Addr,
    int* interfaceIndex);

// /**
// FUNCTION             :; MAPPING_GetInterfaceAddressForSubnet
// PURPOSE              :: Get interface address for subnet using tla nla sla.
// PARAMETERS           ::
// + map                : const AddressMapType* : The address map.
// + nodeId             : NodeId : The node id.
// + tla                : unsigned int : Top level aggregation.
// + nla                : unsigned int : Next level aggregation.
// + sla                : unsigned int : Site level aggregation.
// + ipv6Addr           : in6_addr* : The ipv6 interface address.
// + interfaceIndex     : int* : The interface index.
// RETURN               :: BOOL :
// **/
BOOL
MAPPING_GetInterfaceAddressForSubnet(
    const AddressMapType* map,
    NodeId nodeId,
    unsigned int tla,
    unsigned int nla,
    unsigned int sla,
    in6_addr* ipv6Addr,
    int* interfaceIndex);

// /**
// FUNCTION             :: MAPPING_GetInterfaceFromLinkLayerAddress
// PURPOSE              :: Get interface from link layer address.
// PARAMETERS           ::
// + node               : Node* : The node.
// + linkLayerAddr      : const NodeAddress : The link layer address.
// RETURN               :: int :
// **/
int
MAPPING_GetInterfaceFromLinkLayerAddress(
    Node *node,
    const NodeAddress linkLayerAddr);

// /**
// FUNCTION             :: MAPPING_GetInterfaceIndexFromInterfaceAddress
// PURPOSE              :: Get interface index from interface address.
// PARAMETERS           ::
// + node               : Node* : The node.
// + interfaceAddress   : Address : The interface address.
// RETURN               :: int :
// **/
int
MAPPING_GetInterfaceIndexFromInterfaceAddress(
   Node *node,
   Address interfaceAddress);


// /**
// FUNCTION             : MAPPING_GetIpv6GlobalAddress:
// PURPOSE              : Get ipv6 global address :
// PARAMETERS           ::
// + node               : Node* : The node.
// + nodeId             : NodeId : The node id
// + subnetAddr         : in6_addr : The subnet address.
// + prefixLen          : UInt32 : The subnet prefix length.
// + addr6              : in6_addr* : The IPv6 address.
// RETURN               :: BOOL :
// **/
BOOL
MAPPING_GetIpv6GlobalAddress(
    Node *node,
    NodeId nodeId,
    in6_addr subnetAddr,
    UInt32 prefixLen,
    in6_addr *addr6);


// /**
// FUNCTION             : MAPPING_GetDefaultInterfaceAddressInfoFromNodeId:
// PURPOSE              :Get default interface address based on network type:
// PARAMETERS           ::
// + node               : Node *node: The node.
// + nodeId             : NodeAddress nodeId: The node id.
// + networktype        : NetworkType networktype: The network type.
// RETURN               : Address :
// **/
Address
MAPPING_GetDefaultInterfaceAddressInfoFromNodeId(
    Node *node,
    NodeAddress nodeId,
    NetworkType networktype = NETWORK_IPV4);

// /**
// NAME         :: MAPPING_SetAddress
// DESCRIPTION  :: Set a Generic address from different type of address.
//                 It may be IPv4, IPv6 or ATM address.
// PARAMETERS   ::
// + netType        : NetworkType : The network type.
// + destAddress    : Address* : The destination address.
// + srcAddress     : void* : The source address.
// RETURN       :: NULL :
// **/
void
MAPPING_SetAddress(
    NetworkType netType,
    Address* destAddress,
    void* srcAddress);

void
AddressMap_FindReverseMappingElements(
    const AddressMapType *map,
    const NodeAddress representativeAddress,
    int *numberFoundElements,
    int *indexFound);

void
AddressMap_UpdateIpAddress(
   AddressMapType *map,
   char *qualifier,
   int instanceId,
   char *value);

void
AddressMap_UpdateIpSubnetMask(
   AddressMapType *map,
   char *qualifier,
   int instanceId,
   char *value);

void
MAPPING_UpdateNumNodesInSubnet(
        Node *node,
        NodeAddress subnetAddress,
        int numNewNodes);

int
MAPPING_GetUsapSlot(
        Node *node,
        NodeAddress subnetAddress);

int
AddressMap_ConvertSubnetMaskToNumHostBits(NodeAddress subnetMask);

// /**
// FUNCTION             : Mapping_AutoCreateIPv6SubnetAddress
// PURPOSE              : Create IPv6 Testing Address Prefix (RFC 2471)from
//                      : ipv4 address.
// PARAMETERS           ::
// + ipAddress          : NodeAddress : The IPv4 address.
// + subnetString   :   : char* : string for IPv6 Testing Address Prefix:
// RETURN               : void : NONE
// **/
void Mapping_AutoCreateIPv6SubnetAddress(
    NodeAddress ipAddress,
    char* subnetString);

// /**
// FUNCTION     :: MAPPING_GetSubnetAddressFromInterfaceAddress
// PURPOSE      :: Get subnet address from interface address.
// PARAMETERS           ::
// + node               : Node *node : The node address.
// + interfaceAddress   : NodeAddress : The interface address.
// RETURN               : NodeAddress : subnet address
// **/
NodeAddress
MAPPING_GetSubnetAddressFromInterfaceAddress(
    Node *node,
    NodeAddress interfaceAddress);


// /**
// FUNCTION             :: MAPPING_GetSubnetAddressFromInterfaceAddress
// PURPOSE              :: Get ipv6 network Prefix from interface address.
// PARAMETERS           ::
// + node               : Node * : The node.
// + ipv6InterfaceAddr  : in6_addr* : The IPv6 interface address.
// + ipv6SubnetAddr     : in6_addr* : The subnet address pointer .
// RETURN               :: BOOL :
// **/
BOOL
MAPPING_GetSubnetAddressFromInterfaceAddress(
    Node *node,
    in6_addr* ipv6InterfaceAddr,
    in6_addr* ipv6subnetAddr);

// /**
// FUNCTION             :: MAPPING_GetPrefixLengthForInterfaceAddress
// PURPOSE              :: Get prefix length for interface address.
// PARAMETERS           ::
// + node               : Node* : The node.
// + ipv6InterfaceAddr  : in6_addr* : The IPV6 interface address.
// + prefixLenth        : unsigned int : The interface prefix length.
// RETURN               :: BOOL :
// **/
BOOL
MAPPING_GetPrefixLengthForInterfaceAddress(
    Node *node,
    in6_addr* ipv6InterfaceAddr,
    unsigned int* prefixLenth);

// /**
// FUNCTION             :: MAPPING_GetNetworkProtocolTypeForNode
// PURPOSE              :: Get Network Protocol Type for the node.
// PARAMETERS           ::
// + nodeId             : NodeAddress : The node id.
// + nodeInput          : const NodeInput * : The node input file
// RETURN               :: NetworkProtocolType :
// **/
NetworkProtocolType MAPPING_GetNetworkProtocolTypeForNode(
    Node* node,
    NodeAddress nodeId);

//---------------------------------------------------------------------------
// FUNCTION             : MAPPING_GetNetworkProtocolTypeForInterface:
// PURPOSE              : Get Network Protocol Type for a interface:
// PARAMETERS           ::
// + map                : const AddressMapType *map
// + nodeId             : NodeId nodeId:
// + interfaceIndex     : int interfaceIndex:
// RETURN               : NetworkProtocolType:
//---------------------------------------------------------------------------
NetworkProtocolType
MAPPING_GetNetworkProtocolTypeForInterface(
    const AddressMapType *map,
    NodeId nodeId,
    int interfaceIndex);

#endif /* MAPPING_H */

