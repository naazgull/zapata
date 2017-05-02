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

#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>

#include <zapata/rest.h>
#include <zapata/mem/usage.h>
#include <semaphore.h>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

int main(int argc, char* argv[]) {
	/*
	  [Unit]                                                                                                                                                                                                                                                                    
	  Description=Myfox Container                                                                                                                                                                                                                                               
	  After=docker.service                                                                                                                                                                                                                                                      
	  Requires=docker.service                                                                                                                                                                                                                                                   
                                                                                                                                                                                                                                                                          
	  [Service]                                                                                                                                                                                                                                                                 
	  TimeoutStartSec=0                                                                                                                                                                                                                                                         
	  Restart=on-failure                                                                                                                                                                                                                                                        
	  RemainAfterExit=true                                                                                                                                                                                                                                                      
	  ExecStartPre=-/usr/bin/docker stop manager-myfox-contained                                                                                                                                                                                                                
	  ExecStartPre=-/usr/bin/docker rm manager-myfox-contained                                                                                                                                                                                                                  
	  ExecStart=/usr/bin/docker run -d --name manager-myfox-contained --net=host -P                                                                                                                                                                                             
	  -v /var/log/zapata:/var/log/zapata -v /etc/muzzley/manager-myfox-contained:/etc                                                                                                                                                                                           
	  /zapata/backend-enabled -v /home/projects/manager-myfox-contained:/usr/share/                                                                                                                                                                                             
	  muzzley/manager-myfox-contained dockermuzz/managers:3.0.0                                                                                                                                                                                                                 
                                                                                                                                                                                                                                                                          
	  ExecStop=/usr/bin/docker stop -t 2 manager-myfox-contained                                                                                                                                                                                                                
	  ExecStopPost=/usr/bin/docker rm -f manager-myfox-contained                                                                                                                                                                                                                
                                                                                                                                                                                                                                                                          
	  [Install]                                                                                                                                                                                                                                                                 
	  WantedBy=multi-user.target      
	 */

	zpt::json _args = zpt::conf::getopt(argc, argv);
	if (_args["add"]) {
		std::istringstream
	}
	else if (_args["remove"]) {
		
	}
	
	return 0;
}
