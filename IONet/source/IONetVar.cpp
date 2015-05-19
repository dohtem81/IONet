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

#include "IONetVar.h"

namespace ION {

// this constructor never used
// -------------------------------------------------------------------------------------------------------------------------------------------
IONetVar::IONetVar() {
	// TODO Auto-generated constructor stub

}


// constructor
// will create insatnce of IONet variable class assigning pointer to its data,
// setting up size and offset. memory must be already allocated
// params:
//		char* var - pointer to data
//		varType type - enumerated type
//		int offset - offset in IONet page (default 0)
// -------------------------------------------------------------------------------------------------------------------------------------------
IONetVar::IONetVar(char *dataPtr, varType type, std::string name, int offset) {

	if (dataPtr != 0) {
		this->value 		= 	dataPtr ;
		this->dataType 		= 	type ;
		this->offset		=	offset ;
		this->name			=	name ;

		// set size of this variabe
		if (type == integer)
			this->size = sizeof(int);
		if (type == real)
			this->size = sizeof(float);

	}
}

IONetVar::~IONetVar() {
	// TODO Auto-generated destructor stub
}


} /* namespace ION */
