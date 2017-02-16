
(defun roles-collection-get (performative topic envelope)
  (let* ((indentity (zpt:authorize envelope))
         (t-split (zpt:split topic "/")))
    ;; ---> YOUR CODE HERE <--- ;;
    (json "status" 204)))

(defun roles-collection-post (performative topic envelope)
  (zpt:assertz-timestamp (gethash "payload" envelope) "created" 412)
  (add-to-object (gethash "payload" envelope) "created" (zpt:json-date))
   (zpt:assertz-mandatory (gethash "payload" envelope) "type" 412)
  (zpt:assertz-ascii (gethash "payload" envelope) "type" 412)
  (zpt:assertz-timestamp (gethash "payload" envelope) "updated" 412)
  (add-to-object (gethash "payload" envelope) "updated" (zpt:json-date))
  (let* ((indentity (zpt:authorize envelope))
         (t-split (zpt:split topic "/")))
    ;; ---> YOUR CODE HERE <--- ;;
    (json "status" 201)))

(defun roles-collection-patch (performative topic envelope)
  (zpt:assertz-timestamp (gethash "payload" envelope) "created" 412)
  (zpt:assertz-ascii (gethash "payload" envelope) "type" 412)
  (zpt:assertz-timestamp (gethash "payload" envelope) "updated" 412)
  (let* ((indentity (zpt:authorize envelope))
         (t-split (zpt:split topic "/")))
    ;; ---> YOUR CODE HERE <--- ;;
    (json "status" 200)))

(defun roles-collection-delete (performative topic envelope)
  (let* ((indentity (zpt:authorize envelope))
         (t-split (zpt:split topic "/")))
    ;; ---> YOUR CODE HERE <--- ;;
    (json "status" 200)))

(defun roles-collection-head (performative topic envelope)
  (let* ((indentity (zpt:authorize envelope))
         (t-split (zpt:split topic "/")))
    ;; ---> YOUR CODE HERE <--- ;;
    (json "status" 200 "headers" (json "Content-Length" body-length) )))


(zpt:on "^/v2/datum/roles$"
    (json
      "get" "roles-collection-get"
      "post" "roles-collection-post"
      "patch" "roles-collection-patch"
      "delete" "roles-collection-delete"
      "head" "roles-collection-head"
      )
    (json  "http" t "mqtt" t "0mq" t "amqp" t))

