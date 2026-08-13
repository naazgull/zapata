myapi = {
	user = {
		crud = {},
	},
}

local _user

function myapi.user.crud.create()
	local _conf = zpt.config()

	local _request = zpt.make_request("tcp")
	_request.performative = "POST"
	_request.uri = _conf.rest.prefix .. "/users"
	_request.body = {
		name = "José da Silva",
		email = "jose.silva@unknown.com",
		birthday = "1981-01-01",
	}

	local _reply = zpt.call(_request)
	assert(_reply.status == 201 and _reply.body and _reply.body._id, "unable to create user")

	_user = _reply.body
end

function myapi.user.crud.read()
	local _conf = zpt.config()

	local _request = zpt.make_request("tcp")
	_request.performative = "GET"
	_request.uri = _conf.rest.prefix .. "/users/" .. _user._id

	local _reply = zpt.call(_request)
	assert(_reply.status == 200 and _reply.body and _reply.body.email == _user.email, "unable to read user")

	_user = _reply.body
end

function myapi.user.crud.update()
	local _conf = zpt.config()

	local _request = zpt.make_request("tcp")
	_request.performative = "PATCH"
	_request.uri = _conf.rest.prefix .. "/users/" .. _id
	_request.body = {
		birthday = "1981-01-10",
	}

	local _reply = zpt.call(_request)
	assert(_reply.status == 202, "unable to update user")

	myapi.user.crud.read()
	assert(_user.birthday == "1981-01-10", "user data was not updated")
end

function myapi.user.crud.delete()
	local _conf = zpt.config()

	local _request = zpt.make_request("tcp")
	_request.performative = "DELETE"
	_request.uri = _conf.rest.prefix .. "/users/" .. _id

	local _reply = zpt.call(_request)
	assert(_reply.status == 202, "unable to update user")

	_request = zpt.make_request("tcp")
	_request.performative = "GET"
	_request.uri = _conf.rest.prefix .. "/users/" .. _user._id

	_reply = zpt.call(_request)
	assert(_reply.status == 404, "user still acessible after delete")
end

function myapi.user.crud.run()
	myapi.user.crud.create()
	myapi.user.crud.read()
	myapi.user.crud.update()
	myapi.user.crud.delete()
end

return myapi.user.crud
