#pragma once


#include <vector>
#include <thread/JobChannel.h>
#include <partition/Partitioner.h>
#include <map/Mapper.h>
#include <reduce/Reducer.h>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	class DirectoryService : public Job {
		public:
			DirectoryService(string _key_file_path);
			virtual ~DirectoryService();

			virtual void signalPartioners();
			virtual void signalMappers();
			virtual void signalReducers();

			virtual void addPartioner(zapata::Partitioner& _partitioner);
			virtual void addMapper(zapata::Mapper& _mapper) ;
			virtual void addReducer(zapata::Reducer& _reducer);
			virtual void addPartioner(zapata::JobChannel* _partitioner);
			virtual void addMapper(zapata::JobChannel* _mapper) ;
			virtual void addReducer(zapata::JobChannel* _reducer);

			virtual void removePartioner(zapata::Partitioner& _partitioner);
			virtual void removeMapper(zapata::Mapper& _mapper);
			virtual void removeReducer(zapata::Reducer& _reducer);

			virtual void run();

		private:
			vector<JobChannel*> __partitioners;
			vector<JobChannel*> __mappers;
			vector<JobChannel*> __reducers;

	};

}
