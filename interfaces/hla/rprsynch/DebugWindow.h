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

#ifndef _DEBUG_WINDOW_H_
#define _DEBUG_WINDOW_H_

#include <QApplication>
#include <QTextBrowser>
#include <iostream>
#include <QMutexLocker>

class DebugWindow : public QTextBrowser
{
    Q_OBJECT

    private:
        class streamBuffer : public std::streambuf
        {
            private:
                QMutex mutex;
                DebugWindow *theDebugWindow;
                char_type buffer[4096];
                streamBuffer(const streamBuffer& sb);

            public:
                streamBuffer(DebugWindow *owner) :
                    mutex(QMutex::Recursive), theDebugWindow(owner)
                {
                    setp(buffer, buffer+sizeof(buffer)-2);
                }
                virtual std::streamsize xsputn(const char_type *_Ptr, std::streamsize _Count)
                {
                    QMutexLocker locker(&mutex);
                    int num = pptr()-pbase();
                    if( num < 0 )
                        setp(buffer, buffer+sizeof(buffer)-2);
                    return std::streambuf::xsputn(_Ptr,_Count);
                }
                virtual int overflow(int ch)
                {
                    QMutexLocker locker(&mutex);
                    if( ch != traits_type::eof() )
                    {
                        *pptr() = ch;
                        pbump(1);
                    }
                    return sync();
                }
                virtual int sync()
                {
                    QMutexLocker locker(&mutex);
                    char* start = pbase();
                    int num = pptr()-pbase();
                    if( !theDebugWindow )
                    {
                        setp(buffer, buffer+sizeof(buffer)-2);
                        return 0;
                    }

                    std::string str(start, num);
                    theDebugWindow->appendPlainText(str.c_str());

                    setp(buffer, buffer+sizeof(buffer)-2);
                    return 0;
                }
        };

        std::map<std::ostream*, streamBuffer*> grabbedStreams;
        std::map<std::ostream*, std::streambuf*> oldStreams;
    public:
        DebugWindow()
        {
            connect(this, SIGNAL(callAppendPlainText(QString)),
                this, SLOT(doAppendPlainText(QString)));
        }
        ~DebugWindow()
        {
            std::map<std::ostream*, std::streambuf*>::iterator old;
            for( old = oldStreams.begin(); old != oldStreams.end(); old++ )
            {
                (old->first)->rdbuf(old->second);
            }
            oldStreams.clear();
        }
        void grabStream(std::ostream &out)
        {
            std::map<std::ostream*, streamBuffer*>::iterator it;
            it = grabbedStreams.find(&out);
            if( it != grabbedStreams.end() )
                return;

            oldStreams[&out] = out.rdbuf();
            streamBuffer *newBuffer = new streamBuffer(this);
            out.rdbuf(newBuffer);
            grabbedStreams[&out] = newBuffer;
        }
        void restoreStream(std::ostream &out)
        {
            std::map<std::ostream*, std::streambuf*>::iterator old;
            old = oldStreams.find(&out);
            if( old == oldStreams.end() )
                return;
            out.rdbuf(old->second);
            oldStreams.erase(old);

            std::map<std::ostream*, streamBuffer*>::iterator current;
            current = grabbedStreams.find(&out);
            if( current == grabbedStreams.end() )
                return;
            streamBuffer *buf = current->second;
            grabbedStreams.erase(current);
            delete buf;
        }
        void appendPlainText(std::string str)
        {
            QString qstr(str.c_str());
            emit callAppendPlainText(qstr);
            QApplication::processEvents();
        }
    private slots:
        void doAppendPlainText(QString str)
        {
            QTextCursor cursor = textCursor();
            cursor.movePosition(QTextCursor::End);
            setTextCursor(cursor);
            insertPlainText(str);
        }
    signals:
        void callAppendPlainText(QString str);

};

#endif

