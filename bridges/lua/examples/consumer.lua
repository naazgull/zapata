local example_lua_consumer = {}

function example_lua_consumer.consume()
   local _conf = zpt.config()

   local _status = 0
   local _reply
   while (_status ~= 200) do
      local _request = zpt.make_request("tcp")
      _request.performative = "POST"
      _request.uri = _conf.rest.prefix.."/test_plugin"
      _request.body = {
         name = "client",
         date = "2026-01-01T00:00:00.000"
      }

      _reply = zpt.call(_request)
      _status = _reply.status
   end

   zpt.log("Received message with status ", _reply)
end

return example_lua_consumer
