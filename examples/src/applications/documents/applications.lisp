
(defun applications-document-get (performative topic envelope)
;; YOUR CODE HERE
)

(defun applications-document-put (performative topic envelope)
;; YOUR CODE HERE
)

(defun applications-document-patch (performative topic envelope)
;; YOUR CODE HERE
)

(defun applications-document-delete (performative topic envelope)
;; YOUR CODE HERE
)

(defun applications-document-head (performative topic envelope)
;; YOUR CODE HERE
)


(zpt-on "^/v2/datums/applications/([^/]+)$"
    `(
      (get . "applications-document-get")
      (put . "applications-document-put")
      (patch . "applications-document-patch")
      (delete . "applications-document-delete")
      (head . "applications-document-head")
      ))

