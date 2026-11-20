; ============================================
; Прямой вывод с коэффициентами уверенности

; ==> Некоторые вспомогательные функции

; Разбить строку по разделителю
(deffunction split-delim (?s ?delim)
    (bind ?res (create$)) ; create$ - создать пустой список
    (bind ?pos (str-index ?delim ?s)) ; str-index - позиция подстроки в строке
    (while (not (eq ?pos FALSE)) ; пока не вышли за границы строки
        (bind ?part (sub-string 1 (- ?pos 1) ?s)) ; подстрока от 1 до pos - 1 в строке s
        (bind ?res (create$ ?res ?part)) ; добавлеяем подстроку
        (bind ?s (sub-string (+ ?pos (str-length ?delim)) (str-length ?s) ?s)) ; обрезаем строку и идем дальше
        (bind ?pos (str-index ?delim ?s))
    )
    (bind ?res (create$ ?res ?s)) ; добавляем остаток
    (return ?res)
)

; CF факта по id
(deffunction fact-cf (?id)
    (bind ?cf 0.0)

    (do-for-all-facts ((?f known-fact)) 
        (eq ?f:id ?id)
        (bind ?cf ?f:cf)
    )

    (return ?cf)
)

; Вклад правила в факт
(deffunction get-contribution-cf (?rule-id ?fact-id)
    (bind ?cf 0.0)

    (do-for-all-facts ((?contribution cf-contribution))
        (and (eq ?contribution:rule-id ?rule-id)
             (eq ?contribution:conclusion ?fact-id))
        (bind ?cf ?contribution:cf)
    )

    (return ?cf)
)

; Записать/обновить факт с защитой аксиом
(deffunction set-known-fact (?id ?cf ?origin)
    (bind ?found FALSE)
    (do-for-all-facts ((?f known-fact))
        (eq ?f:id ?id)
        (bind ?found TRUE)
        (if (eq ?f:origin derived) ; если это производный факт
            then (if (> ?cf ?f:cf)
                then (modify ?f (cf ?cf)))
        ) 
    )
    (if (not ?found)
        then (assert (known-fact (id ?id) (cf ?cf) (origin ?origin)))
    )
)

(deffunction compute-rule-cf (?rule)
    (bind ?premise-cfs (create$))
    (bind ?all-present TRUE) ; все посылки подтверждены
    (bind ?rule-cf (fact-slot-value ?rule cf)) ; fact-slot-value - дать значение слота (берем cf у rule)
    (bind ?premises (fact-slot-value ?rule premises))

    (progn$ (?p ?premises)
        (bind ?cf (fact-cf ?p))
        (if (<= ?cf 0.0)
            then (bind ?all-present FALSE))
        (bind ?premise-cfs (create$ ?premise-cfs ?cf))
    )

    (if (not ?all-present)
        then (return 0.0)
    )
    (return (calculate-rule-cf ?rule-cf ?premise-cfs))
)

; ==> Загрузка данных
(deffunction load-axioms (?file)
    (bind ?log (sym-cat "axioms-" (gensym*))) ; генерация потока на каждый вызов

    (if (eq (open ?file ?log "r") FALSE)
        then (return FALSE))
    
    (bind ?line (readline ?log))
    (while (neq ?line EOF)
        (if (and (> (str-length ?line) 0)
            (neq (sub-string 1 1 ?line) "#"))
            then
                (bind ?parts (split-delim ?line ";")) ; split-delim по ;
                (bind ?id (sym-cat (nth$ 1 ?parts)))
                (bind ?cf 1.0)
                (if (>= (length$ ?parts) 2)
                    then (bind ?cf (string-to-field (nth$ 2 ?parts)))) ; CF (в число через string-to-field)
                (assert (known-fact (id ?id) (cf ?cf) (origin axiom)))
        )
        (bind ?line (readline ?log))
    )
    (close ?log)
    (return TRUE)
)

(deffunction load-rules (?file)
    (bind ?log (sym-cat "rules-" (gensym*)))
    (if (eq (open ?file ?log "r") FALSE)
        then (return FALSE))
    (bind ?line (readline ?log))
    (while (neq ?line EOF)
        (if (and (> (str-length ?line) 0)
                 (neq (sub-string 1 1 ?line) "#"))
            then
                (bind ?parts (split-delim ?line ";"))
                (bind ?id (sym-cat (nth$ 1 ?parts)))
                (bind ?prem-str (nth$ 2 ?parts))
                (bind ?premises (create$))
                (progn$ (?p (split-delim ?prem-str ","))
                    (bind ?premises (create$ ?premises (sym-cat ?p))))
                (bind ?conclusion (sym-cat (nth$ 3 ?parts)))
                (bind ?explanation (nth$ 4 ?parts))
                (bind ?cf 1.0)
                (if (>= (length$ ?parts) 5)
                    then (bind ?cf (string-to-field (nth$ 5 ?parts))))
                (assert (production-rule
                    (id ?id) (premises ?premises)
                    (conclusion ?conclusion) (cf ?cf)
                    (explanation ?explanation))))
        (bind ?line (readline ?log)))
    (close ?log)
    (return TRUE))


(deffunction load-target (?file)
    (bind ?log (sym-cat "target-" (gensym*)))
    (if (eq (open ?file ?log "r") FALSE)
        then (return FALSE))
    (bind ?line (readline ?log))
    (bind ?done FALSE)
    (while (and (neq ?line EOF) (not ?done))
        (if (and (> (str-length ?line) 0)
                 (neq (sub-string 1 1 ?line) "#"))
            then
                (assert (target-fact (id (sym-cat ?line))))
                (bind ?done TRUE))
        (bind ?line (readline ?log))
    )
    (close ?log)
    (return ?done)
)

; ==> Прямой вывод
(deffunction run-forward ()
    (bind ?changed TRUE)
    (bind ?steps 0)
    (while ?changed
        (bind ?changed FALSE)
        (do-for-all-facts ((?r production-rule)) TRUE
            (bind ?rcf (compute-rule-cf ?r))
            (if (> ?rcf 0.0)
                then
                    (bind ?old-cf (get-contribution-cf ?r:id ?r:conclusion))
                    (if (> ?rcf ?old-cf)
                        then
                            (bind ?total-cf
                                (update-contribution ?r:id ?r:conclusion ?rcf))
                            (set-known-fact ?r:conclusion ?total-cf derived)
                            (assert (inference-step
                                (rule-id ?r:id)
                                (conclusion ?r:conclusion)
                                (cf ?rcf)
                                (premises ?r:premises)))
                            (bind ?steps (+ ?steps 1))
                            (format t "Step %d: %s -> %s, rule CF=%.4f, total CF=%.4f%n"
                                ?steps ?r:id ?r:conclusion ?rcf ?total-cf)
                            (bind ?changed TRUE)
                    )
            )
        )
    )
    (return ?steps)
)

(deffunction print-result ()
    (bind ?target FALSE)
    (do-for-all-facts ((?t target-fact)) TRUE
        (bind ?target ?t:id))
    (if (eq ?target FALSE)
        then (printout t "Target fact is not loaded." crlf) (return))
    (bind ?cf (fact-cf ?target))
    (format t "%nTarget: %s%n" ?target)
    (format t "CF = %.4f%n" ?cf)
    (if (> ?cf 0.0)
        then (format t "Conclusion: %s is confirmed with CF %.4f%n"
                     ?target ?cf)
        else (format t "Conclusion: %s is not confirmed%n" ?target))
    (format t "%nAxioms:%n")
    (do-for-all-facts ((?f known-fact))
        (eq ?f:origin axiom)
        (format t "%s;%.2f%n" ?f:id ?f:cf))
)
