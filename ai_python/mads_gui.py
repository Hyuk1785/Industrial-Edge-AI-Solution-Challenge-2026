import tkinter as tk
from tkinter import ttk, scrolledtext, messagebox
import tkinter.font as tkfont
import serial
import serial.tools.list_ports
import csv
from datetime import datetime
import threading
import time
import re
import os

# ─────────────────────────────────────────────
# Retro Industrial (High-Contrast Amber CRT)
# ─────────────────────────────────────────────
C = {
    "bg":        "#06070A",
    "panel":     "#0C0F14",
    "card":      "#101622",
    "card2":     "#0B101A",
    "text_bg":   "#05070A",

    "border":    "#263146",
    "border_hi": "#3E5175",

    "amber":     "#FFCC33",
    "amber_dim": "#D9A116",

    "green":     "#24FF72",
    "green_dim": "#0D3F25",

    "red":       "#FF3B3B",
    "red_dim":   "#3B1212",

    "yellow":    "#FFD84D",

    "fg":        "#F2E6C9",
    "fg_dim":    "#B99652",
    "fg_bright": "#FFF2C2",
}


class MotorMonitor:
    def __init__(self, root):
        self.root = root
        self.root.title("MADS | Motor Anomaly Detection System")
        self.root.geometry("1160x740")
        self.root.configure(bg=C["bg"])
        self.root.resizable(True, True)

        # ✅ INFER threshold 고정
        self.THRESHOLD = 0.6

        self._init_fonts()
        self._init_ttk_style()

        self.serial_port = None
        self.is_running = False
        self.app_mode = tk.StringVar(value="collect")

        # COLLECT
        self.data_buffer = []
        self.start_time = None
        self.collect_start_time = None
        self.auto_save_mode = "normal"
        self.auto_save_timer = None
        self.sample_count = 0
        self.samples_in_last_second = 0
        self.last_rate_time = time.time()
        self.file_index = 0

        # INFER
        self.vote_history = []
        self.infer_count = 0
        self.defect_count = 0
        self.last_score = 0.0
        self.current_status = "WAITING"

        # INFER CSV
        self.infer_rows = []
        self.infer_session_start = None
        self.infer_csv_name = None

        self._build_ui()
        self.refresh_ports()
        self._switch_mode()

    # ─────────────────────────────────────────────
    # Font selection (현장 가독성/한글 포함)
    # ─────────────────────────────────────────────
    def _pick_font(self, candidates, size, weight="normal"):
        avail = set(tkfont.families(self.root))
        for fam in candidates:
            if fam in avail:
                return (fam, size, weight)
        return ("TkDefaultFont", size, weight)

    def _init_fonts(self):
        # UI 폰트(한글 포함 잘 보이는 쪽 우선)
        self.F_UI_7   = self._pick_font(["Bahnschrift", "Malgun Gothic", "Segoe UI", "Arial"], 7, "normal")
        self.F_UI_8   = self._pick_font(["Bahnschrift", "Malgun Gothic", "Segoe UI", "Arial"], 8, "normal")
        self.F_UI_9B  = self._pick_font(["Bahnschrift", "Malgun Gothic", "Segoe UI", "Arial"], 9, "bold")
        self.F_UI_10B = self._pick_font(["Bahnschrift", "Malgun Gothic", "Segoe UI", "Arial"], 10, "bold")
        self.F_UI_11B = self._pick_font(["Bahnschrift", "Malgun Gothic", "Segoe UI", "Arial"], 11, "bold")
        self.F_UI_14B = self._pick_font(["Bahnschrift", "Malgun Gothic", "Segoe UI", "Arial"], 14, "bold")
        self.F_UI_24B = self._pick_font(["Bahnschrift", "Malgun Gothic", "Segoe UI", "Arial"], 24, "bold")
        self.F_UI_44B = self._pick_font(["Bahnschrift", "Malgun Gothic", "Segoe UI", "Arial"], 44, "bold")

        # 로그/수치 모노(한글 로그까지 고려: D2Coding 있으면 베스트)
        self.F_MONO_8  = self._pick_font(["D2Coding", "NanumGothicCoding", "Cascadia Mono", "Consolas", "Courier New"], 8, "normal")
        self.F_MONO_8B = self._pick_font(["D2Coding", "NanumGothicCoding", "Cascadia Mono", "Consolas", "Courier New"], 8, "bold")
        self.F_MONO_9B = self._pick_font(["D2Coding", "NanumGothicCoding", "Cascadia Mono", "Consolas", "Courier New"], 9, "bold")
        self.F_MONO_11B = self._pick_font(["D2Coding", "NanumGothicCoding", "Cascadia Mono", "Consolas", "Courier New"], 11, "bold")
        self.F_MONO_12B = self._pick_font(["D2Coding", "NanumGothicCoding", "Cascadia Mono", "Consolas", "Courier New"], 12, "bold")

    # ─────────────────────────────────────────────
    # ttk style
    # ─────────────────────────────────────────────
    def _init_ttk_style(self):
        self.style = ttk.Style(self.root)
        try:
            self.style.theme_use("clam")
        except Exception:
            pass

        self.style.configure(
            "Retro.TCombobox",
            fieldbackground=C["card2"],
            background=C["card2"],
            foreground=C["amber"],
            selectbackground=C["border_hi"],
            selectforeground=C["amber"],
            bordercolor=C["border"],
            lightcolor=C["border_hi"],
            darkcolor=C["border"],
            arrowcolor=C["amber"],
            font=self.F_MONO_9B if hasattr(self, "F_MONO_9B") else None,
        )
        self.style.map(
            "Retro.TCombobox",
            fieldbackground=[("readonly", C["card2"])],
            foreground=[("readonly", C["amber"])],
            background=[("readonly", C["card2"])],
        )

    # ─────────────────────────────────────────────
    # Section title spacing (영문 대문자만)
    # ─────────────────────────────────────────────
    def _format_section_title(self, title: str) -> str:
        if re.fullmatch(r"[A-Z0-9 ]+", title.strip()):
            tokens = title.strip().split()
            spaced_tokens = [" ".join(list(tok)) for tok in tokens]
            return "   ".join(spaced_tokens)
        return title

    # ─────────────────────────────────────────────
    # UI
    # ─────────────────────────────────────────────
    def _build_ui(self):
        title_bar = tk.Frame(self.root, bg=C["panel"], height=48)
        title_bar.pack(fill="x")
        title_bar.pack_propagate(False)

        logo_f = tk.Frame(title_bar, bg=C["panel"])
        logo_f.pack(side="left", padx=16, pady=8)
        tk.Label(logo_f, text="▣", font=self.F_UI_14B,
                 bg=C["panel"], fg=C["amber"]).pack(side="left", padx=(0, 8))
        tk.Label(logo_f, text="MADS", font=self.F_UI_14B,
                 bg=C["panel"], fg=C["amber"]).pack(side="left")
        tk.Label(logo_f, text="  MOTOR ANOMALY DETECTION SYSTEM",
                 font=self.F_UI_8, bg=C["panel"], fg=C["fg_dim"]).pack(side="left", pady=(4, 0))

        right_f = tk.Frame(title_bar, bg=C["panel"])
        right_f.pack(side="right", padx=16, pady=6)

        self.lbl_clock = tk.Label(right_f, text="00:00:00",
                                  font=self.F_UI_11B,
                                  bg=C["panel"], fg=C["amber_dim"])
        self.lbl_clock.pack(side="right", padx=(16, 0))
        self._tick_clock()

        tab_f = tk.Frame(right_f, bg=C["panel"])
        tab_f.pack(side="right")

        self.btn_collect_tab = tk.Button(
            tab_f, text="[ COLLECT ]",
            font=self.F_UI_10B,
            relief="flat", cursor="hand2", bd=0,
            padx=12, pady=4,
            command=lambda: self._set_mode("collect"))
        self.btn_collect_tab.pack(side="left", padx=2)

        self.btn_infer_tab = tk.Button(
            tab_f, text="[ INFER ]",
            font=self.F_UI_10B,
            relief="flat", cursor="hand2", bd=0,
            padx=12, pady=4,
            command=lambda: self._set_mode("infer"))
        self.btn_infer_tab.pack(side="left", padx=2)

        tk.Frame(self.root, bg=C["amber"], height=1).pack(fill="x")
        tk.Frame(self.root, bg=C["amber_dim"], height=1).pack(fill="x")

        self.main_frame = tk.Frame(self.root, bg=C["bg"])
        self.main_frame.pack(fill="both", expand=True, padx=10, pady=8)

        self.collect_frame = tk.Frame(self.main_frame, bg=C["bg"])
        self._build_collect_panel(self.collect_frame)

        self.infer_frame = tk.Frame(self.main_frame, bg=C["bg"])
        self._build_infer_panel(self.infer_frame)

        sb = tk.Frame(self.root, bg=C["panel"], height=22)
        sb.pack(fill="x", side="bottom")
        sb.pack_propagate(False)
        tk.Frame(sb, bg=C["amber"], width=2).pack(side="left", fill="y")
        self.status_bar = tk.Label(sb, text="SYS: READY",
                                   font=self.F_MONO_8,
                                   bg=C["panel"], fg=C["amber_dim"],
                                   anchor="w", padx=8)
        self.status_bar.pack(side="left", fill="x", expand=True)
        tk.Label(sb, text="STM32 EDGE AI  |  INDUSTRIAL MONITOR",
                 font=self.F_MONO_8,
                 bg=C["panel"], fg=C["fg_dim"]).pack(side="right", padx=10)

    def _tick_clock(self):
        self.lbl_clock.config(text=datetime.now().strftime("%H:%M:%S"))
        self.root.after(1000, self._tick_clock)

    # ──────────────────────────────────────────
    # COLLECT
    # ──────────────────────────────────────────
    def _build_collect_panel(self, parent):
        left = tk.Frame(parent, bg=C["bg"], width=240)
        left.pack(side="left", fill="y", padx=(0, 8))
        left.pack_propagate(False)

        right = tk.Frame(parent, bg=C["bg"])
        right.pack(side="right", fill="both", expand=True)

        self._box(left, "CONNECTION", self._conn_content)
        self._box(left, "DATA MODE", self._mode_content)
        self._box(left, "AUTO SAVE", self._save_content)
        self._box(left, "CONTROL", self._ctrl_content)

        self._box(right, "STATISTICS", self._stats_content, fill="x")
        self._box(right, "SENSOR LIVE", self._sensor_content, fill="x")

        log_body = self._box(right, "DATA LOG", None, fill="both", expand=True)
        self.text_log = scrolledtext.ScrolledText(
            log_body, font=self.F_MONO_8,
            bg=C["text_bg"], fg=C["fg"],
            insertbackground=C["amber"],
            selectbackground=C["border_hi"],
            selectforeground=C["fg_bright"],
            relief="flat", bd=0)
        self.text_log.pack(fill="both", expand=True, padx=4, pady=4)

    def _conn_content(self, f):
        self._lbl(f, "PORT")
        pf = tk.Frame(f, bg=C["card"])
        pf.pack(fill="x", padx=8, pady=(2, 4))
        self.combo_port = self._combo(pf, 17)
        self.combo_port.pack(side="left")
        self._mini_btn(pf, "↺", self.refresh_ports).pack(side="left", padx=4)

        self._lbl(f, "BAUD RATE")
        self.combo_baud = self._combo(f, 12, ["115200", "230400"])
        self.combo_baud.set("115200")
        self.combo_baud.pack(anchor="w", padx=8, pady=(2, 8))

    def _mode_content(self, f):
        self.mode_var = tk.StringVar(value="normal")
        for val, txt, col in [("normal", "◈  NORMAL DATA", C["green"]),
                              ("defect", "◈  DEFECT DATA", C["red"])]:
            rb = tk.Radiobutton(
                f, text=txt, variable=self.mode_var, value=val,
                font=self.F_UI_9B,
                bg=C["card"], fg=col,
                selectcolor=C["border"],
                activebackground=C["card"],
                activeforeground=col,
                command=self.update_mode
            )
            rb.pack(anchor="w", padx=10, pady=5)

    def _save_content(self, f):
        tk.Label(f, text="  INTERVAL : 120 SEC", font=self.F_MONO_8,
                 bg=C["card"], fg=C["fg_dim"]).pack(anchor="w", padx=8, pady=(8, 2))
        self.auto_save_var = tk.BooleanVar(value=True)
        cb = tk.Checkbutton(
            f, text=" ENABLE AUTO SAVE", variable=self.auto_save_var,
            bg=C["card"], fg=C["fg_bright"],
            selectcolor=C["border"],
            activebackground=C["card"],
            activeforeground=C["fg_bright"],
            font=self.F_UI_9B
        )
        cb.pack(anchor="w", padx=8, pady=(0, 8))

    def _ctrl_content(self, f):
        self.btn_start = self._big_btn(f, "▶  ACQUIRE", C["green"], self.start)
        self.btn_stop = self._big_btn(f, "■  HALT", C["red"], self.stop, state="disabled")
        self._big_btn(f, "▤  SAVE NOW", C["amber"], self.manual_save)

    def _stats_content(self, f):
        g = tk.Frame(f, bg=C["card"])
        g.pack(fill="x", padx=8, pady=8)
        items = [("SAMPLES", "lbl_samples", "--------"),
                 ("RATE", "lbl_rate", "-- Hz"),
                 ("ELAPSED", "lbl_time", "--:--:--"),
                 ("NEXT SAVE", "lbl_next_save", "--:--")]
        for i, (lbl, attr, init) in enumerate(items):
            col = tk.Frame(g, bg=C["card2"],
                           highlightbackground=C["border"],
                           highlightthickness=1)
            col.grid(row=0, column=i, padx=4, pady=4, sticky="nsew")
            g.columnconfigure(i, weight=1)
            tk.Label(col, text=lbl, font=self.F_MONO_8,
                     bg=C["card2"], fg=C["fg_dim"]).pack(pady=(6, 2))
            lb = tk.Label(col, text=init, font=self.F_MONO_12B,
                          bg=C["card2"], fg=C["amber"])
            lb.pack(pady=(0, 6))
            setattr(self, attr, lb)

    def _sensor_content(self, f):
        g = tk.Frame(f, bg=C["card"])
        g.pack(fill="x", padx=8, pady=8)
        items = [("ACCEL", "lbl_accel", "X:--  Y:--  Z:--"),
                 ("GYRO", "lbl_gyro", "X:--  Y:--  Z:--"),
                 ("CURRENT", "lbl_current", "-.--- A"),
                 ("VOLTAGE", "lbl_voltage", "--.-- V"),
                 ("POWER", "lbl_power", "--.-- W")]
        for i, (lbl, attr, init) in enumerate(items):
            r, c = divmod(i, 3)
            fr = tk.Frame(g, bg=C["card2"],
                          highlightbackground=C["border"],
                          highlightthickness=1)
            fr.grid(row=r, column=c, padx=3, pady=3, sticky="nsew")
            g.columnconfigure(c, weight=1)
            tk.Label(fr, text=lbl, font=self.F_MONO_8,
                     bg=C["card2"], fg=C["fg_dim"]).pack(pady=(4, 1))
            lb = tk.Label(fr, text=init, font=self.F_MONO_9B,
                          bg=C["card2"], fg=C["fg_bright"])
            lb.pack(pady=(0, 4))
            setattr(self, attr, lb)

    # ──────────────────────────────────────────
    # INFER
    # ──────────────────────────────────────────
    def _build_infer_panel(self, parent):
        left = tk.Frame(parent, bg=C["bg"], width=240)
        left.pack(side="left", fill="y", padx=(0, 8))
        left.pack_propagate(False)

        self._box(left, "CONNECTION", self._conn_infer_content)
        self._box(left, "CONTROL", self._ctrl_infer_content)
        self._box(left, "STATISTICS", self._infer_stats_content)

        right = tk.Frame(parent, bg=C["bg"])
        right.pack(side="right", fill="both", expand=True)

        status_outer = tk.Frame(right, bg=C["border"], padx=1, pady=1)
        status_outer.pack(fill="x", pady=(0, 6))
        status_body = tk.Frame(status_outer, bg=C["card"])
        status_body.pack(fill="x")

        tk.Label(status_body, text="  SYSTEM STATUS",
                 font=self.F_UI_10B,
                 bg=C["card"], fg=C["fg_dim"],
                 anchor="w").pack(fill="x", pady=(4, 0))
        tk.Frame(status_body, bg=C["border"], height=1).pack(fill="x")

        inner = tk.Frame(status_body, bg=C["card"])
        inner.pack(fill="x", padx=14, pady=14)

        self.status_indicator = tk.Label(
            inner, text="■",
            font=self.F_UI_44B,
            bg=C["card"], fg=C["fg_dim"])
        self.status_indicator.pack(side="left", padx=(0, 16))

        txt_f = tk.Frame(inner, bg=C["card"])
        txt_f.pack(side="left", fill="x", expand=True)

        self.lbl_status_main = tk.Label(
            txt_f, text="STANDBY",
            font=self.F_UI_24B,
            bg=C["card"], fg=C["fg_dim"],
            anchor="w")
        self.lbl_status_main.pack(fill="x")

        self.lbl_status_sub = tk.Label(
            txt_f, text="> 시리얼 포트 연결 후 START INFER 를 누르세요",
            font=self.F_UI_9B,
            bg=C["card"], fg=C["fg_dim"],
            anchor="w")
        self.lbl_status_sub.pack(fill="x", pady=(4, 0))

        self.lbl_status_time = tk.Label(
            txt_f, text="LAST UPDATE : --:--:--",
            font=self.F_MONO_8,
            bg=C["card"], fg=C["amber_dim"],
            anchor="w")
        self.lbl_status_time.pack(fill="x", pady=(2, 0))

        score_body = self._box(right, "ANOMALY SCORE", None, fill="x")
        sc_inner = tk.Frame(score_body, bg=C["card"])
        sc_inner.pack(fill="x", padx=8, pady=8)

        bar_f = tk.Frame(sc_inner, bg=C["card"])
        bar_f.pack(fill="x")

        tick_f = tk.Frame(bar_f, bg=C["card"])
        tick_f.pack(fill="x")
        for t in ["0.0", "0.2", "0.4", "0.6", "0.8", "1.0"]:
            tk.Label(tick_f, text=t, font=self.F_MONO_8,
                     bg=C["card"], fg=C["fg_dim"]).pack(side="left", expand=True)

        self.score_canvas = tk.Canvas(
            bar_f, height=22,
            bg=C["text_bg"], highlightthickness=1,
            highlightbackground=C["border"])
        self.score_canvas.pack(fill="x", pady=(0, 4))

        score_val_f = tk.Frame(sc_inner, bg=C["card"])
        score_val_f.pack(fill="x")
        tk.Label(score_val_f, text="SCORE :", font=self.F_MONO_8,
                 bg=C["card"], fg=C["fg_dim"]).pack(side="left")
        self.lbl_score_val = tk.Label(
            score_val_f, text="0.0000",
            font=self.F_MONO_11B,
            bg=C["card"], fg=C["amber"])
        self.lbl_score_val.pack(side="left", padx=6)

        # ✅ threshold 표시 0.600
        self.lbl_threshold = tk.Label(
            score_val_f, text=f"THRESHOLD : {self.THRESHOLD:.3f}",
            font=self.F_MONO_8,
            bg=C["card"], fg=C["fg_dim"]
        )
        self.lbl_threshold.pack(side="right")

        vote_body = self._box(right, "VOTE WINDOW  [ 최근 5회 판정 ]", None, fill="x")
        vote_inner = tk.Frame(vote_body, bg=C["card"])
        vote_inner.pack(fill="x", padx=10, pady=10)

        self.vote_boxes = []
        for i in range(5):
            vf = tk.Frame(vote_inner, bg=C["border"], padx=1, pady=1)
            vf.pack(side="left", padx=5)
            box = tk.Label(
                vf, text=f"  {i+1}  \n-----\n  ?  ",
                font=self.F_MONO_9B,
                bg=C["card2"], fg=C["fg_dim"],
                justify="center", padx=8, pady=6)
            box.pack()
            self.vote_boxes.append(box)

        log_body = self._box(right, "INFERENCE LOG", None, fill="both", expand=True)
        self.text_infer_log = scrolledtext.ScrolledText(
            log_body, font=self.F_MONO_8,
            bg=C["text_bg"], fg=C["fg"],
            insertbackground=C["amber"],
            selectbackground=C["border_hi"],
            selectforeground=C["fg_bright"],
            relief="flat", bd=0)
        self.text_infer_log.pack(fill="both", expand=True, padx=4, pady=4)

        self.text_infer_log.tag_config("normal", foreground=C["green"])
        self.text_infer_log.tag_config("defect", foreground=C["red"])
        self.text_infer_log.tag_config("detect", foreground=C["yellow"])
        self.text_infer_log.tag_config("sys", foreground=C["amber_dim"])
        self.text_infer_log.tag_config("ts", foreground=C["fg_dim"])

    def _conn_infer_content(self, f):
        self._lbl(f, "PORT")
        pf = tk.Frame(f, bg=C["card"])
        pf.pack(fill="x", padx=8, pady=(2, 4))
        self.combo_port_infer = self._combo(pf, 17)
        self.combo_port_infer.pack(side="left")
        self._mini_btn(pf, "↺", self.refresh_ports).pack(side="left", padx=4)

        self._lbl(f, "BAUD RATE")
        self.combo_baud_infer = self._combo(f, 12, ["115200", "230400"])
        self.combo_baud_infer.set("115200")
        self.combo_baud_infer.pack(anchor="w", padx=8, pady=(2, 8))

    def _ctrl_infer_content(self, f):
        self.btn_infer_start = self._big_btn(f, "▶  START INFER", C["green"], self.start_infer)
        self.btn_infer_stop = self._big_btn(f, "■  HALT", C["red"], self.stop_infer, state="disabled")
        self.btn_infer_save = self._big_btn(f, "▤  SAVE CSV", C["amber"], self.save_infer_csv, state="disabled")

    def _infer_stats_content(self, f):
        g = tk.Frame(f, bg=C["card"])
        g.pack(fill="x", padx=8, pady=8)
        items = [("TOTAL", "lbl_infer_total", "0"),
                 ("FAULT", "lbl_infer_defect", "0"),
                 ("FAULT %", "lbl_infer_rate", "0.0%")]
        for lbl, attr, init in items:
            rf = tk.Frame(g, bg=C["card2"],
                          highlightbackground=C["border"],
                          highlightthickness=1)
            rf.pack(fill="x", pady=2)
            tk.Label(rf, text=f"  {lbl}", font=self.F_MONO_8,
                     bg=C["card2"], fg=C["fg_dim"],
                     width=10, anchor="w").pack(side="left", padx=4, pady=5)
            lb = tk.Label(rf, text=init, font=self.F_MONO_9B,
                          bg=C["card2"], fg=C["amber"])
            lb.pack(side="right", padx=8)
            setattr(self, attr, lb)

    # ──────────────────────────────────────────
    # common widgets
    # ──────────────────────────────────────────
    def _box(self, parent, title, content_fn, fill="x", expand=False):
        outer = tk.Frame(parent, bg=C["bg"])
        outer.pack(fill=fill, expand=expand, pady=(0, 6))

        tb = tk.Frame(outer, bg=C["card2"])
        tb.pack(fill="x")
        tk.Label(tb, text="  " + self._format_section_title(title),
                 font=self.F_UI_10B,   # ✅ 제목 더 또렷
                 bg=C["card2"], fg=C["fg_dim"],
                 anchor="w").pack(side="left", pady=3)

        body = tk.Frame(outer, bg=C["card"],
                        highlightbackground=C["border"],
                        highlightthickness=1)
        body.pack(fill=fill, expand=expand)

        if content_fn:
            content_fn(body)
        return body

    def _lbl(self, parent, text):
        tk.Label(parent, text=f"  {text}",
                 font=self.F_MONO_8,
                 bg=C["card"], fg=C["fg_dim"],
                 anchor="w").pack(fill="x", padx=4, pady=(6, 1))

    def _combo(self, parent, w, values=None):
        cb = ttk.Combobox(
            parent, width=w, state="readonly",
            font=self.F_MONO_9B,
            style="Retro.TCombobox"
        )
        if values:
            cb["values"] = values
        return cb

    def _mini_btn(self, parent, text, cmd):
        return tk.Button(
            parent, text=text, command=cmd,
            font=self.F_UI_9B,
            bg=C["border"], fg=C["amber"],
            activebackground=C["border_hi"],
            activeforeground=C["amber"],
            relief="flat", cursor="hand2",
            padx=6, pady=2
        )

    def _big_btn(self, parent, text, color, cmd, state="normal"):
        btn = tk.Button(
            parent, text=text, command=cmd, state=state,
            font=self.F_UI_9B,
            bg=C["card2"], fg=color,
            activebackground=C["border_hi"],
            activeforeground=color,
            disabledforeground=C["fg_dim"],
            relief="flat", cursor="hand2",
            pady=8,
            highlightbackground=C["border"],
            highlightthickness=1
        )
        btn.pack(fill="x", padx=8, pady=3)
        return btn

    # ──────────────────────────────────────────
    # mode switch
    # ──────────────────────────────────────────
    def _stop_current_session(self, current_mode):
        if not self.is_running:
            return
        if current_mode == "collect":
            self.stop()
        else:
            self.stop_infer()

    def _set_mode(self, mode):
        current_mode = self.app_mode.get()
        if self.is_running:
            self._stop_current_session(current_mode)
        self.app_mode.set(mode)
        self._switch_mode()

    def _switch_mode(self):
        mode = self.app_mode.get()
        self.collect_frame.pack_forget()
        self.infer_frame.pack_forget()
        if mode == "collect":
            self.collect_frame.pack(fill="both", expand=True)
            self.btn_collect_tab.config(bg=C["amber"], fg=C["bg"], activebackground=C["amber"], activeforeground=C["bg"])
            self.btn_infer_tab.config(bg=C["card2"], fg=C["fg_dim"], activebackground=C["border_hi"], activeforeground=C["fg_bright"])
        else:
            self.infer_frame.pack(fill="both", expand=True)
            self.btn_infer_tab.config(bg=C["amber"], fg=C["bg"], activebackground=C["amber"], activeforeground=C["bg"])
            self.btn_collect_tab.config(bg=C["card2"], fg=C["fg_dim"], activebackground=C["border_hi"], activeforeground=C["fg_bright"])

    # ══════════════════════════════════════════
    # COLLECT logic
    # ══════════════════════════════════════════
    def refresh_ports(self):
        ports = serial.tools.list_ports.comports()
        port_list = [f"{p.device} - {p.description}" for p in ports]
        for cb in ["combo_port", "combo_port_infer"]:
            if hasattr(self, cb):
                getattr(self, cb)["values"] = port_list
                if port_list:
                    getattr(self, cb).current(0)
                else:
                    try:
                        getattr(self, cb).set("")
                    except Exception:
                        pass
        self._set_status(f"SYS: {len(port_list)} PORT(S) DETECTED")

    def update_mode(self):
        self.auto_save_mode = self.mode_var.get()
        self._set_status(f"MODE: {'NORMAL' if self.auto_save_mode == 'normal' else 'DEFECT'} SELECTED")

    def start(self):
        if not self.combo_port.get():
            messagebox.showwarning("WARNING", "NO SERIAL PORT SELECTED")
            return

        port = self.combo_port.get().split(" - ")[0]
        baud = int(self.combo_baud.get())
        try:
            self.serial_port = serial.Serial(port, baud, timeout=1)
            time.sleep(2)
            self.is_running = True
            self.data_buffer = []
            self.start_time = time.time()
            self.collect_start_time = time.time()
            self.sample_count = 0
            self.samples_in_last_second = 0
            self.last_rate_time = time.time()
            self.file_index = 0

            self.btn_start.config(state="disabled", fg=C["fg_dim"])
            self.btn_stop.config(state="normal", fg=C["red"])

            mode_name = "NORMAL" if self.auto_save_mode == "normal" else "DEFECT"
            self._set_status(f"● ACQUIRING [{mode_name}]  PORT:{port}")

            threading.Thread(target=self.collect_loop, daemon=True).start()
            self.update_timer()
            self.update_rate()
            if self.auto_save_var.get():
                self.schedule_auto_save()

            self.log("=" * 60)
            self.log(f"  ACQUISITION START  {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
            self.log(f"  MODE : {mode_name}  |  PORT : {port}  |  BAUD : {baud}")
            self.log("=" * 60)
        except Exception as e:
            messagebox.showerror("CONNECTION ERROR", str(e))

    def collect_loop(self):
        while self.is_running and self.serial_port and self.serial_port.is_open:
            try:
                if self.serial_port.in_waiting:
                    line = self.serial_port.readline().decode("utf-8", errors="ignore").strip()
                    if not line or line[0] in ("=", "M", "t"):
                        continue
                    values = line.split(",")
                    if len(values) == 9:
                        try:
                            data = [float(v) for v in values]
                            ax, ay, az, gx, gy, gz, cur, vol, pwr = data
                            t = round(time.time() - self.collect_start_time, 3)
                            self.data_buffer.append([t, ax, ay, az, gx, gy, gz, cur, vol, pwr, self.auto_save_mode])
                            self.sample_count += 1
                            self.samples_in_last_second += 1
                            self.root.after(0, self.update_display, t, ax, ay, az, gx, gy, gz, cur, vol, pwr)
                        except ValueError:
                            pass
                time.sleep(0.001)
            except Exception:
                break

    def update_display(self, t, ax, ay, az, gx, gy, gz, cur, vol, pwr):
        self.lbl_accel.config(text=f"X:{ax:6.0f} Y:{ay:6.0f} Z:{az:6.0f}")
        self.lbl_gyro.config(text=f"X:{gx:6.0f} Y:{gy:6.0f} Z:{gz:6.0f}")
        self.lbl_current.config(text=f"{cur:.3f} A")
        self.lbl_voltage.config(text=f"{vol:.2f} V")
        self.lbl_power.config(text=f"{pwr:.2f} W")
        self.lbl_samples.config(text=f"{len(self.data_buffer):,}")
        if len(self.data_buffer) % 100 == 0:
            self.log(
                f"  {t:8.3f} | {ax:7.0f} {ay:7.0f} {az:7.0f} | "
                f"{cur:7.3f}A {vol:6.2f}V {pwr:6.2f}W"
            )

    def update_timer(self):
        if self.is_running and self.app_mode.get() == "collect":
            elapsed = int(time.time() - self.start_time)
            h, m, s = elapsed // 3600, (elapsed % 3600) // 60, elapsed % 60
            self.lbl_time.config(text=f"{h:02d}:{m:02d}:{s:02d}")
            if self.auto_save_var.get():
                ns = 120 - (elapsed % 120)
                self.lbl_next_save.config(text=f"{ns // 60:02d}:{ns % 60:02d}")
            else:
                self.lbl_next_save.config(text="--:--")
            self.root.after(1000, self.update_timer)

    def update_rate(self):
        if self.is_running and self.app_mode.get() == "collect":
            now = time.time()
            diff = now - self.last_rate_time
            if diff >= 1.0:
                hz = self.samples_in_last_second / diff
                self.lbl_rate.config(text=f"{hz:.1f} Hz")
                self.samples_in_last_second = 0
                self.last_rate_time = now
            self.root.after(100, self.update_rate)

    def schedule_auto_save(self):
        if self.is_running and self.auto_save_var.get():
            self.auto_save()
            self.auto_save_timer = self.root.after(120000, self.schedule_auto_save)

    def auto_save(self):
        if not self.data_buffer:
            return
        ts = datetime.now().strftime("%Y%m%d_%H%M%S")
        self.file_index += 1
        fname = f"{self.auto_save_mode}_{ts}_part{self.file_index}.csv"
        try:
            with open(fname, "w", newline="") as f:
                w = csv.writer(f)
                w.writerow(["time_s", "ax", "ay", "az", "gx", "gy", "gz",
                            "current", "voltage", "power", "label"])
                w.writerows(self.data_buffer)
            self.log(f"  [AUTO SAVE] >> {fname}  ({len(self.data_buffer):,} samples)")
        except Exception as e:
            self.log(f"  [ERROR] {e}")

    def manual_save(self):
        if not self.data_buffer:
            messagebox.showwarning("WARNING", "NO DATA TO SAVE")
            return
        ts = datetime.now().strftime("%Y%m%d_%H%M%S")
        fname = f"{self.auto_save_mode}_manual_{ts}.csv"
        try:
            with open(fname, "w", newline="") as f:
                w = csv.writer(f)
                w.writerow(["time_s", "ax", "ay", "az", "gx", "gy", "gz",
                            "current", "voltage", "power", "label"])
                w.writerows(self.data_buffer)
            messagebox.showinfo("SAVED", f"FILE : {fname}\nSAMPLES : {len(self.data_buffer):,}")
            self.log(f"  [MANUAL SAVE] >> {fname}")
        except Exception as e:
            messagebox.showerror("ERROR", str(e))

    def stop(self):
        self.is_running = False
        if self.auto_save_timer:
            try:
                self.root.after_cancel(self.auto_save_timer)
            except Exception:
                pass
            self.auto_save_timer = None
        if self.serial_port and self.serial_port.is_open:
            try:
                self.serial_port.close()
            except Exception:
                pass

        self.btn_start.config(state="normal", fg=C["green"])
        self.btn_stop.config(state="disabled", fg=C["fg_dim"])
        self._set_status("SYS: ACQUISITION HALTED")
        self.log(f"  [STOP] TOTAL {len(self.data_buffer):,} SAMPLES COLLECTED")
        self.log("=" * 60)

    def log(self, msg):
        self.text_log.insert("end", f"{msg}\n")
        self.text_log.see("end")
        lines = self.text_log.get("1.0", "end").split("\n")
        if len(lines) > 1000:
            self.text_log.delete("1.0", "500.0")

    # ══════════════════════════════════════════
    # INFER + CSV
    # ══════════════════════════════════════════
    def _infer_make_filename(self, port: str) -> str:
        ts = datetime.now().strftime("%Y%m%d_%H%M%S")
        safe_port = re.sub(r"[^A-Za-z0-9_-]", "_", port)
        return f"infer_{safe_port}_{ts}.csv"

    def _infer_append_row(self, score: float, label: int, raw_line: str):
        if self.infer_session_start is None:
            return
        elapsed = time.time() - self.infer_session_start
        votes = ",".join(str(v) for v in self.vote_history)
        self.infer_rows.append([
            datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
            f"{elapsed:.3f}",
            f"{score:.6f}",
            int(label),
            self.current_status,
            votes,
            raw_line
        ])

    def save_infer_csv(self, manual: bool = True):
        if not self.infer_rows:
            if manual:
                messagebox.showwarning("WARNING", "NO INFER DATA TO SAVE")
            return

        fname = self.infer_csv_name or f"infer_{datetime.now().strftime('%Y%m%d_%H%M%S')}.csv"
        try:
            with open(fname, "w", newline="", encoding="utf-8") as f:
                w = csv.writer(f)
                w.writerow(["timestamp", "elapsed_s", "score", "label",
                            "final_status", "vote_window", "raw"])
                w.writerows(self.infer_rows)

            self._set_status(f"SYS: INFER CSV SAVED -> {os.path.basename(fname)}")
            if manual:
                messagebox.showinfo("SAVED", f"FILE : {fname}\nROWS : {len(self.infer_rows):,}")
            self.infer_log(f"  [CSV SAVE] {fname}  rows={len(self.infer_rows):,}", "sys")
        except Exception as e:
            if manual:
                messagebox.showerror("ERROR", str(e))
            self.infer_log(f"  [CSV ERROR] {e}", "sys")

    def start_infer(self):
        if not self.combo_port_infer.get():
            messagebox.showwarning("WARNING", "NO SERIAL PORT SELECTED")
            return

        port = self.combo_port_infer.get().split(" - ")[0]
        baud = int(self.combo_baud_infer.get())
        try:
            self.serial_port = serial.Serial(port, baud, timeout=1)
            time.sleep(1)
            self.is_running = True
            self.infer_count = 0
            self.defect_count = 0
            self.vote_history = []
            self.last_score = 0.0
            self.current_status = "WAITING"

            self.infer_rows = []
            self.infer_session_start = time.time()
            self.infer_csv_name = self._infer_make_filename(port)

            self.btn_infer_start.config(state="disabled", fg=C["fg_dim"])
            self.btn_infer_stop.config(state="normal", fg=C["red"])
            self.btn_infer_save.config(state="normal", fg=C["amber"])

            self._set_status(f"● INFER ACTIVE  PORT:{port}  -> CSV:{os.path.basename(self.infer_csv_name)}")
            self._update_status_ui("WAITING", 0.0, [])
            self.infer_log("=" * 54, "sys")
            self.infer_log(f"  INFER SESSION START  {datetime.now().strftime('%H:%M:%S')}", "sys")
            self.infer_log(f"  PORT:{port}  BAUD:{baud}", "sys")
            self.infer_log(f"  THRESHOLD:{self.THRESHOLD:.3f}", "sys")
            self.infer_log(f"  CSV:{self.infer_csv_name}", "sys")
            self.infer_log("=" * 54, "sys")

            threading.Thread(target=self.infer_loop, daemon=True).start()
        except Exception as e:
            messagebox.showerror("CONNECTION ERROR", str(e))

    def infer_loop(self):
        while self.is_running and self.serial_port and self.serial_port.is_open:
            try:
                if self.serial_port.in_waiting:
                    line = self.serial_port.readline().decode("utf-8", errors="ignore").strip()
                    if not line:
                        continue
                    self.root.after(0, self._parse_infer_line, line)
                time.sleep(0.001)
            except Exception:
                break

    def _parse_infer_line(self, line):
        if line.startswith("AI:"):
            try:
                parts = line.replace("AI:", "").strip().split("->")
                score = float(parts[0].strip())
                label = int(parts[1].strip())

                self.infer_count += 1
                if label == 1:
                    self.defect_count += 1
                self.vote_history.append(label)
                if len(self.vote_history) > 5:
                    self.vote_history.pop(0)
                self.last_score = score

                if label == 0:
                    msg = f"  AI 추론 결과   score = {score:.4f}   →   정상입니다 ✔"
                    tag = "normal"
                else:
                    msg = f"  AI 추론 결과   score = {score:.4f}   →   결함 감지! ✖"
                    tag = "defect"
                self.infer_log(msg, tag)

                self._refresh_infer_ui()
                self._infer_append_row(score, label, line)
            except (ValueError, IndexError):
                self.infer_log(f"  {line}", "sys")

        elif line.startswith("RESULT,"):
            parts = line.split(",")
            if len(parts) >= 3:
                try:
                    label = int(parts[1])
                    score = float(parts[2])

                    self.infer_count += 1
                    if label == 1:
                        self.defect_count += 1
                    self.vote_history.append(label)
                    if len(self.vote_history) > 5:
                        self.vote_history.pop(0)
                    self.last_score = score

                    if label == 0:
                        self.infer_log(f"  AI 추론 결과   score={score:.4f}   →   정상입니다 ✔", "normal")
                    else:
                        self.infer_log(f"  AI 추론 결과   score={score:.4f}   →   결함 감지! ✖", "defect")

                    self._refresh_infer_ui()
                    self._infer_append_row(score, label, line)
                except ValueError:
                    pass

        elif line.startswith("STATUS,"):
            status = line.split(",")[1].strip() if "," in line else "UNKNOWN"
            self._update_status_ui(status, self.last_score, self.vote_history)
            self.infer_log(f"  [STATUS] {status}", "sys")

        elif line in ("NORMAL", "** NORMAL **"):
            self._update_status_ui("NORMAL", self.last_score, self.vote_history)
            self.infer_log("  [STATUS] NORMAL", "normal")
        elif line in ("DEFECT", "!! DEFECT !!"):
            self._update_status_ui("DEFECT", self.last_score, self.vote_history)
            self.infer_log("  [STATUS] DEFECT CONFIRMED", "defect")
        elif "DETECT" in line:
            self._update_status_ui("DETECTING", self.last_score, self.vote_history)
            self.infer_log("  [STATUS] DETECTING...", "detect")

        # ✅ score 단독 입력은 threshold=0.6로 판정
        elif re.match(r"^0?\.\d+$|^[01]\.\d+$", line):
            try:
                score = float(line)
                self.last_score = score
                label = 1 if score >= self.THRESHOLD else 0

                self.infer_count += 1
                if label == 1:
                    self.defect_count += 1
                self.vote_history.append(label)
                if len(self.vote_history) > 5:
                    self.vote_history.pop(0)

                if label == 0:
                    self.infer_log(f"  AI 추론 결과   score={score:.4f}   →   정상입니다 ✔", "normal")
                else:
                    self.infer_log(f"  AI 추론 결과   score={score:.4f}   →   결함 감지! ✖", "defect")

                self._refresh_infer_ui()
                self._infer_append_row(score, label, line)
            except ValueError:
                pass

        else:
            self.infer_log(f"  {line}", "sys")

    def _refresh_infer_ui(self):
        defect_votes = sum(self.vote_history)
        total = len(self.vote_history)
        if total == 0:
            status = "WAITING"
        elif defect_votes >= 3:
            status = "DEFECT"
        elif defect_votes >= 1:
            status = "DETECTING"
        else:
            status = "NORMAL"

        self.current_status = status
        self._update_status_ui(status, self.last_score, self.vote_history)

    def _update_status_ui(self, status, score, votes):
        cfg = {
            "NORMAL": (C["green"], "■", "NORMAL", "> 모터 정상 작동 중입니다"),
            "DETECTING": (C["yellow"], "◆", "DETECTING...", f"> 이상 징후 감지 중  score={score:.4f}"),
            "DEFECT": (C["red"], "✖", "!! FAULT !!", f"> 결함 확정  anomaly score={score:.4f}"),
            "WAITING": (C["fg_dim"], "■", "STANDBY", "> 시리얼 포트 연결 후 START INFER 를 누르세요"),
        }
        color, icon, title, sub = cfg.get(status, cfg["WAITING"])

        self.status_indicator.config(text=icon, fg=color)
        self.lbl_status_main.config(text=title, fg=color)
        self.lbl_status_sub.config(text=sub, fg=C["fg_dim"])
        self.lbl_status_time.config(text=f"LAST UPDATE : {datetime.now().strftime('%H:%M:%S')}")

        self.score_canvas.update_idletasks()
        w = self.score_canvas.winfo_width()
        if w > 1:
            self.score_canvas.delete("all")
            for i in range(1, 5):
                x = int(w * i * 0.2)
                self.score_canvas.create_line(x, 0, x, 22, fill=C["border"], width=1)

            bar_w = int(w * min(score, 1.0))
            # 색은 그대로: 저/중/고 (0.6 이상이면 붉게 느낌 확)
            bar_color = (C["green"] if score < 0.3 else (C["yellow"] if score < self.THRESHOLD else C["red"]))
            if bar_w > 0:
                self.score_canvas.create_rectangle(0, 2, bar_w, 20, fill=bar_color, outline="")
            self.score_canvas.create_text(
                w // 2, 11, text=f"{score:.4f}",
                font=self.F_MONO_8B, fill=C["fg_bright"]
            )
        self.lbl_score_val.config(text=f"{score:.4f}", fg=color)

        for i, box in enumerate(self.vote_boxes):
            if i < len(votes):
                v = votes[i]
                if v == 1:
                    box.config(bg=C["red_dim"], fg=C["red"], text=f"  {i + 1}  \n-----\n  ✖  ")
                else:
                    box.config(bg=C["green_dim"], fg=C["green"], text=f"  {i + 1}  \n-----\n  ✔  ")
            else:
                box.config(bg=C["card2"], fg=C["fg_dim"], text=f"  {i + 1}  \n-----\n  ?  ")

        total = self.infer_count
        defect = self.defect_count
        rate = (defect / total * 100) if total > 0 else 0.0
        self.lbl_infer_total.config(text=str(total), fg=C["amber"])
        self.lbl_infer_defect.config(text=str(defect), fg=C["red"] if defect > 0 else C["fg"])
        self.lbl_infer_rate.config(text=f"{rate:.1f}%", fg=C["red"] if rate > 20 else C["green"])

    def stop_infer(self):
        self.is_running = False
        if self.serial_port and self.serial_port.is_open:
            try:
                self.serial_port.close()
            except Exception:
                pass

        self.btn_infer_start.config(state="normal", fg=C["green"])
        self.btn_infer_stop.config(state="disabled", fg=C["fg_dim"])
        self.btn_infer_save.config(state="disabled", fg=C["fg_dim"])

        self.save_infer_csv(manual=False)

        self._update_status_ui("WAITING", 0.0, [])
        self._set_status("SYS: INFER SESSION TERMINATED")
        self.infer_log("=" * 54, "sys")
        self.infer_log(
            f"  INFER STOP  {datetime.now().strftime('%H:%M:%S')}"
            f"  |  TOTAL:{self.infer_count}  FAULT:{self.defect_count}",
            "sys"
        )
        self.infer_log("=" * 54, "sys")

    def infer_log(self, msg, tag="sys"):
        ts = datetime.now().strftime("%H:%M:%S")
        self.text_infer_log.insert("end", f"[{ts}]", "ts")
        self.text_infer_log.insert("end", f"{msg}\n", tag)
        self.text_infer_log.see("end")
        lines = self.text_infer_log.get("1.0", "end").split("\n")
        if len(lines) > 600:
            self.text_infer_log.delete("1.0", "200.0")

    def _set_status(self, msg):
        self.status_bar.config(text=f"  {msg}")


if __name__ == "__main__":
    root = tk.Tk()
    app = MotorMonitor(root)
    root.mainloop()