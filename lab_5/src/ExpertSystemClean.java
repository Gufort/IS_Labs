import java.io.File;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.*;

/**
 * Чистая реализация продукционной ЭС без эвристик.
 * Прямой:    CF(f) = combineCf(ax(f), все вклады сработавших правил) — итерации до неподвижной точки.
 * Обратный:  CF(f) = combineCf(ax(f), вклады ВСЕХ правил, чьи посылки доказаны);
 *            цикл в текущей ветке даёт вклад 0 (well-founded).
 * Никаких "только улучшение", "CF>1e-9", "первое успешное правило", memo.
 */
public class ExpertSystemClean {

    // ==== Модели ====
    static class FactDefinition {
        String name;
        String description;
        double defaultCf = 1.0;
    }

    static class RuleDefinition {
        String id;
        List<String> premises = new ArrayList<>();
        String conclusion;
        String explanation;
        double cf = 1.0;
    }

    static class PremiseCF {
        final String id;
        final double cf;
        PremiseCF(String id, double cf) { this.id = id; this.cf = cf; }
    }

    static class Derivation {
        String factId;
        double cf = 0;
        String ruleId;
        RuleDefinition rule;
        List<PremiseCF> premises = new ArrayList<>();
        String explanation;
        int depth = 0;
    }

    static class FiredRule {
        RuleDefinition rule;
        double conclusionCf;
        double premiseCf;
        List<PremiseCF> premises;
    }

    static class ProofNode {
        String factId;
        double cf = 0;
        RuleDefinition rule;
        List<ProofNode> premises = new ArrayList<>();
        String explanation;
        int depth = 0;
    }

    static class ForwardResult {
        Map<String, Double> known = new HashMap<>();
        Map<String, Derivation> derivations = new HashMap<>();
        List<FiredRule> fired = new ArrayList<>();
    }

    // ==== Утилиты ====
    static String trim(String s) { return s == null ? "" : s.trim(); }

    static List<String> split(String s, char d) {
        List<String> r = new ArrayList<>();
        int start = 0;
        for (int i = 0; i < s.length(); i++) {
            if (s.charAt(i) == d) { r.add(s.substring(start, i)); start = i + 1; }
        }
        r.add(s.substring(start));
        return r;
    }

    static double combineCf(double a, double b) {
        if (Math.abs(a) < 1e-12) return b;
        if (Math.abs(b) < 1e-12) return a;
        if (a > 0 && b > 0) return a + b * (1.0 - a);
        if (a < 0 && b < 0) return a + b * (1.0 + a);
        return (a + b) / (1.0 - Math.min(Math.abs(a), Math.abs(b)));
    }

    // ==== Загрузка ====
    static List<String> dataLines(String path) throws Exception {
        List<String> r = new ArrayList<>();
        if (!Files.exists(Paths.get(path))) return r;
        for (String raw : Files.readAllLines(Paths.get(path), StandardCharsets.UTF_8)) {
            String line = trim(raw);
            if (line.isEmpty() || line.charAt(0) == '#') continue;
            r.add(line);
        }
        return r;
    }

    static Map<String, FactDefinition> loadFacts(String path) throws Exception {
        Map<String, FactDefinition> r = new LinkedHashMap<>();
        for (String line : dataLines(path)) {
            List<String> p = split(line, ';');
            if (p.size() < 2) continue;
            FactDefinition fd = new FactDefinition();
            fd.name = trim(p.get(0));
            fd.description = trim(p.get(1));
            if (p.size() >= 3 && !trim(p.get(2)).isEmpty())
                fd.defaultCf = Double.parseDouble(trim(p.get(2)));
            r.put(fd.name, fd);
        }
        return r;
    }

    static Map<String, RuleDefinition> loadRules(String path) throws Exception {
        Map<String, RuleDefinition> r = new LinkedHashMap<>();
        for (String line : dataLines(path)) {
            List<String> p = split(line, ';');
            if (p.size() < 4) continue;
            RuleDefinition rd = new RuleDefinition();
            rd.id = trim(p.get(0));
            for (String pr : split(trim(p.get(1)), ',')) {
                String t = trim(pr);
                if (!t.isEmpty()) rd.premises.add(t);
            }
            rd.conclusion = trim(p.get(2));
            rd.explanation = trim(p.get(3));
            if (p.size() >= 5 && !trim(p.get(4)).isEmpty())
                rd.cf = Double.parseDouble(trim(p.get(4)));
            r.put(rd.id, rd);
        }
        return r;
    }

    static Map<String, Double> loadAxioms(String path,
                                          Map<String, FactDefinition> facts) throws Exception {
        Map<String, Double> r = new LinkedHashMap<>();
        for (String line : dataLines(path)) {
            List<String> p = split(line, ';');
            if (p.isEmpty()) continue;
            String id = trim(p.get(0));
            double cf = 1.0;
            FactDefinition fd = facts.get(id);
            if (fd != null) cf = fd.defaultCf;
            if (p.size() >= 2 && !trim(p.get(1)).isEmpty())
                cf = Double.parseDouble(trim(p.get(1)));
            r.put(id, cf);
        }
        return r;
    }

    static String loadTarget(String path) throws Exception {
        List<String> lines = dataLines(path);
        return lines.isEmpty() ? "" : lines.get(0);
    }

    // ============================================================
    // ЧИСТЫЙ ПРЯМОЙ ВЫВОД
    // ============================================================
    static ForwardResult forwardClean(Map<String, Double> axioms,
                                      Map<String, RuleDefinition> rules) {
        ForwardResult result = new ForwardResult();

        List<RuleDefinition> sorted = new ArrayList<>(rules.values());
        sorted.sort(Comparator.comparing(r -> r.id));

        // Начальное состояние
        Map<String, Double> prev = new HashMap<>(axioms);
        Map<String, Derivation> prevDeriv = new HashMap<>();
        for (var e : axioms.entrySet()) {
            Derivation d = new Derivation();
            d.factId = e.getKey();
            d.cf = e.getValue();
            d.explanation = "Исходный факт";
            d.depth = 0;
            prevDeriv.put(e.getKey(), d);
        }

        final int maxIter = 10000;
        boolean stable = false;
        for (int iter = 0; iter < maxIter && !stable; iter++) {
            Map<String, Double> next = new HashMap<>();
            Map<String, Derivation> nextDeriv = new HashMap<>();

            // Аксиомы — база для next
            for (var e : axioms.entrySet()) {
                next.put(e.getKey(), e.getValue());
                Derivation d = new Derivation();
                d.factId = e.getKey();
                d.cf = e.getValue();
                d.explanation = "Исходный факт";
                d.depth = 0;
                nextDeriv.put(e.getKey(), d);
            }

            // Вклады правил по prev (Kleene-итерация: только прошлые значения)
            for (RuleDefinition r : sorted) {
                boolean ready = true;
                double minCf = 1.0;
                for (String p : r.premises) {
                    Double cf = prev.get(p);
                    if (cf == null) { ready = false; break; }
                    minCf = Math.min(minCf, cf);
                }
                if (!ready) continue;

                double contrib = r.cf * minCf; // в том числе отрицательный/нулевой
                boolean had = next.containsKey(r.conclusion);
                double cur = had ? next.get(r.conclusion) : 0.0;
                double combined = had ? combineCf(cur, contrib) : contrib;
                next.put(r.conclusion, combined);

                int depth = 0;
                for (String p : r.premises) {
                    Derivation pd = prevDeriv.get(p);
                    if (pd != null) depth = Math.max(depth, pd.depth);
                }
                depth += 1;

                Derivation d = new Derivation();
                d.factId = r.conclusion;
                d.cf = combined;
                d.ruleId = r.id;
                d.rule = r;
                for (String p : r.premises)
                    d.premises.add(new PremiseCF(p, prev.get(p)));
                d.explanation = r.explanation;
                d.depth = depth;
                // перезапись: показываем последнее правило, внёсшее вклад
                nextDeriv.put(r.conclusion, d);
            }

            stable = sameValues(prev, next);
            prev = next;
            prevDeriv = nextDeriv;
        }

        if (!stable) {
            System.err.printf(Locale.ROOT,
                    "ВНИМАНИЕ: forwardClean не сошёлся за %d итераций%n", maxIter);
        }

        result.known = prev;
        result.derivations = prevDeriv;

        // Журнал срабатываний по финальному состоянию
        for (RuleDefinition r : sorted) {
            boolean ready = true;
            double minCf = 1.0;
            for (String p : r.premises) {
                Double cf = result.known.get(p);
                if (cf == null) { ready = false; break; }
                minCf = Math.min(minCf, cf);
            }
            if (!ready) continue;
            Double conc = result.known.get(r.conclusion);
            if (conc == null) continue;

            FiredRule fr = new FiredRule();
            fr.rule = r;
            fr.premiseCf = minCf;
            fr.conclusionCf = conc;
            fr.premises = new ArrayList<>();
            for (String p : r.premises)
                fr.premises.add(new PremiseCF(p, result.known.get(p)));
            result.fired.add(fr);
        }

        return result;
    }

    static boolean sameValues(Map<String, Double> a, Map<String, Double> b) {
        if (!a.keySet().equals(b.keySet())) return false;
        for (var e : a.entrySet()) {
            Double bv = b.get(e.getKey());
            if (bv == null || Math.abs(e.getValue() - bv) > 1e-9) return false;
        }
        return true;
    }

    // ============================================================
    // ЧИСТЫЙ ОБРАТНЫЙ ВЫВОД
    // ============================================================
    static class BackwardCleanSolver {
        final Map<String, Double> axioms;
        final Map<String, List<RuleDefinition>> byConclusion = new HashMap<>();
        final Set<String> path = new HashSet<>();

        BackwardCleanSolver(Map<String, Double> axioms,
                            Map<String, RuleDefinition> rules) {
            this.axioms = axioms;
            for (RuleDefinition r : rules.values())
                byConclusion.computeIfAbsent(r.conclusion, k -> new ArrayList<>()).add(r);
            for (List<RuleDefinition> rs : byConclusion.values())
                rs.sort(Comparator.comparing(r -> r.id));
        }

        ProofNode solve(String target) {
            return solveRec(target).orElse(null);
        }

        private Optional<ProofNode> solveRec(String fact) {
            // Цикл в текущей ветке → вклад 0
            if (path.contains(fact)) return Optional.empty();

            path.add(fact);
            List<ProofNode> candidates = new ArrayList<>();

            // (1) Аксиома
            Double axCf = axioms.get(fact);
            if (axCf != null) {
                ProofNode leaf = new ProofNode();
                leaf.factId = fact;
                leaf.cf = axCf;
                leaf.explanation = "Исходный факт";
                leaf.depth = 0;
                candidates.add(leaf);
            }

            // (2) ВСЕ правила, выводящие fact
            for (RuleDefinition r : byConclusion.getOrDefault(fact, List.of())) {
                boolean ready = true;
                List<ProofNode> prems = new ArrayList<>();
                double minCf = 1.0;
                for (String p : r.premises) {
                    Optional<ProofNode> pn = solveRec(p);
                    if (pn.isEmpty()) { ready = false; break; }
                    prems.add(pn.get());
                    minCf = Math.min(minCf, pn.get().cf);
                }
                if (!ready) continue;

                double contrib = r.cf * minCf;
                int depth = 1;
                for (ProofNode pn : prems) depth = Math.max(depth, pn.depth + 1);

                ProofNode node = new ProofNode();
                node.factId = fact;
                node.cf = contrib;
                node.rule = r;
                node.premises = prems;
                node.explanation = r.explanation;
                node.depth = depth;
                candidates.add(node);
            }

            path.remove(fact);

            if (candidates.isEmpty()) return Optional.empty();

            // Комбинируем ВСЕ вклады
            double combined = candidates.get(0).cf;
            for (int i = 1; i < candidates.size(); i++)
                combined = combineCf(combined, candidates.get(i).cf);

            // Для отображения — ветвь с максимальным |cf|
            ProofNode best = candidates.get(0);
            for (int i = 1; i < candidates.size(); i++)
                if (Math.abs(candidates.get(i).cf) > Math.abs(best.cf))
                    best = candidates.get(i);

            ProofNode result = new ProofNode();
            result.factId = fact;
            result.cf = combined;
            result.rule = best.rule;
            result.premises = best.premises;
            result.explanation = best.explanation;
            result.depth = best.depth;
            return Optional.of(result);
        }
    }

    // ==== Печать ====
    static String fmtPrems(List<PremiseCF> prems) {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < prems.size(); i++) {
            if (i > 0) sb.append(" & ");
            sb.append(String.format(Locale.ROOT, "%s (%.4f)",
                    prems.get(i).id, prems.get(i).cf));
        }
        return sb.toString();
    }

    static void printForwardProof(String id, ForwardResult fr,
                                  Set<String> printed, Set<String> visiting) {
        if (printed.contains(id) || visiting.contains(id)) return;
        Derivation d = fr.derivations.get(id);
        if (d == null) return;

        visiting.add(id);
        if (d.rule != null)
            for (PremiseCF p : d.premises)
                printForwardProof(p.id, fr, printed, visiting);
        visiting.remove(id);

        if (!printed.add(id)) return;
        if (d.rule == null) {
            System.out.printf(Locale.ROOT,
                    "  %s = %.4f — исходный факт%n", id, d.cf);
        } else {
            System.out.printf(Locale.ROOT,
                    "  %s: %s => %s; CF=%.4f; %s%n",
                    d.ruleId, fmtPrems(d.premises), id, d.cf, d.explanation);
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
            System.out.println("Дерево доказательства:");
            printForwardProof(target, fr, new LinkedHashSet<>(), new HashSet<>());
        }
    }

    static void printBackward(ProofNode root) {
        if (root == null) {
            System.out.println("Цель не выводится.");
            return;
        }
        System.out.printf(Locale.ROOT,
                "Цель выводится: %s, CF=%.4f, глубина=%d%n",
                root.factId, root.cf, root.depth);
        System.out.println("Дерево доказательства:");

        class Item {
            final ProofNode node; final boolean visited;
            Item(ProofNode n, boolean v) { node = n; visited = v; }
        }
        List<Item> stack = new ArrayList<>();
        Set<String> printedLeaves = new HashSet<>();
        Set<String> visiting = new HashSet<>();
        stack.add(new Item(root, false));

        while (!stack.isEmpty()) {
            Item it = stack.remove(stack.size() - 1);
            ProofNode node = it.node;
            if (!it.visited) {
                if (visiting.contains(node.factId)) continue;
                visiting.add(node.factId);
                stack.add(new Item(node, true));
                for (int i = node.premises.size() - 1; i >= 0; i--)
                    stack.add(new Item(node.premises.get(i), false));
            } else {
                visiting.remove(node.factId);
                if (node.rule == null) {
                    if (printedLeaves.add(node.factId)) {
                        System.out.printf(Locale.ROOT,
                                "  %s = %.4f — исходный факт%n",
                                node.factId, node.cf);
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
                            node.rule.id, prems, node.factId,
                            node.cf, node.explanation);
                }
            }
        }
    }

    // ==== main ====
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

        System.out.println("=== ЧИСТАЯ РЕАЛИЗАЦИЯ (без эвристик) ===");
        System.out.printf(Locale.ROOT,
                "Фактов: %d; правил: %d; исходных фактов: %d; цель: %s%n%n",
                facts.size(), rules.size(), axioms.size(), target);

        System.out.println("=== ПРЯМОЙ ВЫВОД ===");
        ForwardResult fwd = forwardClean(axioms, rules);
        printForward(fwd, target);

        System.out.println();
        System.out.println("=== ОБРАТНЫЙ ВЫВОД ===");
        BackwardCleanSolver solver = new BackwardCleanSolver(axioms, rules);
        ProofNode proof = solver.solve(target);
        printBackward(proof);
    }
}