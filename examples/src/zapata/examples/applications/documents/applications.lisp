
(defun applications-document-get (performative topic envelope)
  (let* ((indentity (zpt:authorize envelope))
         (t-split (zpt:split topic "/"))
	  (tv-id (zpt:topic-var t-split 3)))
    ;; ---> YOUR CODE HERE <--- ;;
    (json "status" 204)))

(defun applications-document-put (performative topic envelope)
  (let* ((indentity (zpt:authorize envelope))
         (t-split (zpt:split topic "/"))
	  (tv-id (zpt:topic-var t-split 3)))
    ;; ---> YOUR CODE HERE <--- ;;
    (json "status" 200)))

(defun applications-document-patch (performative topic envelope)
  (let* ((indentity (zpt:authorize envelope))
         (t-split (zpt:split topic "/"))
	  (tv-id (zpt:topic-var t-split 3)))
    ;; ---> YOUR CODE HERE <--- ;;
    (json "status" 200)))

(defun applications-document-delete (performative topic envelope)
  (let* ((indentity (zpt:authorize envelope))
         (t-split (zpt:split topic "/"))
	  (tv-id (zpt:topic-var t-split 3)))
    ;; ---> YOUR CODE HERE <--- ;;
    (json "status" 200)))

(defun applications-document-head (performative topic envelope)
  (let* ((indentity (zpt:authorize envelope))
         (t-split (zpt:split topic "/"))
	  (tv-id (zpt:topic-var t-split 3)))
    ;; ---> YOUR CODE HERE <--- ;;
    (json "status" 200 "headers" (json "Content-Length" body-length) )))


(zpt:on "^/v2/datums/applications/([^/]+)$"
    (json
      "get" "applications-document-get"
      "put" "applications-document-put"
      "patch" "applications-document-patch"
      "delete" "applications-document-delete"
      "head" "applications-document-head"
      )
    (json  "http" t "mqtt" t "0mq" t "amqp" t))

