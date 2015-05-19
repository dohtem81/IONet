/*
 *   This file is part of IONet.
 *
 *   IONet is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   any later version.
 *
 *   RoverPlatform is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with IONet.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   For any comment or suggestion please contact the author of that software
 *   at pedziwiatr.piotr@gmail.com
 */

#ifndef IONETVAR_H_
#define IONETVAR_H_

#include <string>

namespace ION {

// enumerations
// -------------------------------------------------------------------------------------------------------------------------------------------
enum varType { 	integer = 0,
				boolean = 1,
				real = 2 };

// IONetVar class is single variable class
// whole page contains multiple instances of that class
// key components are:
//		1. pointer to char - this is pointer where data is located in the page
//			and is assigned during class construction
//		2. size - based on type, size in bytes, starting at location pointed by
//			pointer to char
//		3. enum type - what type of data, used to determine size
// -------------------------------------------------------------------------------------------------------------------------------------------
class IONetVar {
public:
	IONetVar();

	// main constructor
	IONetVar(char*, varType, std::string, int =0);
	virtual ~IONetVar();

	// returns name of the variable
	std::string getName(){	return this->name ;	}

	// returns size in bytes of the variable
	int getSize(){	return this->size ;	}

private:
	int 			size ;
	int 			offset ;
	char 			*value ;
	std::string		name ;
	varType 		dataType ;

};

} /* namespace ION */
#endif /* IONETVAR_H_ */