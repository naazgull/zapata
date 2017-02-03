
(defun applications-collection-get (performative topic envelope)
;; YOUR CODE HERE
)

(defun applications-collection-post (performative topic envelope)
;; YOUR CODE HERE
)

(defun applications-collection-patch (performative topic envelope)
;; YOUR CODE HERE
)

(defun applications-collection-delete (performative topic envelope)
;; YOUR CODE HERE
)

(defun applications-collection-head (performative topic envelope)
;; YOUR CODE HERE
)


(zpt-on "^/v2/datums/applications$"
    (json
      "get" "applications-collection-get"
      "post" "applications-collection-post"
      "patch" "applications-collection-patch"
      "delete" "applications-collection-delete"
      "head" "applications-collection-head"
      )
    (json  "http" t "mqtt" t "0mq" t "amqp" t)
    )

