#include <algorithm/ClusterDirectoryService.h>

#include <zapata/net.h>

zapata::ClusterDirectoryService::ClusterDirectoryService(string _key_file_path) : zapata::DirectoryService(_key_file_path) {
}

zapata::ClusterDirectoryService::~ClusterDirectoryService(){
}

void zapata::ClusterDirectoryService::run() {
	zapata::serversocketstream _ss;
	_ss.bind((int) this->configuration()["core"]["port"]);
	for (; true; ) {
		int _cs;
		_ss.accept(&_cs);
	}

}
