#!/usr/bin/env python3
"""Build the standalone File Manager search-architecture decision board."""

from __future__ import annotations

import html
import json
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
SEARCH_DIR = ROOT / "planning" / "search"


def clean(value: str) -> str:
    value = re.sub(r"\[([^]]+)]\([^)]+\)", r"\1", value)
    value = value.replace("`", "").replace("**", "").replace("*", "")
    return re.sub(r"\s+", " ", value).strip()


def excerpt(lines: list[str]) -> str:
    return clean(" ".join(line.strip() for line in lines if line.strip()))


def parse_interview() -> list[dict]:
    path = SEARCH_DIR / "SEARCH_ARCHITECT_INTERVIEW.md"
    lines = path.read_text(encoding="utf-8").splitlines()
    cards: list[dict] = []
    section = "Foundational decisions"
    index = 0
    while index < len(lines):
        line = lines[index]
        if line.startswith("## "):
            section = clean(re.sub(r"^##\s+", "", line))
        detailed = re.match(r"^###\s+(S\d{3})\s+—\s+(.+)$", line)
        if detailed:
            card_id, question = detailed.groups()
            index += 1
            body: list[str] = []
            while index < len(lines) and not lines[index].startswith(("### ", "## ")):
                body.append(lines[index])
                index += 1
            joined_body = "\n".join(body)
            option_pattern = re.compile(
                r"^\*\*([A-D])\.\s+(.*?)\*\*(.*?)(?=^\*\*[A-D]\.\s+|\Z)",
                re.MULTILINE | re.DOTALL,
            )
            options = []
            boundary_notes: list[str] = []
            for match in option_pattern.finditer(joined_body):
                detail = clean(match.group(3))
                for marker in ("Other choice:", "Candidate boundary:"):
                    position = detail.find(marker)
                    if position >= 0:
                        boundary_notes.append(detail[position:])
                        detail = detail[:position].strip()
                options.append({
                    "key": match.group(1),
                    "label": clean(match.group(2)),
                    "detail": detail,
                })
            context = clean(option_pattern.sub("", joined_body))
            if boundary_notes:
                context = " ".join(part for part in [context, *boundary_notes] if part)
            cards.append({
                "id": card_id,
                "source": "Deep interview",
                "section": section,
                "question": clean(question),
                "context": context or "Choose an option, then qualify it with a verdict or architect note.",
                "options": options,
                "priority": "foundational",
                "source_path": "SEARCH_ARCHITECT_INTERVIEW.md",
            })
            continue
        if line.startswith("|"):
            cells = [clean(cell) for cell in line.strip().strip("|").split("|")]
            if len(cells) >= 3 and re.fullmatch(r"[UQCHFB]\d{3}", cells[0]):
                cards.append({
                    "id": cells[0],
                    "source": "Deep interview",
                    "section": section,
                    "question": cells[1],
                    "context": cells[2],
                    "options": [],
                    "priority": "standard",
                    "source_path": "SEARCH_ARCHITECT_INTERVIEW.md",
                })
        index += 1
    return cards


def parse_question_atlas() -> list[dict]:
    path = ROOT / "planning" / "QUESTION_ATLAS.md"
    lines = path.read_text(encoding="utf-8").splitlines()
    admitted = {
        "I": "Index scope, economy, and lifecycle",
        "J": "Search language and results",
        "K": "Local semantics and AI",
        "L": "CLI and AI-interrogatable API",
        "T": "Performance and resource budgets",
        "U": "Configuration, state, and registry",
        "V": "Distribution, governance, and longevity",
        "W": "Deep architectural taste",
    }
    cards = []
    index = 0
    while index < len(lines):
        line = lines[index]
        match = re.match(r"^-\s+\*\*([IJKLTUVW]\d{3})(\s+★)?\*\*\s+(.+)$", line)
        if not match:
            index += 1
            continue
        card_id, star, first = match.groups()
        body = [first]
        index += 1
        while index < len(lines) and lines[index].startswith("  "):
            body.append(lines[index].strip())
            index += 1
        cards.append({
            "id": card_id,
            "source": "Original question atlas",
            "section": admitted[card_id[0]],
            "question": clean(" ".join(body)),
            "context": "Earlier grand-architect question retained for reconciliation with the deeper search workbook.",
            "options": [],
            "priority": "starred" if star else "standard",
            "source_path": "../QUESTION_ATLAS.md",
        })
    return cards


def parse_experiments() -> list[dict]:
    path = ROOT / "research" / "zeta_search" / "EXPERIMENT_BACKLOG.md"
    lines = path.read_text(encoding="utf-8").splitlines()
    cards = []
    index = 0
    while index < len(lines):
        match = re.match(r"^##\s+(E\d{2})\s+—\s+(.+)$", lines[index])
        if not match:
            index += 1
            continue
        card_id, title = match.groups()
        index += 1
        body: list[str] = []
        while index < len(lines) and not lines[index].startswith("## "):
            body.append(lines[index])
            index += 1
        hypothesis = ""
        gates = ""
        for body_index, body_line in enumerate(body):
            if body_line.startswith(("**HYPOTHESIS", "**Question")):
                hypothesis = clean(body_line)
            if body_line.startswith("**Exact rejection gates"):
                gates = excerpt(body[body_index:])
                break
        cards.append({
            "id": card_id,
            "source": "Zeta-derived experiment backlog",
            "section": "Experiments and falsification",
            "question": f"Should we authorize and schedule {title}?",
            "context": " ".join(part for part in (hypothesis, gates) if part),
            "options": [
                {"key": "RUN", "label": "Authorize experiment", "detail": "Run against declared baselines and rejection gates."},
                {"key": "LATER", "label": "Sequence later", "detail": "Keep the experiment but block it on prerequisite evidence."},
                {"key": "DROP", "label": "Reject experiment", "detail": "The question is unnecessary or the mechanism is prohibited."},
            ],
            "priority": "experiment",
            "source_path": "../../research/zeta_search/EXPERIMENT_BACKLOG.md",
        })
    return cards


def parse_candidate_tables() -> list[dict]:
    path = SEARCH_DIR / "SEARCH_SYSTEM_OPTIONS.md"
    lines = path.read_text(encoding="utf-8").splitlines()
    included_sections = {
        "Exact identity and catalogue algorithms": "ALG",
        "Lexical candidate algorithms": "LEX",
        "Structural, duplicate, and semantic candidates": "STR",
        "Database and index-store candidates": "DB",
        "Sharding choices": "SH",
    }
    cards = []
    section = ""
    prefix = ""
    counters: dict[str, int] = {}
    for line in lines:
        heading = re.match(r"^##\s+\d+\.\s+(.+)$", line)
        if heading:
            section = clean(heading.group(1))
            prefix = included_sections.get(section, "")
            continue
        if not prefix or not line.startswith("|") or "---" in line:
            continue
        cells = [clean(cell) for cell in line.strip().strip("|").split("|")]
        if len(cells) < 3 or cells[0] in {"Mechanism", "Candidate", "Shard key"}:
            continue
        counters[prefix] = counters.get(prefix, 0) + 1
        card_id = f"{prefix}{counters[prefix]:02d}"
        cards.append({
            "id": card_id,
            "source": "Algorithm and system candidates",
            "section": section,
            "question": f"Should {cells[0]} be admitted to the measured candidate set?",
            "context": " · ".join(cells[1:]),
            "options": [
                {"key": "ADMIT", "label": "Admit to comparison", "detail": "It earns a controlled experiment, not architecture status."},
                {"key": "CONDITIONAL", "label": "Conditional", "detail": "Admit only for a named workload, hive, or scale."},
                {"key": "REJECT", "label": "Reject", "detail": "Its category, authority, or cost does not belong."},
            ],
            "priority": "candidate",
            "source_path": "SEARCH_SYSTEM_OPTIONS.md",
        })
    return cards


def supplemental_hive_questions() -> list[dict]:
    questions = [
        ("HV01", "Who or what is authoritative for each hive?"),
        ("HV02", "Is each hive fully rebuildable, partly user-authored, or irreplaceable?"),
        ("HV03", "What key hierarchy encrypts each hive, offline catalogue, and synchronized export?"),
        ("HV04", "What constitutes proof that a root and every derived record have been erased?"),
        ("HV05", "Which hives may synchronize, and which must remain machine-local?"),
        ("HV06", "How are user-fact, catalogue, and federation conflicts represented rather than hidden?"),
        ("HV07", "What migration and rollback window applies to each hive schema?"),
        ("HV08", "What does the product do when a hive is absent, stale, corrupt, locked, offline, or newer than the reader?"),
    ]
    return [{
        "id": card_id,
        "source": "Algorithm and system candidates",
        "section": "Hive authority checklist",
        "question": question,
        "context": "Apply the answer independently to catalogue, lexical, media, semantic, view, and federation hives.",
        "options": [],
        "priority": "foundational",
        "source_path": "SEARCH_SYSTEM_OPTIONS.md",
    } for card_id, question in questions]


def build_html(cards: list[dict]) -> str:
    data = json.dumps(cards, ensure_ascii=False, separators=(",", ":")).replace("</", "<\\/")
    sources = sorted({card["source"] for card in cards})
    source_options = "".join(f'<option value="{html.escape(source)}">{html.escape(source)}</option>' for source in sources)
    return f'''<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>File Manager — Search Architecture Board</title>
<style>
:root{{--ink:#172033;--muted:#647088;--line:#9aa9bc;--paper:#f5f3ea;--panel:#e8edf2;--blue:#315f9e;--blue2:#d7e7fb;--gold:#b46d2b;--green:#2f715b;--red:#9f463f;--violet:#75559a;--shadow:#6b7480;--white:#fff;}}
*{{box-sizing:border-box}} body{{margin:0;color:var(--ink);font:14px/1.35 system-ui,-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif;background:linear-gradient(135deg,#d8e7f7 0,#efe5f2 48%,#f5e6d5 100%);background-attachment:fixed}}
button,input,select,textarea{{font:inherit}} button{{cursor:pointer}} .app{{min-height:100vh;display:grid;grid-template-rows:auto auto 1fr}}
header{{background:linear-gradient(#f9faf8,#dfe5e8);border-bottom:1px solid #727d89;box-shadow:0 2px 6px #56627355}}
.titlebar{{display:flex;align-items:center;gap:12px;padding:10px 16px 8px}} .mark{{width:30px;height:30px;border:1px solid #58708f;border-radius:7px;background:radial-gradient(circle at 35% 30%,#fff 0 9%,#8db3df 10% 28%,#486f9f 29% 48%,#e3a969 49% 68%,#825a91 69%);box-shadow:inset 1px 1px #fff,1px 1px 2px #667}}
h1{{font-size:19px;margin:0}} .subtitle{{color:var(--muted);font-size:12px}}
.toolbar{{display:flex;flex-wrap:wrap;gap:7px;align-items:center;padding:7px 12px;border-top:1px solid #fff;border-bottom:1px solid #aab3bc;background:#eef1f1}}
.toolbar input[type=search]{{min-width:280px;flex:1;padding:7px 9px;border:1px solid #7e8996;background:#fff;box-shadow:inset 1px 1px 2px #9ba4ac}}
.toolbar select,.toolbar button{{padding:6px 9px;border:1px solid #7d8791;background:linear-gradient(#fff,#d8dde0);box-shadow:inset 1px 1px #fff}}
.toolbar button:active{{background:#cbd1d5;box-shadow:inset 1px 1px 2px #6d7780}}
.stats{{display:grid;grid-template-columns:repeat(5,minmax(110px,1fr));gap:1px;background:#7e8996;border-bottom:1px solid #68727e}}
.stat{{background:#f5f6f3;padding:8px 12px}} .stat b{{font-size:18px;color:var(--blue)}} .stat span{{display:block;font-size:11px;color:var(--muted)}}
main{{display:grid;grid-template-columns:230px 1fr;min-height:0}} aside{{padding:12px;background:#e8eceb;border-right:1px solid #7b8792;position:sticky;top:0;height:calc(100vh - 124px);overflow:auto}}
aside h2{{font-size:12px;letter-spacing:.08em;margin:7px 0}} .filter-btn{{display:block;width:100%;text-align:left;padding:7px 8px;margin:3px 0;border:1px solid transparent;background:transparent}} .filter-btn:hover,.filter-btn.active{{background:var(--blue2);border-color:#7891ae}}
.progress{{height:14px;border:1px solid #727e89;background:#fff;box-shadow:inset 1px 1px 2px #9099a2;margin:8px 0}} .progress>i{{display:block;height:100%;background:linear-gradient(90deg,#4777b4,#d2833d);width:0}}
.content{{padding:12px 16px 60px;overflow:auto}} .section-head{{display:flex;align-items:center;gap:10px;margin:13px 0 7px}} .section-head h2{{font-size:15px;margin:0;white-space:nowrap}} .section-head:after{{content:"";height:1px;background:#8793a1;flex:1}}
.card{{background:rgba(250,250,247,.96);border:1px solid #84909c;margin:0 0 10px;box-shadow:1px 2px 4px #56606c33}} .card.answered{{border-left:5px solid var(--green)}} .card.rejected{{border-left:5px solid var(--red)}} .card.deferred{{border-left:5px solid var(--violet)}}
.card-head{{display:grid;grid-template-columns:auto 1fr auto;gap:9px;align-items:start;padding:9px 10px;background:linear-gradient(#f5f6f3,#e2e6e7);border-bottom:1px solid #aab2ba}} .qid{{font-weight:800;color:#fff;background:var(--blue);padding:3px 6px;border-radius:2px;min-width:52px;text-align:center}} .priority{{font-size:10px;text-transform:uppercase;letter-spacing:.06em;color:#fff;background:var(--gold);padding:3px 6px}}
.question{{font-weight:700}} .meta{{font-size:11px;color:var(--muted);margin-top:2px}} .card-body{{padding:10px}} .context{{color:#384258;margin:0 0 9px;white-space:pre-line}}
.choices{{display:flex;flex-wrap:wrap;gap:6px;margin:8px 0}} .choice{{border:1px solid #7a8693;background:linear-gradient(#fff,#dce1e4);padding:6px 9px;max-width:360px;text-align:left}} .choice strong{{display:block}} .choice small{{display:block;color:#596477;margin-top:2px}} .choice.selected{{background:linear-gradient(#dcecff,#b7d2f1);border-color:#2f5f98;box-shadow:inset 0 0 0 1px #fff}}
.verdicts{{display:flex;flex-wrap:wrap;gap:4px;padding-top:8px;border-top:1px dotted #a6afb8}} .verdict{{padding:5px 8px;border:1px solid #8b949e;background:#ecefed}} .verdict.selected[data-v=yes]{{background:#cce8d9;border-color:#31705a}} .verdict.selected[data-v=no]{{background:#f0d0cc;border-color:#99473f}} .verdict.selected[data-v=conditional]{{background:#f2e0b9;border-color:#a56a2a}} .verdict.selected[data-v=experiment]{{background:#d5e4f7;border-color:#3d6596}} .verdict.selected[data-v=discuss],.verdict.selected[data-v=defer]{{background:#e2d7ef;border-color:#72538f}}
.notes{{width:100%;min-height:54px;margin-top:8px;padding:7px;border:1px solid #9aa4ae;background:#fffdf7;resize:vertical}} .empty{{padding:40px;text-align:center;color:var(--muted)}}
.sr-only{{position:absolute;width:1px;height:1px;overflow:hidden;clip:rect(0,0,0,0)}}
@media(max-width:820px){{.stats{{grid-template-columns:repeat(2,1fr)}}main{{grid-template-columns:1fr}}aside{{position:static;height:auto;border-right:0;border-bottom:1px solid #7b8792}}.filter-btn{{display:inline-block;width:auto}}}}
</style>
</head>
<body><div class="app">
<header><div class="titlebar"><div class="mark"></div><div><h1>Search Architecture Decision Board</h1><div class="subtitle">Identity · databases · updates · shards · ranking · semantic hives</div></div></div>
<div class="toolbar"><input id="search" type="search" placeholder="Search IDs, questions, algorithms, or context…"><select id="source"><option value="">All sources</option>{source_options}</select><select id="state"><option value="">All states</option><option value="unanswered">Unanswered</option><option value="answered">Answered</option><option value="defer">Deferred</option><option value="rejected">Rejected</option></select><button id="export-md">Export Markdown</button><button id="export-json">Export JSON</button><button id="import-json">Import</button><input id="import-file" class="sr-only" type="file" accept="application/json"><button id="reset">Reset</button></div></header>
<div class="stats"><div class="stat"><b id="total">0</b><span>visible questions</span></div><div class="stat"><b id="answered">0</b><span>answered</span></div><div class="stat"><b id="discuss">0</b><span>discussion/deferred</span></div><div class="stat"><b id="remaining">0</b><span>remaining</span></div><div class="stat"><b id="percent">0%</b><span>overall progress</span></div></div>
<main><aside><h2>PROGRESS</h2><div class="progress"><i id="progress-bar"></i></div><div id="source-filters"></div><h2>QUICK VIEWS</h2><button class="filter-btn" data-quick="foundational">Foundational</button><button class="filter-btn" data-quick="starred">Starred atlas</button><button class="filter-btn" data-quick="candidate">Algorithm candidates</button><button class="filter-btn" data-quick="experiment">Experiments</button><button class="filter-btn" data-quick="">Clear quick view</button><p class="subtitle">Answers are saved only in this browser via localStorage. Export before moving machines or clearing browser data.</p></aside><section class="content" id="cards"></section></main>
</div>
<script id="question-data" type="application/json">{data}</script>
<script>
const questions=JSON.parse(document.getElementById('question-data').textContent);const STORE='file-manager-search-board-v1';let answers=JSON.parse(localStorage.getItem(STORE)||'{{}}');let quick='';
const $=s=>document.querySelector(s);const save=()=>localStorage.setItem(STORE,JSON.stringify(answers));
function statusOf(a){{if(!a)return'unanswered';if(a.verdict==='no')return'rejected';if(['defer','discuss'].includes(a.verdict))return'defer';if(a.choice||a.verdict||a.notes)return'answered';return'unanswered'}}
function esc(s){{return String(s||'').replace(/[&<>"']/g,c=>({{'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}}[c]))}}
function render(){{const term=$('#search').value.trim().toLowerCase(),src=$('#source').value,state=$('#state').value;let visible=questions.filter(q=>(!src||q.source===src)&&(!quick||q.priority===quick)&&(!term||[q.id,q.question,q.context,q.section,q.source].join(' ').toLowerCase().includes(term))&&(!state||statusOf(answers[q.id])===state));let groups=new Map;visible.forEach(q=>{{let k=q.source+' · '+q.section;if(!groups.has(k))groups.set(k,[]);groups.get(k).push(q)}});let out='';groups.forEach((list,name)=>{{out+=`<div class="section-head"><h2>${{esc(name)}} <small>(${{list.length}})</small></h2></div>`;list.forEach(q=>out+=card(q))}});$('#cards').innerHTML=out||'<div class="empty">No questions match this view.</div>';bind();stats(visible)}}
function card(q){{const a=answers[q.id]||{{}},status=statusOf(a);let choices=q.options.map(o=>`<button class="choice ${{a.choice===o.key?'selected':''}}" data-id="${{esc(q.id)}}" data-choice="${{esc(o.key)}}"><strong>${{esc(o.key)}} · ${{esc(o.label)}}</strong><small>${{esc(o.detail)}}</small></button>`).join('');return`<article class="card ${{status}}"><div class="card-head"><span class="qid">${{esc(q.id)}}</span><div><div class="question">${{esc(q.question)}}</div><div class="meta">${{esc(q.source)}} · ${{esc(q.source_path)}}</div></div><span class="priority">${{esc(q.priority)}}</span></div><div class="card-body"><p class="context">${{esc(q.context)}}</p>${{choices?`<div class="choices">${{choices}}</div>`:''}}<div class="verdicts">${{[['yes','Yes / accept'],['no','No / reject'],['conditional','Conditional'],['experiment','Experiment first'],['discuss','Discuss'],['defer','Defer']].map(v=>`<button class="verdict ${{a.verdict===v[0]?'selected':''}}" data-id="${{esc(q.id)}}" data-v="${{v[0]}}">${{v[1]}}</button>`).join('')}}</div><textarea class="notes" data-id="${{esc(q.id)}}" placeholder="Architect notes, boundaries, defaults, failure modes…">${{esc(a.notes||'')}}</textarea></div></article>`}}
function bind(){{document.querySelectorAll('.choice').forEach(b=>b.onclick=()=>{{let a=answers[b.dataset.id]||{{}};a.choice=a.choice===b.dataset.choice?'':b.dataset.choice;answers[b.dataset.id]=a;save();render()}});document.querySelectorAll('.verdict').forEach(b=>b.onclick=()=>{{let a=answers[b.dataset.id]||{{}};a.verdict=a.verdict===b.dataset.v?'':b.dataset.v;answers[b.dataset.id]=a;save();render()}});document.querySelectorAll('.notes').forEach(t=>t.onchange=()=>{{let a=answers[t.dataset.id]||{{}};a.notes=t.value;answers[t.dataset.id]=a;save();stats()}})}}
function stats(visible=questions){{let vals=questions.map(q=>statusOf(answers[q.id])),done=vals.filter(v=>v==='answered'||v==='rejected').length,disc=vals.filter(v=>v==='defer').length;$('#total').textContent=visible.length;$('#answered').textContent=done;$('#discuss').textContent=disc;$('#remaining').textContent=questions.length-done-disc;let p=Math.round(100*(done+disc)/questions.length);$('#percent').textContent=p+'%';$('#progress-bar').style.width=p+'%'}}
function download(name,type,text){{let a=document.createElement('a');a.href=URL.createObjectURL(new Blob([text],{{type}}));a.download=name;a.click();setTimeout(()=>URL.revokeObjectURL(a.href),500)}}
$('#export-json').onclick=()=>download('search-architecture-answers.json','application/json',JSON.stringify({{version:1,exported:new Date().toISOString(),answers}},null,2));
$('#export-md').onclick=()=>{{let rows=['# Search architecture answers',''];questions.forEach(q=>{{let a=answers[q.id];if(!a)return;rows.push(`## ${{q.id}} — ${{q.question}}`,'',`- Source: ${{q.source}}`,`- Choice: ${{a.choice||'—'}}`,`- Verdict: ${{a.verdict||'—'}}`,`- Notes: ${{a.notes||'—'}}`,'')}});download('search-architecture-answers.md','text/markdown',rows.join('\\n'))}};
$('#import-json').onclick=()=>$('#import-file').click();$('#import-file').onchange=e=>{{let f=e.target.files[0];if(!f)return;let r=new FileReader;r.onload=()=>{{try{{let o=JSON.parse(r.result);answers=o.answers||o;save();render()}}catch{{alert('Invalid answer JSON.')}}}};r.readAsText(f)}};
$('#reset').onclick=()=>{{if(confirm('Clear every saved answer and note in this browser?')){{answers={{}};save();render()}}}};['search','source','state'].forEach(id=>$('#'+id).oninput=render);
document.querySelectorAll('[data-quick]').forEach(b=>b.onclick=()=>{{quick=b.dataset.quick;document.querySelectorAll('[data-quick]').forEach(x=>x.classList.toggle('active',x===b));render()}});
const counts={{}};questions.forEach(q=>counts[q.source]=(counts[q.source]||0)+1);$('#source-filters').innerHTML='<h2>SOURCES</h2>'+Object.entries(counts).map(([s,n])=>`<button class="filter-btn" data-src="${{esc(s)}}">${{esc(s)}} (${{n}})</button>`).join('');document.querySelectorAll('[data-src]').forEach(b=>b.onclick=()=>{{$('#source').value=b.dataset.src;render()}});render();
</script></body></html>'''


def main() -> None:
    cards = parse_interview() + parse_question_atlas() + parse_experiments()
    cards += parse_candidate_tables() + supplemental_hive_questions()
    seen: set[str] = set()
    duplicates = [card["id"] for card in cards if card["id"] in seen or seen.add(card["id"])]
    if duplicates:
        raise SystemExit(f"duplicate question IDs: {duplicates}")
    output = SEARCH_DIR / "search-architecture-board.html"
    output.write_text(build_html(cards), encoding="utf-8")
    print(json.dumps({"output": str(output), "questions": len(cards)}, indent=2))


if __name__ == "__main__":
    main()
