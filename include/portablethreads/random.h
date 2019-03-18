/*  Copyright (c) October 2005 Jean Gressmann (jsg@rz.uni-potsdam.de)
 *
 *  This is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version. 
 * 
 *	This file is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this file; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef PT_RANDOM_H
#define PT_RANDOM_H
#include <vector>
#include <portablethreads/config.h>

namespace PortableThreads 
{
	/*! \class PTRandom random.h portablethreads/random.h
		\brief Random number generator.

		Mersenne Twister 32 bit (pseudo) random numbers, due to: http://www.math.keio.ac.jp/matumoto/emt.html
		
		This class aims to replace the standard std::rand() and std::srand() functions in a thread-safe
		way. While concurrent access to one object still needs to be synchronized, different
		threads may each utilize an different PTRandom object without interfering with each other.
	*/
	class PTRandom
	{
	public:
		~PTRandom();
		/*! \brief Constructs a random number generator.

			\param seed Use this value to seed the generator.
		*/
		PTRandom(unsigned long seed = 5489UL);
		/*! \brief Reseeds the generator.

			\param seed Value used to reseed the generator.
		*/
		void seed(unsigned long seed = 5489UL);
		/*! \brief Get a random number.
				
			The numbers returned are evenly distributed
			among the range [-max(long), max(long)].
		*/
		long rand();
		/*! \brief Get a random number.
				
			The numbers returned are evenly distributed
			among the range [0, max(unsigned long)].
		*/
		unsigned long urand();
	private:
		PTRandom(const PTRandom&);
		PTRandom& operator=(const PTRandom&);
	private:
		std::vector<unsigned long> mt;
		unsigned long mti;
	private:
		static const unsigned long N, M, MATRIX_A, UPPER_MASK, LOWER_MASK;
	};
}

#endif
