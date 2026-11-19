(clear)
(load "templates.clp")
(load "confidence.clp")
(load "inference.clp")

(reset)

(load-axioms "axioms.txt")
(load-rules "rules.txt")
(load-target "target.txt")

(run-forward)
(print-result)
