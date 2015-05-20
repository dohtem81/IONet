/*
 *   This file is part of IONet.
 *
 *   IONet is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   any later version.
 *
 *   IONet is distributed in the hope that it will be useful,
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

#include "IONetPage.h"
#include "IONetVar.h"
#include <iostream>

namespace ION {

// constructor
// requires pageNumber and map of variables
//------------------------------------------------------------------------------
IONetPage::IONetPage(int pageNumber, std::map<std::string, IONetVar> varsMap) {
	// first lets initialize page number
	this->myPageNumber		=	pageNumber ;

	// empty map
	this->myMap 			=	new std::map<std::string, IONetVar>() ;

	// at this moment page is empty
	// we need to add all variables that were passed in constructor call
	// then we need to calculate page size
	this->myPageSize 		=	0 ;

	// insert all variables from varsMap into object map
	this->myMap->insert(varsMap.begin(), varsMap.end());

	// now we need to calculate byte size
	for ( std::map<std::string, IONetVar>::iterator it = this->myMap->begin();
	      it != this->myMap->end();
	      it++) {

		// set offset for that variable and calculate page size
		it->second.setOffset(this->myPageSize) ;
		this->myPageSize += it->second.getSize() ;
	}

	// once we know total size we can allocate memory for all bytes needed
	this->pageInBytes = new char[ this->myPageSize ] ;

	// now set values on the byte map
	for ( std::map<std::string, IONetVar>::iterator it = this->myMap->begin();
	      it != this->myMap->end();
	      it++) {

		IONetVar var = it->second ;
		if (var.getType() == integer) {
			int *pointInMap = (int *)(this->pageInBytes + var.getOffset()) ;
			*pointInMap = (int)(*var.getValue()) ;
		}
		if (var.getType() == real) {
			float *pointInMap = (float *)(this->pageInBytes + var.getOffset()) ;
			*pointInMap = (float)(*var.getValue()) ;
		}
	}

}

IONetPage::~IONetPage() {
	// TODO Auto-generated destructor stub
}


// print variable list and values
// -------------------------------------------------------------------------------------------------------------------------------------------
void IONetPage::printPage(){
	std::cout << "Page#: " << this->myPageNumber
			  << " size: " << this->myPageSize << "[b]" << std::endl ;

	// loop through map
	for ( std::map<std::string, IONetVar>::iterator it = this->myMap->begin();
	      it != this->myMap->end();
	      it++) {

		std::cout << it->first << "\t" ;

		IONetVar var = it->second ;
		if (var.getType() == integer) {
			int *pointInMap = (int *)(var.getValue()) ;
			std::cout << *pointInMap ;
		}
		if (var.getType() == real) {
			float *pointInMap = (float *)(var.getValue()) ;
			std::cout << *pointInMap ;
		}
		std::cout << std::endl ;
	}
}


} /* namespace ION */
