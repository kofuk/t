;;;          -*- lexical-binding: t; -*-
(defun memfile-update-addr ()
  "Update addr in current paragraph"
  (interactive)
  (save-excursion
    (save-restriction
      (goto-char (point-min))
      (let ((current-addr 0) (line-str "") (should-continue t))
        (while should-continue
          (setq line-str (string-replace "\n" "" (thing-at-point 'line t)))
          (if (string-match-p "@[0-9a-z][0-9a-z] ?" line-str)
              (progn
                (forward-char 1)
                (delete-char 2)
                (insert (format "%02x" current-addr))
                (if (= (length line-str) 3) ;; lines like "@00" only; put zeros.
                    (insert " 00000000 00000000"))
                (forward-line)
                (beginning-of-line))
            (setq should-continue nil))
          (setq current-addr (+ current-addr 2)))))))

(provide 'memfile)
;;; memfile.el ends here
