/*
The MIT License (MIT)

Copyright (c) 2014 n@zgul <naazgull@dfz.pt>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <zapata/auth/oauth2/AuthAgent.h>

zpt::oauth2::AuthAgent::AuthAgent(zpt::AuthAgentCallback _callback, zpt::JSONPtr& _options) : zpt::AuthAgent( _callback, _options ) {
}

zpt::oauth2::AuthAgent::~AuthAgent(){
}

std::string zpt::oauth2::AuthAgent::code_url() {

}

std::string zpt::oauth2::AuthAgent::token_url() {

}

std::string zpt::oauth2::AuthAgent::refresh_token_url() {
	
}

zpt::JSONPtr zpt::oauth2::AuthAgent::authenticate(zpt::JSONPtr _credentials) {
	zpt::HTTPReq _req;
	//_req->url(this->code_url());
	//_req->url(this->token_url());
	//_req->url(this->refresh_token_url());
	return zpt::undefined;
}