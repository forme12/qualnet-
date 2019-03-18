#include "api.h"
#include "partition.h"
#include "mac_csma.h"
#include "mac_dot11.h"
#include "mac_tdma.h"
#include "platform.h"
#include "interfaceutil.h"
#ifdef JNE_LIB
#include "mac_wnw_main.h"
#endif

BOOL IsMyMac(
    Node *node,
    Message *msg)
{
    char str[MAX_STRING_LENGTH];

    // Check the MAC header to make sure it is being sent to this node
    switch (msg->headerProtocols[msg->numberOfHeaders - 1])
    {
    case TRACE_CSMA:
    {
        CsmaHeader *csma = (CsmaHeader*) MESSAGE_ReturnPacket(msg);

        MacHWAddress  destHWAddress;

        Convert802AddressToVariableHWAddress(
            node,
            &destHWAddress,
            &csma->destAddr);

        // Ignore frame if it is not sent to us
        if (!MAC_IsMyAddress (node, &destHWAddress))
        {
            return FALSE;
        }

        return TRUE;
    }

    case TRACE_DOT11:
    {
        DOT11_ShortControlFrame *hdr = (DOT11_ShortControlFrame*)
                                       MESSAGE_ReturnPacket(msg);
        MacHWAddress  destHWAddress;

        Convert802AddressToVariableHWAddress(
            node,
            &destHWAddress,
            &hdr->destAddr);

        // Ignore frame if it is not sent to us
        if (!MAC_IsMyAddress (node, &destHWAddress))
        {
            return FALSE;
        }

        return TRUE;
    }

    case TRACE_USAP:
    {
        return TRUE;
    }

    case TRACE_USAP_SLOT:
    {
        return TRUE;
    }
    case TRACE_USAP_CELL:
    {
        return TRUE;

    }
    case TRACE_TDMA:
    {
        TdmaHeader *hdr = (TdmaHeader*) MESSAGE_ReturnPacket(msg);

        NodeAddress ipv4DestAddr =
            DefaultMac802AddressToIpv4Address(node, &hdr->destAddr);

        // Ignore frame if broadcast
        if (ipv4DestAddr == ANY_DEST)
        {
            return FALSE;
        }

        // Ignore frame if it is not sent to us
        if (!MAC_IsMyUnicastFrame(node, ipv4DestAddr))
        {
            return FALSE;
        }

        return TRUE;
    }

    default:
        sprintf(str, "Unknown MAC protocol %d",
                msg->headerProtocols[msg->numberOfHeaders-1]);
        ERROR_ReportWarning(str);
        return FALSE;
        break;
    }
}

#define PIvalue ((double)3.14159265358979)
#define pio2    ((double)0.5*PIvalue)
#define twopi   ((double)2.0*PIvalue)
#define rad_to_deg      ((double)360.0/twopi)


void xyz2plh( double *xyz, double *plh, double A, double FL )
/********1*********2*********3*********4*********5*********6*********7**
 * Name:        xyz2plh
 * Version:     9602.17
 * Author:      B. Archinal (USNO)
 * Purpose:     Converts XYZ geocentric coordinates to Phi (latitude),
 *              Lambda (longitude), H (height) referred to an
 *              ellipsoid of semi-major axis A and flattening FL.
 *
 * Input:
 * -----------
 * A                semi-major axis of ellipsoid [units are of distance]
 * FL               flattening of ellipsoid [unitless]
 * xyz[]            geocentric Cartesian coordinates [units are of distance]
 *
 * Output:
 * -----------
 * plh[]            ellipsoidal coordinates of point, in geodetic latitude,
 *                  longitude east of Greenwich, and height [units for
 *                  latitude=plh[0] and longitude=plh[1] are in degrees;
 *                  height=plh[2] are distance and will be the same as
 *                  those of the input parameters]
 *
 * Local:
 * -----------
 * B                semi-minor axis of ellipsoid [same units as A]
 *
 * Global:
 * -----------
 *
 * Notes:
 * -----------
 * This routine will fail for points on the Z axis, i.e. if X= Y= 0
 * (Phi = +/- 90 degrees).
 *
 * Units of input parameters `A' and `xyz' must be the same.
 *
 * References:
 * -----------
 * Borkowski, K. M. (1989).  "Accurate algorithms to transform geocentric
 * to geodetic coordinates", *Bulletin Geodesique*, v. 63, pp. 50-56.
 *
 * Borkowski, K. M. (1987).  "Transformation of geocentric to geodetic
 * coordinates without approximations", *Astrophysics and Space Science*,
 * v. 139, n. 1, pp. 1-4.  Correction in (1988), v. 146, n. 1, p. 201.
 *
 * An equivalent formulation is recommended in the IERS Standards
 * (1995), draft.
 *
 ********1*********2*********3*********4*********5*********6*********7**
 * Modification History:
 * 9007.20, BA,  Creation
 * 9507,21, JR,  Modified for use with the page programs
 * 9602.17, MSS, Converted to C.
 ********1*********2*********3*********4*********5*********6*********7*/
{
    double B;
    double d;
    double e;
    double f;
    double g;
    double p;
    double q;
    double r;
    double t;
    double v;
    double x= xyz[0];
    double y= xyz[1];
    double z= xyz[2];
    double zlong;
    /*
     *   1.0 compute semi-minor axis and set sign to that of z in order
     *       to get sign of Phi correct
     */
    B= A * (1.0 - FL);
    if (z < 0.0)
        B= -B;
    /*
     *   2.0 compute intermediate values for latitude
     */
    r= sqrt( x*x + y*y );
    e= ( B*z - (A*A - B*B) ) / ( A*r );
    f= ( B*z + (A*A - B*B) ) / ( A*r );
    /*
     *   3.0 find solution to:
     *       t^4 + 2*E*t^3 + 2*F*t - 1 = 0
     */
    p= (4.0 / 3.0) * (e*f + 1.0);
    q= 2.0 * (e*e - f*f);
    d= p*p*p + q*q;

    if (d >= 0.0) {
        v= pow( (sqrt( d ) - q), (1.0 / 3.0) )
           - pow( (sqrt( d ) + q), (1.0 / 3.0) );
    } else {
        v= 2.0 * sqrt( -p )
           * cos( acos( q/(p * sqrt( -p )) ) / 3.0 );
    }
    /*
     *   4.0 improve v
     *       NOTE: not really necessary unless point is near pole
     */
    if (v*v < fabs(p)) {
        v= -(v*v*v + 2.0*q) / (3.0*p);
    }
    g= (sqrt( e*e + v ) + e) / 2.0;
    t = sqrt( g*g  + (f - v*g)/(2.0*g - e) ) - g;

    plh[0] = atan( (A*(1.0 - t*t)) / (2.0*B*t) );
    /*
     *   5.0 compute height above ellipsoid
     */
    plh[2]= (r - A*t)*cos( plh[0] ) + (z - B)*sin( plh[0] );
    /*
     *   6.0 compute longitude east of Greenwich
     */
    zlong = atan2( y, x );
    if (zlong < 0.0)
        zlong= zlong + twopi;

    plh[1]= zlong;
    /*
     *   7.0 convert latitude and longitude to degrees
     */
    plh[0] = plh[0] * rad_to_deg;
    plh[1] = plh[1] * rad_to_deg;

    return;
}

void ConvertGCCCartesianToGCC(
    double x,
    double y,
    double z,
    double *lat,
    double *lon,
    double *alt)
{
    double a = 6378137.0; // WGS84
    double f = 1/298.257223563; // WGS84

    double xyz[3] = {x, y, z};

    double plh[3];

    xyz2plh(xyz, plh, a, f );

    *lat = plh[0];
    *lon = plh[1];
    *alt = plh[2];
}


void ConvertGCCCartesianVelocityToGCC(
    double xpos,
    double ypos,
    double zpos,
    double xvel,
    double yvel,
    double zvel,
    double *latvel,
    double *lonvel,
    double *altvel)
{
    //plh constants
    double a = 6378137.0;   //WGS84
    double f = 1/298.257223563; //WGS84

    //convert the position to latlonalt
    double xyzpos[3] = {xpos, ypos, zpos};
    double plh[3];
    xyz2plh(xyzpos, plh, a, f);

    //convert the position + delta movement to latlonalt
    double xyzvel[3] = {xpos + xvel, ypos + yvel, zpos + zvel};
    double plhvel[3];
    xyz2plh(xyzvel, plhvel, a, f);

    //return ((pos+delta) - pos)
    *latvel = plhvel[0] - plh[0];
    *lonvel = plhvel[1] - plh[1];
    *altvel = plhvel[2] - plh[2];

    //check bounds for circumventing the earth
    if (*latvel > 90)
    {
        *latvel = *latvel - 180;
    }
    if (*latvel < -90)
    {
        *latvel = *latvel + 180;
    }
    if (*lonvel > 180)
    {
        *lonvel = *lonvel - 360;
    }
    if (*lonvel < -180)
    {
        *lonvel = *lonvel + 360;
    }
}


void CreateNewConnection(
    EXTERNAL_Interface* iface,
    SocketInterface_Sockets* sockets,
    int portIndex,
    char* outputString)
{
    EXTERNAL_SocketErrorType err;
    int i;
    EXTERNAL_Socket* s;

    assert(iface != NULL);

    outputString[0] = 0;

    pthread_mutex_lock(&sockets->socketMutex);

    // Initialize blocking and threaded
    s = new EXTERNAL_Socket;
    EXTERNAL_SocketInit(s, TRUE, TRUE);

    // Add to arrays, currently socket is inactive
    i = sockets->connections.size();
    sockets->activeConnections.push_back(FALSE);
    sockets->connections.push_back(s);

    // Accept the connection
    err = EXTERNAL_SocketAccept(
              sockets->listeningSockets[portIndex],
              sockets->connections[i]);
    if (err == EXTERNAL_NoSocketError)
    {
        sprintf(
            outputString,
            "Socket connection accepted from MTS on port %d.  "
            "Descriptor Assigned - %d (%d total connections)\n",
            sockets->listenPorts[portIndex],
            sockets->connections[i]->socketFd,
            sockets->numConnections + 1);

        sockets->activeConnections[i] = TRUE;
        sockets->numConnections++;
    }
    else
    {
        ERROR_ReportWarning("Could not create new connection\n");
    }

    pthread_mutex_unlock(&sockets->socketMutex);
}

void HandleSocketError(
    EXTERNAL_Interface *iface,
    SocketInterface_Sockets *sockets,
    int socketId,
    EXTERNAL_SocketErrorType err,
    char* outputString)
{
    char errString[MAX_STRING_LENGTH];
    EXTERNAL_Socket* s;

    assert(iface != NULL);

    outputString[0] = 0;

    assert(0 <= socketId && socketId < sockets->connections.size());
    s = sockets->connections[socketId];

    pthread_mutex_lock(&sockets->socketMutex);

    if (err == EXTERNAL_SocketError || err == EXTERNAL_InvalidSocket)
    {
        // A miscellaneous socket error.  Close the socket connection.
        // Qualnet will attempt to re-establish the connection in the next
        // call to SocketInterface_Receive.

        EXTERNAL_SocketClose(s);
        sockets->numConnections--;
        printf("Client closed connection...\n");

        // If the socket id was specified (this is a data socket not a
        // listening socket) then set it as inactive
        if (socketId != -1)
        {
            sockets->activeConnections[socketId] = FALSE;
        }

        // Join on the receiver thread if threaded
        if (s->threaded)
        {
            pthread_join(s->reciever, NULL);
        }

        // Clear error
        s->error = FALSE;

        sprintf(outputString, "incoming_run ( ) - Client closed connection.\n");
    }
    else
    {
        sprintf(errString, "Unknown socket error \"%d\"", err);
        ERROR_ReportError(errString);
    }

    pthread_mutex_unlock(&sockets->socketMutex);
}

