consume(Status) :-
    zpt_make_request(tcp, Req1),
    zpt_config(Config),
    zpt_value_for(Config, "rest.prefix", Prefix1),
    string_concat(Prefix1, "/test_plugin", Prefix),
    Req2 = (Req1,
            performative:"POST",
            uri:Prefix,
            body:(name:"client", date:"2026-01-01T00:00:00.000")),
    zpt_call(Req2, Reply),
    zpt_value_for(Reply, "status", Status),
    zpt_log(["Received message: ", Reply]).

consume :-
    consume(200) ;
    (
        sleep(1),
        consume
    ).
