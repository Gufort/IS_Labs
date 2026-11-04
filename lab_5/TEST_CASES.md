# Варианты стартовых аксиом для тестирования

Все приведённые варианты проверены обратным выводом на основной автомобильной базе.  
Глубина целевого факта в каждом случае не меньше четырёх.

Для проверки варианта замените содержимое `axioms.txt` и `target.txt` указанными блоками, затем запустите пункт **2 — обратный вывод для target.txt**.

Важно: не добавляйте в `axioms.txt` промежуточные факты из цепочки. Если промежуточный факт сразу объявить аксиомой, глубина доказательства уменьшится.

## Замена аккумулятора — основная цепочка

Ожидаемая глубина: **5**.

`axioms.txt`:

```text
engine_wont_start
starter_clicks
headlights_dim
terminals_corroded
jump_start_works
battery_old
```

`target.txt`:

```text
recommend_replace_battery
```

Ожидаемая цепочка:

```text
R001 → R002 → R003 → R004 → R005
```

## Неисправность генератора из-за ослабленного ремня

Ожидаемая глубина: **8**.

`axioms.txt`:

```text
alternator_lamp_on
voltage_drops_running
belt_loose
engine_wont_start
headlights_dim
terminals_corroded
jump_start_works
battery_old
```

`target.txt`:

```text
recommend_replace_battery
```

Ожидаемая цепочка:

```text
R014 → R015 → R018 → R019 → R002 → R003 → R004 → R005
```

## Неисправность генератора из-за оборванного ремня

Ожидаемая глубина: **8**.

`axioms.txt`:

```text
alternator_lamp_on
voltage_drops_running
belt_broken
engine_wont_start
headlights_dim
terminals_corroded
jump_start_works
battery_old
```

`target.txt`:

```text
recommend_replace_battery
```

Ожидаемая цепочка:

```text
R014 → R016 → R018 → R019 → R002 → R003 → R004 → R005
```

## Неисправность ДМРВ через богатую смесь

Ожидаемая глубина: **5**.

`axioms.txt`:

```text
fuel_smell
engine_wont_start
plugs_fouled
misfire
fuel_consumption_high
maf_sensor_code
```

`target.txt`:

```text
maf_fault
```

Ожидаемая цепочка:

```text
R029 → R032 → R046 → R033 → R034
```

## Неисправность ДМРВ через систему зажигания

Ожидаемая глубина: **5**.

`axioms.txt`:

```text
spark_weak
misfire
plugs_fouled
fuel_consumption_high
maf_sensor_code
```

`target.txt`:

```text
maf_fault
```

Ожидаемая цепочка:

```text
R042 → R044 → R046 → R033 → R034
```

В этом варианте поиск дополнительно обнаруживает циклическую альтернативу `mixture_rich → maf_fault → ... → mixture_rich`, отбрасывает её и успешно использует ветку через `ignition_suspect`.

## Краткая таблица

| Вариант | Цель | Глубина | Продукций в доказательстве |
|---|---|---:|---:|
| Основная цепочка аккумулятора | `recommend_replace_battery` | 5 | 5 |
| Генератор, ослабленный ремень | `recommend_replace_battery` | 8 | 8 |
| Генератор, оборванный ремень | `recommend_replace_battery` | 8 | 8 |
| ДМРВ через богатую смесь | `maf_fault` | 5 | 5 |
| ДМРВ через зажигание | `maf_fault` | 5 | 5 |

## Дополнительные проверенные варианты

Ниже приведены остальные содержательно отличающиеся длинные маршруты. Формат каждой строки:

```text
цель | ожидаемая глубина | стартовые аксиомы | выбранные правила
```

### Аккумулятор и электрическая система

```text
recommend_replace_battery | 7
alternator_lamp_on, voltage_drops_running, belt_loose, engine_wont_start, headlights_dim, jump_start_works, battery_old
R014 → R015 → R018 → R019 → R002 → R008 → R005
```

```text
recommend_replace_battery | 4
engine_cranks_slowly, headlights_dim, terminals_corroded, jump_start_works, battery_old
R006 → R003 → R004 → R005
```

```text
recommend_replace_battery | 4
battery_voltage_low_measured, engine_wont_start, terminals_corroded, jump_start_works, battery_old
R007 → R003 → R004 → R005
```

```text
recommend_replace_battery | 4
engine_wont_start, starter_clicks, headlights_dim, jump_start_works, battery_old
R001 → R002 → R008 → R005
```

```text
starting_system_suspect | 4
alternator_lamp_on, voltage_drops_running, belt_loose, engine_wont_start
R014 → R015 → R018 → R019
```

```text
battery_charge_low | 5
alternator_lamp_on, voltage_drops_running, belt_loose, engine_wont_start, headlights_dim
R014 → R015 → R018 → R019 → R002
```

```text
battery_connection_fault | 6
alternator_lamp_on, voltage_drops_running, belt_loose, engine_wont_start, headlights_dim, terminals_corroded
R014 → R015 → R018 → R019 → R002 → R003
```

```text
battery_fault_confirmed | 7
alternator_lamp_on, voltage_drops_running, belt_loose, engine_wont_start, headlights_dim, terminals_corroded, jump_start_works
R014 → R015 → R018 → R019 → R002 → R003 → R004
```

Во всех вариантах с `belt_loose` можно получить отдельную равнозначную вариацию:

```text
belt_loose  заменить на  belt_broken
R015        заменить на  R016
```

Глубина при такой замене не изменяется.

### Зажигание, впуск и топливная система

```text
air_intake_fault | 4
engine_wont_start, idle_rough, misfire, spark_absent
R041 → R043 → R047 → R052
```

```text
maf_fault | 5
engine_wont_start, idle_rough, misfire, spark_absent, maf_sensor_code
R041 → R043 → R047 → R052 → R034
```

```text
fuel_filter_clogged | 4
engine_wont_start, fuel_pressure_low, misfire, spark_absent
R041 → R043 → R047 → R026
```

```text
do_not_drive | 4
alternator_lamp_on, voltage_drops_running, engine_wont_start, misfire, spark_absent
R014 → R041 → R043 → R047 → R020
```

У предыдущих четырёх вариантов существуют ещё два способа получить `ignition_suspect`.

Вариант через слабую искру:

```text
engine_wont_start заменить на spark_weak
R041 заменить на R042
```

Вариант через индикатор неисправности двигателя:

```text
engine_wont_start заменить на check_engine_on
R041 заменить на R050
```

Факты `misfire` и `spark_absent` при этих заменах остаются в наборе. Глубина результата не изменяется.

### Сервисные решения

```text
urgent_service | 6
alternator_lamp_on, voltage_drops_running, belt_loose, engine_wont_start, headlights_dim
R014 → R015 → R018 → R019 → R002 → R017
```

```text
power_low | 4
check_engine_on, oxygen_sensor_code, fuel_consumption_high, plugs_fouled, misfire
R036 → R037 → R032 → R046
```

Для `urgent_service` также допустима замена `belt_loose`/`R015` на `belt_broken`/`R016` без изменения глубины.

## Сводка охвата

В файле перечислено **19 непосредственно проверенных наборов**, если учитывать пять основных и четырнадцать дополнительных. После раскрытия равнозначных замен ремня и трёх вариантов получения `ignition_suspect` получается более тридцати комбинаций стартовых аксиом.

Автоматический анализ базы находит ещё десятки комбинаций, но большинство из них отличаются добавлением избыточной аксиомы или перестановкой симптомов и приводят к той же последовательности продукций. Такие дубликаты здесь намеренно не перечислены.
