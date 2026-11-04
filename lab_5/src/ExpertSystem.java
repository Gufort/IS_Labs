import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.*;

public class ExpertSystem {
    static class FactDefinition{
        String name;
        String description;
        double defaultCf = 1.0;
    }

    static class RuleDefinition{
        String id;
        List<String> premises = new ArrayList<>(); // Список посылок, левая часть
        String conclusion; // id-шник факта заключения
        String explanation; // текстовое объяснение для печати
        double cf = 1.0; // уверенность правила
    }

    static class PremiseCF {
        final String id;
        final double cf;
        PremiseCF(String id, double cf) { this.id = id; this.cf = cf; } // ID посылки, её CF
    }

    static class Derivation {
        String factId; // какой факт выведен
        double cf = 0; // итоговая уверенность
        String ruleId; // правило, которым факт выведен
        RuleDefinition rule; // rule == null ==> это аксиома, а не вывод
        List<PremiseCF> premises = new ArrayList<>(); // снимок посылок
        String explanation; // текст объяснения
        int depth = 0; // 0 для аксиом, max(depth посылок) + 1 для выведенных
    }

    // фиксирует каждое срабатывание правила для печати списка сработавших продукций
    static class FiredRule{
        RuleDefinition rule;
        double conclusionCf; // conclusionCf = rule.cf * premiseCf
        double premiseCf;
        List<PremiseCF> premises;
    }

    // узел дерева доказательства
    static class ProofNode {
        String factId;
        double cf = 0;
        RuleDefinition rule; // rule == null — лист-аксиома
        List<ProofNode> premises = new ArrayList<>();
        String explanation;
        int depth = 0;
    }


    // Вспомогательные функции
    static String trim(String str){
        return str == null ? "" : str.trim();
    }

    static List<String> split(String str, char d){
        var result = new ArrayList<String>();
        int start = 0;

        for(int i = 0; i < str.length(); i++){
            if(str.charAt(i) == d){
                result.add(str.substring(start, i));
                start = i + 1;
            }
        }

        result.add(str.substring(start));
        return result;
    }

    static double combineCf(double a, double b) {
        if (Math.abs(a) < 1e-12) return b;
        if (Math.abs(b) < 1e-12) return a;
        if (a > 0 && b > 0) return a + b * (1.0 - a);
        if (a < 0 && b < 0) return a + b * (1.0 + a);
        return (a + b) / (1.0 - Math.min(Math.abs(a), Math.abs(b)));
    }

    // Загрузка данных
    static List<String> dataLines(String path) throws Exception{
        var result = new ArrayList<String>();
        if(!Files.exists(Paths.get(path))) return result;

        for (String raw : Files.readAllLines(Paths.get(path), StandardCharsets.UTF_8)) {
            String line = trim(raw);
            if (line.isEmpty() || line.charAt(0) == '#') continue;
            result.add(line);
        }

        return result;
    }

    static Map<String, FactDefinition> loadFacts(String path) throws Exception {
        Map<String, FactDefinition> res = new LinkedHashMap<>();
        for (String line : dataLines(path)) {
            List<String> parts = split(line, ';');
            if (parts.size() < 2) continue;
            var fd = new FactDefinition();
            fd.name = trim(parts.get(0));
            fd.description = trim(parts.get(1));
            if (parts.size() >= 3 && !trim(parts.get(2)).isEmpty())
                fd.defaultCf = Double.parseDouble(trim(parts.get(2)));
            res.put(fd.name, fd);
        }
        return res;
    }

    static Map<String, RuleDefinition> loadRules(String path) throws Exception {
        Map<String, RuleDefinition> res = new LinkedHashMap<>();
        for (String line : dataLines(path)) {
            List<String> parts = split(line, ';');
            if (parts.size() < 4) continue;
            var rd = new RuleDefinition();
            rd.id = trim(parts.get(0));
            for (String p : split(trim(parts.get(1)), ',')) {
                String tp = trim(p);
                if (!tp.isEmpty()) rd.premises.add(tp);
            }
            rd.conclusion = trim(parts.get(2));
            rd.explanation = trim(parts.get(3));
            if (parts.size() >= 5 && !trim(parts.get(4)).isEmpty())
                rd.cf = Double.parseDouble(trim(parts.get(4)));
            res.put(rd.id, rd);
        }
        return res;
    }

    static Map<String, Double> loadAxioms(String path, Map<String, FactDefinition> facts) throws Exception {
        Map<String, Double> res = new LinkedHashMap<>();
        for (String line : dataLines(path)) {
            List<String> parts = split(line, ';');
            if (parts.isEmpty()) continue;
            String id = trim(parts.get(0));
            double cf = 1.0;
            FactDefinition fd = facts.get(id);
            if (fd != null) cf = fd.defaultCf;
            if (parts.size() >= 2 && !trim(parts.get(1)).isEmpty())
                cf = Double.parseDouble(trim(parts.get(1)));
            res.put(id, cf);
        }
        return res;
    }

    static String loadTarget(String path) throws Exception {
        List<String> lines = dataLines(path);
        return lines.isEmpty() ? "" : lines.get(0);
    }

    static class ForwardResult {
        Map<String, Double> known = new HashMap<>(); // множество известных фактов
        Map<String, Derivation> derivations = new HashMap<>(); // лучшая деривация каждого факта
        List<FiredRule> fired = new ArrayList<>(); // журнал срабатываний
    }

    // Прямой вывод
    static ForwardResult forward(Map<String, Double> axioms, Map<String, RuleDefinition> rules) {
        var result = new ForwardResult();

        // Аксиомы сразу известны
        for (Map.Entry<String, Double> e : axioms.entrySet()) {
            result.known.put(e.getKey(), e.getValue());

            Derivation der = new Derivation();
            der.factId = e.getKey();
            der.cf = e.getValue();
            der.explanation = "Исходный факт";
            der.depth = 0;
            result.derivations.put(e.getKey(), der);
        }

        // Порядок срабатывания правил напрямую влияет на итоговый CF
        List<RuleDefinition> sorted = new ArrayList<>(rules.values());
        sorted.sort(Comparator.comparing((RuleDefinition r) -> r.id));

        Map<String, String> lastSig = new HashMap<>();
        int guard = 0;
        boolean changed = true;

        // Итерация до неподвижной точки: крутимся, пока хоть одно правило
        while (changed && guard++ < 10000) {
            changed = false;

            // Проверка готовности правила.
            // Срабатывает только в случае, когда все посылки известны
            for (RuleDefinition rule : sorted) {
                boolean ready = true;
                for (String p : rule.premises)
                    if (!result.known.containsKey(p)) {
                        ready = false;
                        break;
                    }
                if (!ready) continue;

                double premiseCf = 1.0;
                // Уверенность самого слабого звена
                for (String p : rule.premises)
                    premiseCf = Math.min(premiseCf, result.known.get(p));

                // Защита от расходимости
                StringBuilder strB = new StringBuilder();
                for (String p : rule.premises)
                    strB.append(String.format(Locale.ROOT, "%.6f|", result.known.get(p)));
                String sigStr = strB.toString();

                if (sigStr.equals(lastSig.get(rule.id))) continue;
                lastSig.put(rule.id, sigStr); // Запоминаем новую сигнатуру

                // Применение правила
                double conclusionCf = rule.cf * premiseCf;
                if (conclusionCf <= 1e-9) continue;

                Double oldCfObj = result.known.get(rule.conclusion);
                double oldCf = oldCfObj == null ? 0.0 : oldCfObj;
                double combined = combineCf(oldCf, conclusionCf);

                if (oldCfObj == null || combined > oldCf + 1e-9) {
                    result.known.put(rule.conclusion, combined);

                    int depth = 1;
                    if (!rule.premises.isEmpty()) {
                        depth = 0;
                        // depth = max(depth посылок) + 1
                        for (String p : rule.premises) {
                            Derivation pd = result.derivations.get(p);
                            if (pd != null) depth = Math.max(depth, pd.depth);
                        }
                        depth += 1;
                    }

                    // перезапись
                    Derivation d = new Derivation();
                    d.factId = rule.conclusion;
                    d.cf = combined;
                    d.ruleId = rule.id;
                    d.rule = rule;
                    for (String p : rule.premises)
                        d.premises.add(new PremiseCF(p, result.known.get(p)));
                    d.explanation = rule.explanation;
                    d.depth = depth;
                    result.derivations.put(rule.conclusion, d);

                    // для печати сработавших продукции
                    FiredRule fr = new FiredRule();
                    fr.rule = rule;
                    fr.conclusionCf = combined;
                    fr.premiseCf = premiseCf;
                    fr.premises = d.premises;
                    result.fired.add(fr);

                    changed = true;
                }
            }
        }

        return result;
    }


    // Обратный вывод
    enum BState { Enter, TryRule, TryPremise, AfterPremise }

    static class BFrame {
        String factId;
        BState state = BState.Enter;
        List<RuleDefinition> rules = new ArrayList<>(); // альтернативы (дети ИЛИ-узла)
        int nextRule = 0; // индекс следующей альтернативы
        RuleDefinition currentRule;
        int premiseIdx = 0; // какую посылку currentRule доказываем
        List<ProofNode> premises = new ArrayList<>(); // уже доказанные посылки
        ProofNode childResult; // результат последней подзадачи

        BFrame(String factId) { this.factId = factId; }
    }

    static class BackwardSolver{
        final Map<String, Double> axioms;
        final Map<String, List<RuleDefinition>> byConclusion = new HashMap<>(); // factId → правила, выводящие его. Сам граф
        final Map<String, ProofNode> memo = new HashMap<>(); // кэш успехов
        final Set<String> path = new HashSet<>(); // факты в текущей ветке
        final List<BFrame> stack = new ArrayList<>();
        final List<ProofNode> pool = new ArrayList<>(); // держит ссылки на узлы
        ProofNode root;

        BackwardSolver(Map<String, Double> axioms, Map<String, RuleDefinition> rules) {
            this.axioms = axioms;
            for (RuleDefinition r : rules.values())
                byConclusion.computeIfAbsent(r.conclusion, k -> new ArrayList<>()).add(r);
            for (List<RuleDefinition> rs : byConclusion.values())
                rs.sort(Comparator.comparing((RuleDefinition r) -> r.id));
        }

        void deliver(ProofNode node) {
            if (stack.isEmpty()) root = node;
            else stack.get(stack.size() - 1).childResult = node;
        }

        ProofNode solve(String target) {
            stack.add(new BFrame(target));

            while (!stack.isEmpty()) {
                BFrame f = stack.get(stack.size() - 1);
                switch (f.state) {
                    // Вход в ИЛИ-узел
                    case Enter: {
                        Double ax = axioms.get(f.factId);
                        if (ax != null) {
                            ProofNode node = new ProofNode();
                            node.factId = f.factId;
                            node.cf = ax;
                            node.explanation = "Исходный факт";
                            node.depth = 0;
                            pool.add(node);

                            stack.remove(stack.size() - 1);
                            deliver(node);
                            continue;
                        }

                        if (path.contains(f.factId)) {
                            stack.remove(stack.size() - 1);
                            deliver(null);
                            continue;
                        }

                        ProofNode cached = memo.get(f.factId);
                        if (cached != null) {
                            stack.remove(stack.size() - 1);
                            deliver(cached);
                            continue;
                        }

                        path.add(f.factId);

                        List<RuleDefinition> rs = byConclusion.get(f.factId);
                        if (rs != null) f.rules = rs;
                        f.nextRule = 0;
                        f.state = BState.TryRule;
                        break;
                    }
                    //  Перебор альтернатив ИЛИ-узла
                    case TryRule:{
                        // Альтернативы закончились => факт недоказуем
                        if (f.nextRule >= f.rules.size()) {
                            path.remove(f.factId);
                            stack.remove(stack.size() - 1);
                            deliver(null);
                        }
                        // Иначе следующее правило, прогресс обнуляется
                        else {
                            f.currentRule = f.rules.get(f.nextRule++);
                            f.premiseIdx = 0;
                            f.premises.clear();
                            f.state = BState.TryPremise;
                        }
                        break;
                    }
                    // перебор детей И-узла
                    case TryPremise:{
                        if (f.premiseIdx >= f.currentRule.premises.size()) {
                            double premiseCf = 1.0;
                            for (ProofNode pn : f.premises)
                                premiseCf = Math.min(premiseCf, pn.cf);
                            double cf = f.currentRule.cf * premiseCf;

                            int depth = 1;
                            if (!f.premises.isEmpty()) {
                                depth = 0;
                                for (ProofNode pn : f.premises)
                                    depth = Math.max(depth, pn.depth);
                                depth += 1;
                            }

                            ProofNode node = new ProofNode();
                            node.factId = f.factId;
                            node.cf = cf;
                            node.rule = f.currentRule;
                            node.premises = new ArrayList<>(f.premises);
                            node.explanation = f.currentRule.explanation;
                            node.depth = depth;
                            pool.add(node);

                            memo.put(f.factId, node);
                            path.remove(f.factId);
                            stack.remove(stack.size() - 1);
                            deliver(node);
                        }
                        // Есть ещё посылки - переходим к следующей
                        else {
                            String premise = f.currentRule.premises.get(f.premiseIdx);
                            f.state = BState.AfterPremise;
                            stack.add(new BFrame(premise));
                        }
                        break;
                    }
                    // обработка возврата
                    case AfterPremise: {
                        ProofNode child = f.childResult;
                        f.childResult = null;
                        if (child != null) {
                            f.premises.add(child);
                            f.premiseIdx++;
                            f.state = BState.TryPremise;
                        } else {
                            f.state = BState.TryRule;
                        }
                        break;
                    }
                }
            }
            return root;
        }
    }

    static String fmtPrems(List<PremiseCF> prems) {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < prems.size(); i++) {
            if (i > 0) sb.append(" & ");
            sb.append(String.format(Locale.ROOT, "%s (%.4f)",
                    prems.get(i).id, prems.get(i).cf));
        }
        return sb.toString();
    }

    static void printForwardProof(String target, ForwardResult fr) {
        class Item {
            final String id; final boolean visited;
            Item(String i, boolean v) { id = i; visited = v; }
        }

        List<Item> stack = new ArrayList<>();
        Set<String> printed = new HashSet<>();
        stack.add(new Item(target, false));

        while (!stack.isEmpty()) {
            Item it = stack.remove(stack.size() - 1);
            String id = it.id;
            boolean visited = it.visited;

            if (!visited) {
                Derivation d = fr.derivations.get(id);
                if (d == null) continue;
                stack.add(new Item(id, true));
                if (d.rule != null) {
                    for (int i = d.premises.size() - 1; i >= 0; i--)
                        stack.add(new Item(d.premises.get(i).id, false));
                }
            } else {
                if (!printed.add(id)) continue;
                Derivation d = fr.derivations.get(id);
                if (d.rule == null) {
                    System.out.printf(Locale.ROOT, "  %s = %.4f — исходный факт%n", id, d.cf);
                } else {
                    System.out.printf(Locale.ROOT,
                            "  %s: %s => %s; CF=%.4f; %s%n",
                            d.ruleId, fmtPrems(d.premises), id, d.cf, d.explanation);
                }
            }
        }
    }

    static void printForward(ForwardResult fr, String target) {
        System.out.println("Сработавшие продукции:");
        if (fr.fired.isEmpty()) System.out.println("  (нет)");
        for (FiredRule f : fr.fired) {
            System.out.printf(Locale.ROOT,
                    "  %s: %s => %s; CF=%.4f (CF правила %.2f); %s%n",
                    f.rule.id, fmtPrems(f.premises), f.rule.conclusion,
                    f.conclusionCf, f.rule.cf, f.rule.explanation);
        }

        System.out.println();
        Double cf = fr.known.get(target);
        if (cf == null) {
            System.out.println("Цель " + target + ": не выведена");
        } else {
            System.out.printf(Locale.ROOT, "Цель %s: CF=%.4f%n", target, cf);
            System.out.println("Объяснение цели, посылки перед заключением:");
            printForwardProof(target, fr);
        }
    }

    static void printBackward(ProofNode root) {
        System.out.printf(Locale.ROOT,
                "Цель выводится: %s, CF=%.4f, глубина=%d%n",
                root.factId, root.cf, root.depth);
        System.out.println("Участвующие продукции и исходные факты, посылки перед заключением:");

        class Item {
            final ProofNode node; final boolean visited;
            Item(ProofNode n, boolean v) { node = n; visited = v; }
        }

        List<Item> stack = new ArrayList<>();
        Set<String> printedInitial = new HashSet<>();
        stack.add(new Item(root, false));

        while (!stack.isEmpty()) {
            Item it = stack.remove(stack.size() - 1);
            ProofNode node = it.node;
            boolean visited = it.visited;

            if (!visited) {
                stack.add(new Item(node, true));
                for (int i = node.premises.size() - 1; i >= 0; i--)
                    stack.add(new Item(node.premises.get(i), false));
            } else {
                if (node.rule == null) {
                    if (printedInitial.add(node.factId)) {
                        System.out.printf(Locale.ROOT,
                                "  %s = %.4f — исходный факт%n", node.factId, node.cf);
                    }
                } else {
                    StringBuilder prems = new StringBuilder();
                    for (int i = 0; i < node.premises.size(); i++) {
                        if (i > 0) prems.append(" & ");
                        ProofNode pn = node.premises.get(i);
                        prems.append(String.format(Locale.ROOT,
                                "%s (%.4f)", pn.factId, pn.cf));
                    }
                    System.out.printf(Locale.ROOT,
                            "  %s: %s => %s; CF=%.4f; %s%n",
                            node.rule.id, prems, node.factId, node.cf, node.explanation);
                }
            }
        }
    }

    public static void main(String[] args) throws Exception {
        String dir = args.length > 0 ? args[0] : ".";
        File base = new File(dir);

        Map<String, FactDefinition> facts = loadFacts(new File(base, "facts.txt").getPath());
        Map<String, RuleDefinition> rules = loadRules(new File(base, "rules.txt").getPath());
        Map<String, Double> axioms = loadAxioms(new File(base, "axioms.txt").getPath(), facts);
        String target = loadTarget(new File(base, "target.txt").getPath());

        if (facts.isEmpty() || rules.isEmpty()) {
            System.err.println("Ошибка загрузки facts.txt/rules.txt (dir=" + dir + ")");
            System.exit(1);
        }
        if (target.isEmpty()) {
            System.err.println("Ошибка: пустой target.txt");
            System.exit(1);
        }
        if (!facts.containsKey(target)) {
            System.err.println("Цель " + target + " отсутствует в facts.txt");
            System.exit(1);
        }

        System.out.println("=== Проверка входных данных ===");
        System.out.printf(Locale.ROOT,
                "Фактов: %d; правил: %d; исходных фактов: %d; цель: %s%n%n",
                facts.size(), rules.size(), axioms.size(), target);

        System.out.println("=== ПРЯМОЙ ВЫВОД ===");
        ForwardResult fwd = forward(axioms, rules);
        printForward(fwd, target);

        System.out.println();
        System.out.println("=== ОБРАТНЫЙ ВЫВОД ===");
        BackwardSolver solver = new BackwardSolver(axioms, rules);
        ProofNode proof = solver.solve(target);
        if (proof == null) System.out.println("Цель не выводится.");
        else printBackward(proof);
    }
}