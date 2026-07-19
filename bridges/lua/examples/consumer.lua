local example_lua_consumer = {}

function example_lua_consumer.consume()
   local _conf = zpt.config()
   local _request = zpt.make_request("tcp")

   _request.performative = "POST"
   _request.uri = _conf.rest.prefix.."/test_plugin"
   _request.body = {}
   _request.body.name = "client"
   _request.body.date = "2026-01-01T00:00:00.000"

   local _reply = zpt.call(_request)
   print("Received message with status ".._reply.status)
end

return example_lua_consumer
