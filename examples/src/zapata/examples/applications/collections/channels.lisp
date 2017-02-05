
(defun channels-collection-get (performative topic envelope)
   (zpt:validate-authorization envelope)
   (setf t-split (zpt:split topic "/"))
   (setf tv-app-id (zpt:topic-var t-split 3))
   (setf tv-user-id (zpt:topic-var t-split 5))
   ;; YOUR CODE HERE
   (json "status" 204)
)

(defun channels-collection-head (performative topic envelope)
   (zpt:validate-authorization envelope)
   (setf t-split (zpt:split topic "/"))
   (setf tv-app-id (zpt:topic-var t-split 3))
   (setf tv-user-id (zpt:topic-var t-split 5))
   ;; YOUR CODE HERE
   (json "status" 204)
)


(zpt:on "^/v2/datums/applications/([^/]+)/users/([^/]+)/channels$"
    (json
      "get" "channels-collection-get"
      "head" "channels-collection-head"
      )
    (json  "http" t "mqtt" t "0mq" t "amqp" t)
    )

