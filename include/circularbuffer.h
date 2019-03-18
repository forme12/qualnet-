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

#ifndef CIRBUF_H
#define CIRBUF_H

// /**
// PACKAGE     :: CIRCULAR-BUFFER
// DESCRIPTION :: This file describes data structures and functions used for circular buffer implementation.
// **/

// /**
// CONSTANT    :: CIR_BUF_SIZE                 :   256
// DESCRIPTION :: Default Circular Buffer Size
// **/
#define  CIR_BUF_SIZE           256

// /**
// ENUM        ::
// DESCRIPTION :: Type of Circular Buffer Operation
// **/
enum
{
    idRead,
    idWrite
};

// /**
// CLASS       :: CircularBuffer
// DESCRIPTION :: Circular Buffer implementation
// **/
class CircularBuffer
{

private:
    unsigned short      m_index;      // To uniquely identify the Cir.Buffer
    Int32               m_read;       // start position to read from queue
    Int32               m_write;      // start position to write into queue
    Int32               m_queSz;      // size of queue
    Int32               m_wrapType;   // stores wrap type
    unsigned char*      m_que;        // stores recieved data

protected:
    // /**
    // ENUM        ::
    // DESCRIPTION :: Type of Wrap operation
    // **/
    enum
    {
        idNoWrap,
        idReadWrap,
        idWriteWrap
    };

    // /**
    // API        :: CircularBuffer.incPos
    // PURPOSE    :: increment read/write position based on operation
    // PARAMETERS ::
    // + increment : Int32: How much will be incremented
    // + operation : Int32: Type of Operation (Read or Write )
    // RETURN     :: bool : Successful or not
    // **/
    bool incPos(Int32 inc, Int32 operation);

public:
    // /**
    // API        :: CircularBuffer.CircularBuffer
    // PURPOSE    :: Constructor
    // PARAMETERS ::
    // RETURN     :: None : 
    // **/
    CircularBuffer();

    // /**
    // API        :: CircularBuffer.CircularBuffer
    // PURPOSE    :: Constructor
    // PARAMETERS ::
    // + queueSize : Int32: Size of the Queue
    // RETURN     :: None :
    // **/
    CircularBuffer(Int32 queueSize);

    // /**
    // API        :: CircularBuffer.CircularBuffer
    // PURPOSE    :: Constructor
    // PARAMETERS ::
    // + index    :  unsigned short: Circular Buffer Index
    // RETURN     :: None :
    // **/
    CircularBuffer(unsigned short index);

    // /**
    // API        :: CircularBuffer.CircularBuffer
    // PURPOSE    :: Constructor
    // PARAMETERS ::
    // + index     :  unsigned short: Circular Buffer Index
    // + queueSize :  Int32: Size of the queue
    // RETURN     :: None :
    // **/
    CircularBuffer(unsigned short index,Int32 queueSize);

    // /**
    // API        :: CircularBuffer.~CircularBuffer
    // PURPOSE    :: Destructor
    // PARAMETERS ::
    // RETURN     :: Node :
    // **/
    ~CircularBuffer();

    // /**
    // API        :: CircularBuffer.create
    // PURPOSE    :: Memory allocation for Circular Buffer
    // PARAMETERS ::
    // + queueSize : Int32: Size of queue
    // RETURN     :: bool : Successful or not
    // **/
    bool create(Int32 queueSize);

    // /**
    // API        :: CircularBuffer.release
    // PURPOSE    :: To free the allocated memory
    // PARAMETERS :: 
    // + : void : None
    // RETURN     :: void : None
    // **/
    void release();

    // /**
    // API        :: CircularBuffer.reset
    // PURPOSE    :: reset position and wrap values
    // PARAMETERS ::
    // PARAMETERS :: 
    // + : void : None
    // RETURN     :: void : None
    // **/
    void reset();

    // /**
    // API        :: CircularBuffer.getCount
    // PURPOSE    :: gets the number of bytes to read
    // PARAMETERS ::
    // + count : Int32&: the parameter to be filled up
    // + operation : Int32: Type of Operation (Read or Write)
    // RETURN     :: bool : successful or not
    // **/
    bool getCount(Int32& count, Int32 operation);

    // /**
    // API        :: CircularBuffer.lengthToEnd
    // PURPOSE    :: get the circular buffer's allocated size
    // PARAMETERS ::
    // + operation : Int32: Read or Write Operation
    // RETURN     :: Int32 : Total length of data to be read
    // **/
    Int32 lengthToEnd(Int32 op = 0);

    // /**
    // API        :: CircularBuffer.readWithCount
    // PURPOSE    :: Read data from Buffer and pass the length of data read
    // PARAMETERS ::
    // + data : unsigned char*: Container to which data will be read
    // + length : Int32&: length of data read
    // RETURN     :: bool :  Successful or not
    // **/
    bool readWithCount(unsigned char* data, Int32 & length);

    // /**
    // API        :: CircularBuffer.readFromBuffer
    // PURPOSE    :: Reading the required no. of bytes from the circular buffer
    // PARAMETERS ::
    // + data   : unsigned char* : Container to which data will be read
    // + length : Int32 : length of data to be read
    // + noIncrement : bool : Whether the read pointer is to be incremented or not
    // RETURN     :: bool : successful or not
    // **/
    bool readFromBuffer(unsigned char* data,
                        Int32 length,
                        bool noIncrement = true);

    // /**
    // API        :: CircularBuffer.write
    // PURPOSE    :: Write to the circular buffer
    // PARAMETERS ::
    // + data   : unsigned char*: Container to which data will be written
    // + length : Int32 : Length of data to be written
    // RETURN     :: bool : successful or not
    // **/
    bool write(unsigned char* data, Int32 length);

    // /**
    // API        :: CircularBuffer.read
    // PURPOSE    :: To Read data from Buffer
    // PARAMETERS ::
    // + buffer : unsigned char*: Container to which data will be read
    // RETURN     :: bool : Succesful or not
    // **/
    bool read(unsigned char* buffer);

    // /**
    // API        :: CircularBuffer.getIndex
    // PURPOSE    :: get the circular buffer's allocated size
    // PARAMETERS ::
    // + operation : Int32: Read or Write Operation
    // RETURN     :: Int32 : Current operation Position
    // **/
    Int32 getIndex(Int32 operation);

    // /**
    // API        :: CircularBuffer.getCirBufSize
    // PURPOSE    :: get the circular buffer's allocated size
    // PARAMETERS :: 
    // + none : void : None
    // RETURN     :: Int32 : circular buffer's allocated size
    // **/
    Int32 getCirBufSize();

    // /**
    // API        :: CircularBuffer.getIndex
    // PURPOSE    :: get the circular buffer's unique index
    // PARAMETERS :: 
    // + none : void : None
    // RETURN     :: unsigned short : unique index
    // **/
    unsigned short getIndex()
    {
        return m_index;
    }
};


#endif /* CIRBUF_H */
