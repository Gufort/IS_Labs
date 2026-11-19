;========================================================================
; Этот блок связан определением моделей

; База знаний - что известно и с какой вероятностью
(deftemplate known-fact
    (slot id 
        (type SYMBOL))

    (slot cf 
        (type NUMBER) 
        (default 0.0))

    (slot origin 
        (type SYMBOL) 
        (allowed-values axiom derived) 
        (default derived))
)    

; Продукционное правило
; в premises хранится произвольное количество посылок
(deftemplate production-rule
    (slot id
        (type SYMBOL))

    (multislot premises
        (type SYMBOL))

    (slot conclusion
        (type SYMBOL))

    (slot cf
        (type NUMBER)
        (default 1.0))

    (slot explanation
        (type STRING)
        (default ""))
)

; Вклад конкретного правила в уверенность заключения.
; Пара rule-id + conclusion должна быть уникальной.
(deftemplate cf-contribution
    (slot rule-id
        (type SYMBOL))
    
    (slot conclusion
        (type SYMBOL))

    (slot cf
        (type NUMBER))
)

; Целевой факт, который требуется получить.
(deftemplate target-fact
    (slot id
        (type SYMBOL))
)

; Информация о выполненном шаге вывода.
(deftemplate inference-step
    (slot rule-id
        (type SYMBOL))

    (slot conclusion
        (type SYMBOL))

    (slot cf
        (type NUMBER))

    (multislot premises
        (type SYMBOL))
)