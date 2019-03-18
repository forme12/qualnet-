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
 
#ifndef PT_EXCEPTION_H
#define PT_EXCEPTION_H
#include <string>
#include <stdexcept>
#include <portablethreads/config.h>

namespace PortableThreads 
{
	/*! \class PTException exception.h portablethreads/exception.h
		\brief Base class for all exceptions throw from within PortableThreads code.

		All exceptions used in PortableThreads code inherit in some way
		or another from std::exception.
	*/
	class PTException : public std::runtime_error
	{
	public:
		/*! \brief Create an exception object

			The base class object of type std::runtime_error
			is initialized with the parameter reason.

			\param reason A message describing the reason for the exception-
		*/
		PTException(const std::string& reason = "");
		/*! \brief Throws a copy of this exception object.
			
			This method should be overwritten in derived
			exception classes to throw an exception of the
			proper type.
		*/
		virtual void raise();
		/*! \brief Creates a heap-allocated copy of this exception object

			This method should be overwritten in derived
			exception classes to create a copy of the proper type.
		*/
		virtual PTException* clone() const;
	};

	/*! \class PTParameterError exception.h portablethreads/exception.h
		\brief Indicates that some parameter or some combination of parameters is invalid.
	*/
	class PTParameterError : public PTException
	{
	public:
		PTParameterError(const std::string& reason = "Invalid parameter");
		virtual void raise();
		virtual PTException* clone() const;
	};
	
	/*! \class PTResourceError exception.h portablethreads/exception.h
		\brief Indicates that an operation system resource could not be allocated.

		All types in PortableThreads reserve the right to throw this exception
		from any method except those methods that 
		- are destructors,
		- have an exception specification which prohibits it.

	*/
	class PTResourceError : public PTException
	{
	public:
		PTResourceError(const std::string& reason = "OS resources exhausted");
		virtual void raise();
		virtual PTException* clone() const;
	};
}

#endif
