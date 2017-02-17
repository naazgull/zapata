
(defun channels-collection-get (performative topic envelope)
  (let* ((indentity (zpt:authorize "/v2/datums/applications/{app_id}/users/{user_id}/channels" envelope))
         (t-split (zpt:split topic "/"))
	  (tv-app-id (zpt:topic-var t-split 3))
	  (tv-user-id (zpt:topic-var t-split 5)))
            (zpt:route "get"
                   (zpt:join (list  "v2" "datums" "applications" tv-app-id "users" tv-user-id "channels" ) "/")
                   (json "headers" (zpt:authorization-headers identity) "params" (zpt:merge (gethash "params" envelope) (json "app_id" tv-app-id "user_id" tv-user-id))))

(defun channels-collection-post (performative topic envelope)
  (let* ((indentity (zpt:authorize "/v2/datums/applications/{app_id}/users/{user_id}/channels" envelope))
         (t-split (zpt:split topic "/"))
	  (tv-app-id (zpt:topic-var t-split 3))
	  (tv-user-id (zpt:topic-var t-split 5)))
            (zpt:route "post"
                   (zpt:join (list  "v2" "datums" "applications" tv-app-id "users" tv-user-id "channels" ) "/")
                   (json "headers" (zpt:authorization-headers identity) "params" (zpt:merge (gethash "params" envelope) (json "app_id" tv-app-id "user_id" tv-user-id))))


(defun channels-collection-patch (performative topic envelope)
  (let* ((indentity (zpt:authorize "/v2/datums/applications/{app_id}/users/{user_id}/channels" envelope))
         (t-split (zpt:split topic "/"))
	  (tv-app-id (zpt:topic-var t-split 3))
	  (tv-user-id (zpt:topic-var t-split 5)))
            (zpt:route "patch"
                   (zpt:join (list  "v2" "datums" "applications" tv-app-id "users" tv-user-id "channels" ) "/")
                   (json "headers" (zpt:authorization-headers identity) "params" (zpt:merge (gethash "params" envelope) (json "app_id" tv-app-id "user_id" tv-user-id))))

(defun channels-collection-delete (performative topic envelope)
  (let* ((indentity (zpt:authorize "/v2/datums/applications/{app_id}/users/{user_id}/channels" envelope))
         (t-split (zpt:split topic "/"))
	  (tv-app-id (zpt:topic-var t-split 3))
	  (tv-user-id (zpt:topic-var t-split 5)))
            (zpt:route "delete"
                   (zpt:join (list  "v2" "datums" "applications" tv-app-id "users" tv-user-id "channels" ) "/")
                   (json "headers" (zpt:authorization-headers identity) "params" (zpt:merge (gethash "params" envelope) (json "app_id" tv-app-id "user_id" tv-user-id))))

(defun channels-collection-head (performative topic envelope)
  (let* ((indentity (zpt:authorize "/v2/datums/applications/{app_id}/users/{user_id}/channels" envelope))
         (t-split (zpt:split topic "/"))
	  (tv-app-id (zpt:topic-var t-split 3))
	  (tv-user-id (zpt:topic-var t-split 5)))
            (zpt:route "head"
                   (zpt:join (list  "v2" "datums" "applications" tv-app-id "users" tv-user-id "channels" ) "/")
                   (json "headers" (zpt:authorization-headers identity) "params" (zpt:merge (gethash "params" envelope) (json "app_id" tv-app-id "user_id" tv-user-id))))


(zpt:on "^/v2/datums/applications/([^/]+)/users/([^/]+)/channels$"
    (json
      "get" "channels-collection-get"
      "post" "channels-collection-post"
      "put" "channels-collection-put"
      "patch" "channels-collection-patch"
      "delete" "channels-collection-delete"
      "head" "channels-collection-head"
      )
    (json  "http" t "mqtt" t "0mq" t "amqp" t))

