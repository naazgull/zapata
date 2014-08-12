/*
    Author: Pedro (n@zgul) Figueiredo <pedro.figueiredo@gmail.com>
    Copyright (c) 2014 Pedro (n@zgul)Figueiredo
    This file is part of Zapata.

    Zapata is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Zapata is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the Lesser GNU General Public License
    along with Zapata.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <thread/Job.h>
#include <api/RESTPool.h>
#include <queue>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	class RESTJob: public Job {
		public:
			RESTJob(string _key_file_path);
			virtual ~RESTJob();

			virtual void run();
			virtual void assign(int _cs_fd);

			RESTPool& pool();
			void pool(RESTPool* _pool);

		private:
			queue<int> __cur_fd;
			RESTPool* __pool;
	};

}
