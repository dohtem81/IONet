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

#ifndef IONET_H_
#define IONET_H_

#include <map>
#include <string>
#include "IONetVar.h"
#include "IONetPage.h"

namespace ION {

class IONet {
public:
	IONet();
	virtual ~IONet();

	// adds new page to network definition
	int addPage(int, std::map<std::string, IONetVar>) ;

	// prints specified page
	void printPage(int) ;

private:
	std::map<int, IONetPage>		*ioPages ;
};

} /* namespace ION */
#endif /* IONET_H_ */
