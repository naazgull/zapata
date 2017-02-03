(defun document-get (_performative _topic _envelope)
  (zlog _envelope 7))

(defun document-post (_performative _topic _envelope)
  (zlog _envelope 7))

(zpt-on "/v2/document"
    `(
      (get . "document-get")
      (post . "document-post")
      ))
