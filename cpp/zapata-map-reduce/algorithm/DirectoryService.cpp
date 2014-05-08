#include <algorithm/DirectoryService.h>

zapata::DirectoryService::DirectoryService(string _key_file_path) : zapata::Job(_key_file_path) {
}

zapata::DirectoryService::~DirectoryService() {
}

void zapata::DirectoryService::signalPartioners() {
}

void zapata::DirectoryService::signalMappers() {
}

void zapata::DirectoryService::signalReducers() {
}

void zapata::DirectoryService::addPartioner(zapata::Partitioner& _partitioner) {
	JobChannel* _jc = _partitioner.channel();
	this->__partitioners.push_back(_jc);
}

void zapata::DirectoryService::addMapper(zapata::Mapper& _mapper) {
	JobChannel* _jc = _mapper.channel();
	this->__mappers.push_back(_jc);
}

void zapata::DirectoryService::addReducer(zapata::Reducer& _reducer) {
	JobChannel* _jc = _reducer.channel();
	this->__reducers.push_back(_jc);
}

void zapata::DirectoryService::addPartioner(zapata::JobChannel* _partitioner) {
	this->__partitioners.push_back(_partitioner);
}

void zapata::DirectoryService::addMapper(zapata::JobChannel* _mapper) {
	this->__mappers.push_back(_mapper);
}

void zapata::DirectoryService::addReducer(zapata::JobChannel*_reducer) {
	this->__reducers.push_back(_reducer);
}

void zapata::DirectoryService::removePartioner(zapata::Partitioner& _partitioner) {
}

void zapata::DirectoryService::removeMapper(zapata::Mapper& _mapper) {
}

void zapata::DirectoryService::removeReducer(zapata::Reducer& _reducer) {
}

void zapata::DirectoryService::run() {
}
