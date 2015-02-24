; lisp
; mediaan functie

(defun CalcMedian (lst)
  (if (evenp lst)
  (nth lst (/ (length lst) 2))
  (/ (+(/ (length lst) 2)())2)
