
(defun applications-collection-get (performative topic envelope)
  (let* ((indentity (zpt:authorize "/v2/datums/applications" envelope))
         (t-split (zpt:split topic "/")))
    ;; ---> YOUR CODE HERE <--- ;;
    (json "status" 204)))

(defun applications-collection-post (performative topic envelope)
  (let* ((indentity (zpt:authorize "/v2/datums/applications" envelope))
         (t-split (zpt:split topic "/")))
    ;; ---> YOUR CODE HERE <--- ;;
    (json "status" 201)))

(defun applications-collection-patch (performative topic envelope)
  (let* ((indentity (zpt:authorize "/v2/datums/applications" envelope))
         (t-split (zpt:split topic "/")))
    ;; ---> YOUR CODE HERE <--- ;;
    (json "status" 200)))

(defun applications-collection-delete (performative topic envelope)
  (let* ((indentity (zpt:authorize "/v2/datums/applications" envelope))
         (t-split (zpt:split topic "/")))
    ;; ---> YOUR CODE HERE <--- ;;
    (json "status" 200)))

(defun applications-collection-head (performative topic envelope)
  (let* ((indentity (zpt:authorize "/v2/datums/applications" envelope))
         (t-split (zpt:split topic "/")))
    ;; ---> YOUR CODE HERE <--- ;;
    (json "status" 200 "headers" (json "Content-Length" body-length) )))


(zpt:on "^/v2/datums/applications$"
    (json
      "get" "applications-collection-get"
      "post" "applications-collection-post"
      "patch" "applications-collection-patch"
      "delete" "applications-collection-delete"
      "head" "applications-collection-head"
      )
    (json  "http" t "mqtt" t "0mq" t "amqp" t))

