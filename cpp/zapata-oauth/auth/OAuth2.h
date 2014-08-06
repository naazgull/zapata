/*
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

#include <string>
#include <zapata/rest.h>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	class OAuth2 : public zapata::RESTDocument {
		public:
			/**
			 * Default constructor.
			 * @param _url_pattern regular expression that SHOULD match the request URL for this resource
			 */
			OAuth2(string _url_pattern);
			/**
			 * Receives the client configuration data.
			 * @param _url_pattern regular expression that should match the request URL for this resource
			 * @param _client_id the third-party application client_id
			 * @param _client_id the third-party application client_secret
			 * @param _scopes the third-party application requesting scopes
			 */
			OAuth2(string _url_pattern, string _client_id, string _client_secret, string _scopes);
			/**
			 * Default destructor.
			 */
			virtual ~OAuth2();

			/**
			 * Method to be overriden by child classes. SHOULD return the authentication endpoint for the child class target platform.
			 * @return the authentication endpoint for the child class target platform
			 */
			virtual string authEndpoint() = 0;
			/**
			 * Method to be overriden by child classes. SHOULD return the token endpoint for the child class target platform.
			 * @return the token endpoint for the child class target platform
			 */
			virtual string tokenEndpoint() = 0;
			/**
			 * Method to be overriden by child classes. SHOULD process the authentication data received from the child class target platform
			 * @param _auth the authentication data received from the child class target platform
			 */
			virtual void process(zapata::JSONObj& _auth) = 0;

			/**
			 * Setter method for the third-party application client_id
			 * @param _in the third-party application client_id
			 */
			virtual void id(string _in) final;
			/**
			 * Getter method for the thid-party application client_id
			 * @return the third-party application client_id
			 */
			virtual string& id() final;
			/**
			 * Setter method for the third-party application client_secret
			 * @param _in the third-party application client_secret
			 */
			virtual void secret(string _in) final;
			/**
			 * Getter method for the thid-party application client_secret
			 * @return the third-party application client_secret
			 */
			virtual string& secret() final;
			/**
			 * Setter method for the third-party application requesting scopes
			 * @param _in the third-party application requesting scopes
			 */
			virtual void scopes(string _in) final;
			/**
			 * Getter method for the thid-party application requesting scopes
			 * @return the third-party application requesting scopes
			 */
			virtual string& scopes() final;
			/**
			 * Setter method for the third-party application redirect_uri
			 * @param _in the third-party application redirect_uri
			 */
			virtual void redirect(string _in) final;
			/**
			 * Getter method for the thid-party application redirect_uri
			 * @return the third-party application redirect_uri
			 */
			virtual string& redirect() final;

			/**
			 * Processes the request for authentication, redirecting the client to the suitable URI, for target platform authentication
			 * @param _req
			 * @param _rep
			 */
			virtual void get(HTTPReq& _req, HTTPRep& _rep);
			/**
			 * Processes the request a PUT request, returning a "405 Method Not Allowed ",
			 * @param _req
			 * @param _rep
			 */
			virtual void put(HTTPReq& _req, HTTPRep& _rep) final;
			/**
			 * Processes the request a DELETE request, returning a "405 Method Not Allowed ",
			 * @param _req
			 * @param _rep
			 */
			virtual void remove(HTTPReq& _req, HTTPRep& _rep) final;
			/**
			 * Processes the request a HEAD request, returning a "405 Method Not Allowed ",
			 * @param _req
			 * @param _rep
			 */
			virtual void head(HTTPReq& _req, HTTPRep& _rep);
			/**
			 * Processes the request a PATCH request, returning a "405 Method Not Allowed ",
			 * @param _req
			 * @param _rep
			 */
			virtual void patch(HTTPReq& _req, HTTPRep& _rep) final;

		private:
			string __id;
			string __secret;
			string __scopes;
			string __redirect_uri;


	};

}
