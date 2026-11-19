;========================================================================
; Механизм подсчета CF

; CF правила = CF_правила × минимум из CF посылок
(deffunction calculate-rule-cf (?rule-cf $?premise-cfs)
    (if (= (length$ ?premise-cfs) 0)
        then (return 0.0))
    (bind ?min-cf (nth$ 1 ?premise-cfs)) ; минимуму присваем первый элемент списка
    (progn$ (?cf ?premise-cfs)
        (if (< ?cf ?min-cf)
            then (bind ?min-cf ?cf)))
    (return (* ?rule-cf ?min-cf))
)

; Объединение двух положительных подтверждений:
; CF = a + b * (1 - a)
(deffunction combine-cf (?a ?b)
    (return (+ ?a (* ?b (- 1.0 ?a))))
)

; Итоговая уверенность факта
(deffunction calculate-total-cf (?fact-id)
    (bind ?result 0.0)

    (do-for-all-facts ((?contribution cf-contribution))
        (eq ?contribution:conclusion ?fact-id) ; eq для сравнения символов, для чисел =
        (bind ?result (combine-cf ?result ?contribution:cf))
    )

    (return ?result)
)
; ну вот например примеры, примеры например
; (cf-contribution (rule-id R1) (conclusion X) (cf 0.6))
; (cf-contribution (rule-id R2) (conclusion X) (cf 0.5))
; (cf-contribution (rule-id R3) (conclusion Y) (cf 0.9))
; 0.6 + 0.5·0.4

; Регистрация вклада правила
(deffunction update-contribution (?rule-id ?fact-id ?new-cf)
    (bind ?found FALSE)
    (do-for-all-facts ((?contribution cf-contribution))
        (and (eq ?contribution:rule-id ?rule-id)
             (eq ?contribution:conclusion ?fact-id))
        (bind ?found TRUE)
        (if (> ?new-cf ?contribution:cf)
            then (modify ?contribution (cf ?new-cf)))
    )
    (if (not ?found)
        then(assert (cf-contribution ; assert - добавление нового факта
                (rule-id ?rule-id)
                (conclusion ?fact-id)
                (cf ?new-cf)))) 
    (return (calculate-total-cf ?fact-id))
)