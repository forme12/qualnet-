/*  Copyright (c) January 2005 Jean Gressmann (jsg@rz.uni-potsdam.de)

 *

 *  This is free software; you can redistribute it and/or modify

 *  it under the terms of the GNU General Public License as published by

 *  the Free Software Foundation; either version 2 of the License, or

 *  (at your option) any later version. 

 * 

 *  This file is distributed in the hope that it will be useful,

 *  but WITHOUT ANY WARRANTY; without even the implied warranty of

 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the

 *  GNU General Public License for more details.

 *

 *  You should have received a copy of the GNU General Public License

 *  along with this file; if not, write to the Free Software

 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */



#ifndef ARCH_COMMON_H

#define ARCH_COMMON_H



#include <iostream>



#define PT_ARCH_COMMON_INCLUDED



namespace PortableThreads 

{

    // Interger type definitions based on size

    namespace Private

    {

        template<unsigned>

        struct PTSizeToType;



        template<>

        struct PTSizeToType<1>

        {

            typedef int8 int_type;

            typedef uint8 uint_type;

        };



        template<>

        struct PTSizeToType<2>

        {

            typedef int16 int_type;

            typedef uint16 uint_type;

        };



        template<>

        struct PTSizeToType<4>

        {

            typedef int32 int_type;

            typedef uint32 uint_type;

        };



        template<>

        struct PTSizeToType<8>

        {

            typedef int64 int_type;

            typedef uint64 uint_type;

        };



        template<typename T>

        struct PTSign;



        template<>

        struct PTSign<int8>

        {

            enum {sign = 1};

        };

        template<>

        struct PTSign<int16>

        {

            enum {sign = 1};

        };

        template<>

        struct PTSign<int32>

        {

            enum {sign = 1};

        };

        template<>

        struct PTSign<int64>

        {

            enum {sign = 1};

        };

        template<>

        struct PTSign<uint8>

        {

            enum {sign = 0};

        };

        template<>

        struct PTSign<uint16>

        {

            enum {sign = 0};

        };

        template<>

        struct PTSign<uint32>

        {

            enum {sign = 0};

        };

        template<>

        struct PTSign<uint64>

        {

            enum {sign = 0};

        };



        template<typename T>

        struct PTCompilerHelper

        {

            typedef typename PTSizeToType<sizeof(T)>::uint_type uint_type;

            typedef typename PTSizeToType<sizeof(T)>::int_type int_type;

        };

    }



    typedef Private::PTSizeToType<sizeof(void*)>::int_type pt_pointer_type;

    typedef Private::PTSizeToType<sizeof(long)>::int_type pt_int_type;

    typedef Private::PTSizeToType<sizeof(unsigned long)>::uint_type pt_uint_type;



    namespace LockFree

    {

        namespace Private

        {

            template<bool HAS_COUNTER, bool HAS_USER_BITS>

            class token_t

            {

            public: 

                typedef pt_pointer_type int_t;

                explicit

                token_t(int_t pointer = 0, int_t count = 0, int_t userbits = 0)

                    :   pointer_(pointer)

                    ,   count_(count)

                    ,   userbits_(userbits)

                {}

                inline int_t pointer() const { return pointer_; }

                inline void pointer(int_t v) { pointer_ = v; }

                inline int_t count() const { return count_; }

                inline void count(int_t c) { count_ = c; }

                inline int_t userbits() const { return userbits_; }

                inline void userbits(int_t c) { userbits_ = c; }

            private:

                int_t pointer_, count_, userbits_;

            };



            template<>

            class token_t<false, true>

            {

            public: 

                typedef pt_pointer_type int_t;

                explicit

                token_t(int_t pointer = 0, int_t count = 0, int_t userbits = 0)

                    :   pointer_(pointer)

                    ,   userbits_(userbits)

                {}

                inline int_t pointer() const { return pointer_; }

                inline void pointer(int_t v) { pointer_ = v; }

                inline int_t count() const { return 0; }

                inline void count(int_t c) {}

                inline int_t userbits() const { return userbits_; }

                inline void userbits(int_t c) { userbits_ = c; }

            private:

                int_t pointer_, userbits_;

            };



            template<>

            class token_t<true, false>

            {

            public: 

                typedef pt_pointer_type int_t;

                explicit

                token_t(int_t pointer = 0, int_t count = 0, int_t userbits = 0)

                    :   pointer_(pointer)

                    ,   count_(count)

                {}

                inline int_t pointer() const { return pointer_; }

                inline void pointer(int_t v) { pointer_ = v; }

                inline int_t count() const { return count_; }

                inline void count(int_t c) { count_ = c; }

                inline int_t userbits() const { return 0; }

                inline void userbits(int_t c) {}

            private:

                int_t pointer_, count_;

            };



            template<>

            class token_t<false, false>

            {

            public: 

                typedef pt_pointer_type int_t;

                explicit

                token_t(int_t pointer = 0, int_t count = 0, int_t userbits = 0)

                    :   pointer_(pointer)

                {}

                inline int_t pointer() const { return pointer_; }

                inline void pointer(int_t v) { pointer_ = v; }

                inline int_t count() const { return 0; }

                inline void count(int_t c) {}

                inline int_t userbits() const { return 0; }

                inline void userbits(int_t c) {}

            private:

                int_t pointer_;

            };

            

            template<bool HAS_COUNTER, bool HAS_USER_BITS>

            inline bool operator==( const token_t<HAS_COUNTER, HAS_USER_BITS>& lhs, 

                                    const token_t<HAS_COUNTER, HAS_USER_BITS>& rhs)

            {

                return  lhs.pointer() == rhs.pointer() && 

                        lhs.count() == rhs.count() && 

                        lhs.userbits() == rhs.userbits();

            }



            template<bool HAS_COUNTER, bool HAS_USER_BITS>

            inline bool operator!=( const token_t<HAS_COUNTER, HAS_USER_BITS>& lhs, 

                                    const token_t<HAS_COUNTER, HAS_USER_BITS>& rhs)

            {

                return !(lhs == rhs);

            }

        }

    }

}







#endif

