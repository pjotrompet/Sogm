;;; CLOS simple Framework for CAC-IV
;;; HISTORY: started 140708
;;; 230215
;;;


;; ----------------- SUPERCLASS ---------------------------
(defclass SuperClass ()
  ((name :initarg :name :initform "Super Class" :accessor name))
  )

(defmethod initialize-instance ((instance SuperClass) &rest initargs)
;;  (apply #'call-next-method SuperClass initargs)
  (print "SuperClass initialize-instance of ")
  (princ instance)
  (terpri))

(defgeneric print-class (instance args)
  (:documentation "print-class-def-generic (instance args &optional moreargs)"))

(defmethod print-class ((instance SuperClass) args)
  (declare (ignore args))
  (print "--> SuperClass: ")
  (princ instance)
  (terpri))

;; ----------------- CLASS A --------------------------- Single Inheritance
(defclass ClassA (SuperClass)
  ((name :initarg :name :initform "Class A" :accessor name))
  )

(defmethod initialize-instance :after ((SuperClass ClassA) &rest initargs)
;;  (apply #'call-next-method SuperClass initargs)
  (print "initialize-instance of ClassA")
  (princ initargs)
  (terpri)
  (print-class SuperClass nil)
  (terpri))

(defmethod print-class ((instance ClassA) args)
  (declare (ignore args))
  (print "defmethod print-class")
  (print "--> ClassA: ")
  (princ instance)
  (terpri))

;; ----------------- CLASS B --------------------------- Single Inheritance
(defclass ClassB (SuperClass)
  ((name :initarg :name :initform "Class B" :accessor name))
  )

(defmethod initialize-instance :after ((SuperClass ClassB) &rest initargs)
  ;;  (apply #'call-next-method SuperClass initargs)
  (print "initialize-instance of ClassB")
  (princ initargs)
  (terpri)
  (print-class SuperClass nil)
  (terpri))

(defmethod print-class ((instance ClassB) args)
 ; (declare (ignore args))
  (print "defmethod print-class")
  (print "--> ClassB: ")
  (princ instance)
  (princ args)
  (terpri))

;; ----------------- CLASS C --------------------------- Multiple Inheritence
(defclass ClassC (ClassA ClassB)
  ((name :initarg :name :initform "Class C" :accessor name))
  (:documentation "This is ClassC"))

(defmethod print-class ((instance ClassC) args)
  (declare (ignore args))
  (print "defmethod print-class")
  (print "--> ClassC: ")
  (princ instance)
  (princ args)
  (terpri))

(defmethod initialize-instance :after ((ClassB ClassC) &rest initargs)
  ;;  (apply #'call-next-method SuperClass initargs)
  (print "initialize-instance of ClassC")
  (princ initargs)
  (terpri)
  (print-class ClassB nil)
  (terpri))



;; --------------------------------------------

(defun MyTest ()
  (let* ((superClass (make-instance 'SuperClass :name "input SuperClass"))
         (theClass-A (make-instance 'ClassA     :name "input Class A"))
         (theClass-B (make-instance 'ClassB     :name "input Class B"))
         (theClass-C (make-instance 'ClassC     :name "input Class C")))
    (print-class theClass-A theClass-A)
    (print-class theClass-B theClass-B)
    (print-class theClass-C theClass-C)
    (print-class superClass superClass)))
