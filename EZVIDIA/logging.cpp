//This file is part of EZVIDIA.
//
//EZVIDIA is free software : you can redistribute itand /or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
//EZVIDIA is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with EZVIDIA. If not, see < https://www.gnu.org/licenses/>.

#include "logging.hpp"

void logPrint(std::string msg) {
	auto t = std::time(nullptr);
	struct tm newtime;
	localtime_s(&newtime, &t);
	logMutex.lock();
	logStream << "[" << std::put_time(&newtime, "%d-%m-%Y %H:%M:%S") << "] " << msg << std::endl;
	logStream.flush();
	logMutex.unlock();
}