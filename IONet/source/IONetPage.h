/*
 * IONetPage.h
 *
 *  Created on: May 19, 2015
 *      Author: strus
 */

#ifndef IONETPAGE_H_
#define IONETPAGE_H_

#include <map>
#include <string>
#include "IONetVar.h"

namespace ION {

class IONetPage {
public:
	IONetPage(int, std::map<std::string, IONetVar>);
	virtual ~IONetPage();

	// adds new variable to the map
	int insert(IONetVar *) ;

private:
	std::map<std::string, IONetVar>				*myMap ;
	int											myPageNumber ;
	char										*pageInBytes ;
	int											myPageSize ;
};

} /* namespace ION */
#endif /* IONETPAGE_H_ */
