/*
 *   This file is part of IONet.
 *
<<<<<<< HEAD
 *   IONet is free software: you can redistribute it and/or modify
=======
 *   RoverPlatform is free software: you can redistribute it and/or modify
>>>>>>> 376b3c54b712d5ce946455ae54ba2d7573f07313
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   any later version.
 *
<<<<<<< HEAD
 *   IONet is distributed in the hope that it will be useful,
=======
 *   RoverPlatform is distributed in the hope that it will be useful,
>>>>>>> 376b3c54b712d5ce946455ae54ba2d7573f07313
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

	// print variable list and values
	void printPage() ;

private:
	std::map<std::string, IONetVar>				*myMap ;
	int											myPageNumber ;
	char										*pageInBytes ;
	int											myPageSize ;
};

} /* namespace ION */
#endif /* IONETPAGE_H_ */
