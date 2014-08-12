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
#include <parsers/json.h>

namespace zapata {

	class JobServer {
		public:
			JobServer(string _key_file_path);
			virtual ~JobServer();

			virtual void start();
			virtual void notify() = 0;
			virtual void wait() = 0;

			size_t max();
			size_t next();
			semid_t semid();
			void max(size_t _max);
			zapata::JSONObj& configuration();

		private:
			size_t __next;
			size_t __max_idx;
			semid_t __sem;

		protected:
			JSONObj __configuration;
			string __skey;
	};
}
