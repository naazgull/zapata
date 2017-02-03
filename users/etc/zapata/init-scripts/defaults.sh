#!/bin/bash

zpt -c /etc/zapata/backend-available/auth.conf -l 0 -r &

# CLEAN UP
redis-cli del 'applications/authentication/tokens/default' &> /dev/null
redis-cli hdel 'applications/clients/apps' '/0.9/apps/00000000-0000-0000-0000-000000000000' &> /dev/null
mongo localhost/users --eval 'db.users.remove({"role" : "administrator"})' &> /dev/null
mongo localhost/users --eval 'db.roles.drop()' &> /dev/null

# SETUP DEFAULTS
redis-cli hset 'applications/clients/apps' '/0.9/apps/00000000-0000-0000-0000-000000000000' '{ "_id" : "/0.9/apps/00000000-0000-0000-0000-000000000000", "href" : "/0.9/apps/00000000-0000-0000-0000-000000000000", "id" : "00000000-0000-0000-0000-000000000000", "name" : "confapp", "description" : "Configuration Application", "scope" : "all{arwx}", "redirect_domain" : "localhost" }'  &> /dev/null
mongo localhost/users --eval 'db.roles.insert({ "_id" : "/0.9/roles/administrator", "href" : "/0.9/roles/administrator", "id" : "administrator", "name" : "Administrator", "scope" : "all{arwx}" })' &> /dev/null
mongo localhost/users --eval 'db.roles.insert({ "_id" : "/0.9/roles/app-admin", "href" : "/0.9/roles/app-admin", "id" : "app-admin", "name" : "Application Administrator", "scope" : "apps{arwx}" })' &> /dev/null
mongo localhost/users --eval 'db.roles.insert({ "_id" : "/0.9/roles/user", "href" : "/0.9/roles/user", "id" : "user", "name" : "User", "scope" : "me{arwx},users{r},roles{r}" })' &> /dev/null
mongo localhost/users --eval 'db.roles.insert({ "_id" : "/0.9/roles/guest", "href" : "/0.9/roles/guest", "id" : "guest", "name" : "Guest", "scope" : "open" })' &> /dev/null

unset password
prompt="Enter administrator password: "
while IFS= read -p "$prompt" -r -s -n 1 char
do
    if [[ $char == $'\0' ]]
    then
	break
    fi
    prompt='*'
    password+="$char"
done
echo

zcli -b tcp://localhost:10009 -t req -m POST -u /0.9/users -j "{ \"name\" : \"Administrator\", \"username\" : \"root\", \"e-mail\" : \"root@localhost\", \"password\" : \"$password\", \"role\" : \"administrator\" }" -l 0 -r
zcli -b tcp://localhost:10009 -t req -m POST -u /0.9/oauth2.0/authorize -j "{ \"response_type\" : \"password\", \"client_id\" : \"00000000-0000-0000-0000-000000000000\", \"redirect_uri\" : \"/blank\", \"scope\" : \"all{arwx}\", \"username\" : \"root\", \"password\" : \"$password\", \"state\" : \"administrator\" }" -l 0 -r

kill -9 $(ps ax | grep 'zpt -c /etc/zapata/backend-available/auth.conf' | grep -v grep | awk '{print $1}')

echo "Successfully create root user and default application"
