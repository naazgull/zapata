
(defun channels-collection-get (performative topic envelope)
;; YOUR CODE HERE
)

(defun channels-collection-head (performative topic envelope)
;; YOUR CODE HERE
)


(zpt-on "^/v2/datums/applications/([^/]+)/users/([^/]+)/channels$"
    `(
      (get . "channels-collection-get")
      (head . "channels-collection-head")
      ))

