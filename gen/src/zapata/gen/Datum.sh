
curl -i http://${1}:${2}/${3}_$[datum.name] \
     -X PUT
$[datum.fields]
