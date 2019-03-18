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



#ifndef NATIVE_POINTER_CAS_H

#define NATIVE_POINTER_CAS_H



#ifndef PT_ARCH_COMMON_INCLUDED

#   error "Architectural types and functions must be included first!"

#endif 



#define PT_POINTER_CAS_INCLUDED



namespace PortableThreads 

{



    namespace LockFree

    {

        namespace Private

        {

            template<class Traits>

            class PointerCAS

            {

            public:

                typedef typename Traits::token_t token_t;

                typedef typename Traits::int_t large_int_t;

                typedef pt_pointer_type int_t;

            public:

                explicit

                PointerCAS(int_t p = 0, int_t u = 0)

                    :   value_(0)

                {

                    assign(p, u);

                }

                /*

                inline bool cas(int_t nv, const token_t& ov)

                {

                    return pt_atomic_cas(

                        &value_,

                        Traits::multiplex(nv, ov.count()+1, ov.userbits()),

                        Traits::multiplex(ov.pointer(), ov.count(), ov.userbits()));

                }

                */

                inline bool cas(token_t& nv, const token_t& ov)

                {

                    const large_int_t mux = Traits::multiplex(nv.pointer(), ov.count()+1, nv.userbits());

                    if(pt_atomic_cas(

                        &value_,

                        mux,

                        Traits::multiplex(ov.pointer(), ov.count(), ov.userbits())))

                    {

                        nv.count(static_cast<int_t>(Traits::count(mux)));

                        return true;

                    }

                    return false;

                }

                inline token_t get() const

                {

                    const large_int_t v = value_;

                    return token_t(

                        static_cast<int_t>(Traits::pointer(v)), 

                        static_cast<int_t>(Traits::count(v)),

                        static_cast<int_t>(Traits::userbits(v)));

                }

                inline void assign(int_t p, int_t u)

                {

                    value_ = Traits::multiplex(p, Traits::count(value_), u);

                }

                inline void assign(const token_t& t)

                {

                    assign(t.pointer(), t.userbits());

                }

            private:

                PointerCAS(const PointerCAS&);

                PointerCAS operator=(const PointerCAS&);

            private:

                volatile large_int_t value_;

            };

        }

    }

}



#endif

