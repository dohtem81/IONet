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

#include "IONet.h"
#include "IONetVar.h"
#include "IONetPage.h"
#include <iostream>
//#include <map>

namespace ION {

// this constructor creates empty network
// this is base for network mapping
IONet::IONet() {

	// create map to keep all pages
	this->ioPages	=	new std::map<int, IONetPage> ();
}

IONet::~IONet() {
	// TODO Auto-generated destructor stub
}

// adds new page to network definition
// returns -1 if page already exists
//          0 if success
//----------------------------------------------------------------------------------------------
int IONet::addPage(int pageNumber, std::map<std::string, IONetVar> variables) {

	// first we check if specified page already exists
	// if it already exists return -1
	std::map<int, IONetPage>::iterator page = this->ioPages->find(pageNumber) ;
	if (page != this->ioPages->end()) {
		std::cout << "Page# " << pageNumber << " already exists!" << std::endl ;
		return -1 ;
	}

	// create new page and add it to page list
	IONetPage *tempPage = new IONetPage(pageNumber, variables) ;
	this->ioPages->insert(std::pair<int, IONetPage>(pageNumber, *tempPage));
	delete tempPage ;

	return 0 ;
}


// prints page variables and values
//----------------------------------------------------------------------------------------------
void IONet::printPage(int pageNumber){
	// first we check if specified page already exists
	// if it already exists return
	std::map<int, IONetPage>::iterator page = this->ioPages->find(pageNumber) ;
	if (page == this->ioPages->end()) {
		std::cout << "Page does not exists" << std::endl ;
		return ;
	}

	// page was set by find method, we can call rintPage
	page->second.printPage() ;

	return ;

}

} /* namespace ION */



using namespace ION ;
// for testing purposes
// *******************************
int main() {

	IONet *network = new IONet();

	int varA, varB ;
	float varC ;

	// set values
	varA = 1 ;
	varB = 2 ;
	varC = 3.1 ;

	IONetVar *netVarA, *netVarB, *netVarC ;

	netVarA = new IONetVar((char*)&varA, integer, "varA") ;
	netVarB = new IONetVar((char*)&varB, integer, "varB") ;
	netVarC = new IONetVar((char*)&varC, real, "varC") ;

	std::map<std::string, IONetVar> *tempMap = new std::map<std::string, IONetVar>() ;
	tempMap->insert(std::pair<std::string, IONetVar>(netVarA->getName(), *netVarA)) ;
	tempMap->insert(std::pair<std::string, IONetVar>(netVarB->getName(), *netVarB)) ;
	tempMap->insert(std::pair<std::string, IONetVar>(netVarC->getName(), *netVarC)) ;

	// add new page to network
	network->addPage(1, *tempMap);
	network->addPage(1, *tempMap);

	// delete objects
	tempMap->empty();
	delete tempMap ;
	delete netVarA ;
	delete netVarB ;
	delete netVarC ;

	// here we can test page
	network->printPage(1);
	network->printPage(2);

	std::cout << "done..." << std::endl ;

	return 0 ;
}
