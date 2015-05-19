/*
 * IONetPage.cpp
 *
 *  Created on: May 19, 2015
 *      Author: strus
 */

#include "IONetPage.h"
#include "IONetVar.h"

namespace ION {

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

		this->myPageSize += it->second.getSize() ;
	}

	// once we know total size we can allocate memory for all bytes needed
	this->pageInBytes = new char[ this->myPageSize ] ;
}

IONetPage::~IONetPage() {
	// TODO Auto-generated destructor stub
}

} /* namespace ION */
