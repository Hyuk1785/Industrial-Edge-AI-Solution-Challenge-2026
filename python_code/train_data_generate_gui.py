import tkinter as tk
from tkinter import ttk, scrolledtext, messagebox
import serial
import serial.tools.list_ports
import csv
from datetime import datetime
import threading
import time
import re

class MotorMonitor:
    def __init__(self, root):
        self.root = root
        self.root.title("Motor Anomaly Detection - Data Collector")
        self.root.geometry("900x650")
        self.root.configure(bg='#2b2b2b')

        self.setup_style()

        self.serial_port = None
        self.is_running = False
        self.data_buffer = []
        self.start_time = None
        self.auto_save_mode = "normal"
        self.auto_save_timer = None
        self.sample_count = 0
        self.last_sample_time = time.time()
        self.collect_start_time = None

        self.create_widgets()
        self.refresh_ports()

    def setup_style(self):
        style = ttk.Style()
        style.theme_use('default')

        self.bg_dark   = '#2b2b2b'
        self.bg_medium = '#3c3f41'
        self.bg_light  = '#4e5254'
        self.fg_normal = '#bbbbbb'
        self.fg_bright = '#ffffff'

        style.configure('Dark.TCombobox',
                        fieldbackground=self.bg_medium,
                        background=self.bg_medium,
                        foreground=self.fg_normal)

    def create_widgets(self):
        # ===== 헤더 =====
        header = tk.Frame(self.root, bg='#1e1e1e', height=50)
        header.pack(fill='x')
        tk.Label(header,
                 text="MOTOR ANOMALY DETECTION - DATA ACQUISITION SYSTEM",
                 font=('Arial', 14, 'bold'),
                 bg='#1e1e1e', fg='#ffffff').pack(pady=12)

        # ===== 메인 =====
        main = tk.Frame(self.root, bg=self.bg_dark)
        main.pack(fill='both', expand=True, padx=10, pady=10)

        # ===== 좌측 =====
        left = tk.Frame(main, bg=self.bg_dark)
        left.pack(side='left', fill='y', padx=(0, 10))

        # CONNECTION
        conn = tk.LabelFrame(left, text="CONNECTION",
                             bg=self.bg_dark, fg=self.fg_bright,
                             font=('Arial', 9, 'bold'), bd=1, relief='solid')
        conn.pack(fill='x', pady=(0, 8))

        tk.Label(conn, text="Port:", bg=self.bg_dark,
                 fg=self.fg_normal, font=('Arial', 9)).grid(
                     row=0, column=0, sticky='w', padx=5, pady=5)
        self.combo_port = ttk.Combobox(conn, width=22, state='readonly')
        self.combo_port.grid(row=0, column=1, padx=5, pady=5)
        tk.Button(conn, text="Refresh", command=self.refresh_ports,
                  bg=self.bg_light, fg=self.fg_normal,
                  relief='flat', font=('Arial', 9)).grid(
                      row=0, column=2, padx=5, pady=5)

        tk.Label(conn, text="Baud:", bg=self.bg_dark,
                 fg=self.fg_normal, font=('Arial', 9)).grid(
                     row=1, column=0, sticky='w', padx=5, pady=5)
        self.combo_baud = ttk.Combobox(conn, width=10, state='readonly',
                                       values=['115200', '230400'])
        self.combo_baud.set('115200')
        self.combo_baud.grid(row=1, column=1, sticky='w', padx=5, pady=5)

        # COLLECTION MODE
        mode = tk.LabelFrame(left, text="COLLECTION MODE",
                             bg=self.bg_dark, fg=self.fg_bright,
                             font=('Arial', 9, 'bold'), bd=1, relief='solid')
        mode.pack(fill='x', pady=(0, 8))

        self.mode_var = tk.StringVar(value="normal")
        tk.Radiobutton(mode, text="Normal Data",
                       variable=self.mode_var, value="normal",
                       bg=self.bg_dark, fg=self.fg_normal,
                       selectcolor=self.bg_medium, font=('Arial', 9),
                       activebackground=self.bg_dark,
                       command=self.update_mode).pack(
                           anchor='w', padx=10, pady=5)
        tk.Radiobutton(mode, text="Defect Data",
                       variable=self.mode_var, value="defect",
                       bg=self.bg_dark, fg=self.fg_normal,
                       selectcolor=self.bg_medium, font=('Arial', 9),
                       activebackground=self.bg_dark,
                       command=self.update_mode).pack(
                           anchor='w', padx=10, pady=5)

        # AUTO SAVE
        save = tk.LabelFrame(left, text="AUTO SAVE",
                             bg=self.bg_dark, fg=self.fg_bright,
                             font=('Arial', 9, 'bold'), bd=1, relief='solid')
        save.pack(fill='x', pady=(0, 8))

        tk.Label(save, text="Interval: 5 minutes",
                 bg=self.bg_dark, fg=self.fg_normal,
                 font=('Arial', 9)).pack(padx=10, pady=5)
        self.auto_save_var = tk.BooleanVar(value=True)
        tk.Checkbutton(save, text="Enable Auto Save",
                       variable=self.auto_save_var,
                       bg=self.bg_dark, fg=self.fg_normal,
                       selectcolor=self.bg_medium,
                       activebackground=self.bg_dark,
                       font=('Arial', 9)).pack(anchor='w', padx=10, pady=5)

        # CONTROL
        ctrl = tk.LabelFrame(left, text="CONTROL",
                             bg=self.bg_dark, fg=self.fg_bright,
                             font=('Arial', 9, 'bold'), bd=1, relief='solid')
        ctrl.pack(fill='x', pady=(0, 8))

        self.btn_start = tk.Button(ctrl, text="START",
                                   command=self.start,
                                   bg='#3a3a3a', fg='#ffffff',
                                   relief='flat',
                                   font=('Arial', 11, 'bold'), height=2,
                                   activebackground='#4a4a4a')
        self.btn_start.pack(fill='x', padx=10, pady=5)

        self.btn_stop = tk.Button(ctrl, text="STOP",
                                  command=self.stop,
                                  bg='#3a3a3a', fg='#ffffff',
                                  relief='flat',
                                  font=('Arial', 11, 'bold'), height=2,
                                  state='disabled',
                                  activebackground='#4a4a4a')
        self.btn_stop.pack(fill='x', padx=10, pady=5)

        self.btn_save = tk.Button(ctrl, text="MANUAL SAVE",
                                  command=self.manual_save,
                                  bg='#3a3a3a', fg='#ffffff',
                                  relief='flat',
                                  font=('Arial', 11, 'bold'), height=2,
                                  activebackground='#4a4a4a')
        self.btn_save.pack(fill='x', padx=10, pady=5)

        # ===== 우측 =====
        right = tk.Frame(main, bg=self.bg_dark)
        right.pack(side='right', fill='both', expand=True)

        # STATISTICS
        stats = tk.LabelFrame(right, text="STATISTICS",
                              bg=self.bg_dark, fg=self.fg_bright,
                              font=('Arial', 9, 'bold'), bd=1, relief='solid')
        stats.pack(fill='x', pady=(0, 8))

        sg = tk.Frame(stats, bg=self.bg_dark)
        sg.pack(fill='x', padx=10, pady=8)

        for col, (label, attr, init) in enumerate([
            ("Samples:", "lbl_samples", "0"),
            ("Rate:",    "lbl_rate",    "0 Hz"),
            ("Elapsed:", "lbl_time",    "00:00:00"),
            ("Next Save:","lbl_next_save","--:--"),
        ]):
            tk.Label(sg, text=label, bg=self.bg_dark,
                     fg=self.fg_normal,
                     font=('Arial', 9)).grid(
                         row=0, column=col*2, sticky='w', padx=5)
            lbl = tk.Label(sg, text=init, bg=self.bg_dark,
                           fg='#ffffff', font=('Arial', 13, 'bold'))
            lbl.grid(row=0, column=col*2+1, sticky='w', padx=5)
            setattr(self, attr, lbl)

        # CURRENT VALUES
        vals = tk.LabelFrame(right, text="CURRENT VALUES",
                             bg=self.bg_dark, fg=self.fg_bright,
                             font=('Arial', 9, 'bold'), bd=1, relief='solid')
        vals.pack(fill='x', pady=(0, 8))

        vg = tk.Frame(vals, bg=self.bg_dark)
        vg.pack(fill='x', padx=10, pady=8)

        # MPU6050
        tk.Label(vg, text="Accel (g):",
                 bg=self.bg_dark, fg=self.fg_normal,
                 font=('Arial', 9, 'bold')).grid(
                     row=0, column=0, sticky='w')
        self.lbl_accel = tk.Label(vg,
                                  text="X:      0  Y:      0  Z:      0",
                                  bg=self.bg_dark, fg='#bbbbbb',
                                  font=('Courier', 10))
        self.lbl_accel.grid(row=0, column=1, sticky='w', padx=10)

        tk.Label(vg, text="Gyro (°/s):",
                 bg=self.bg_dark, fg=self.fg_normal,
                 font=('Arial', 9, 'bold')).grid(
                     row=1, column=0, sticky='w', pady=(4,0))
        self.lbl_gyro = tk.Label(vg,
                                 text="X:      0  Y:      0  Z:      0",
                                 bg=self.bg_dark, fg='#bbbbbb',
                                 font=('Courier', 10))
        self.lbl_gyro.grid(row=1, column=1, sticky='w', padx=10, pady=(4,0))

        # INA219 구분선
        tk.Frame(vg, bg=self.bg_light, height=1).grid(
            row=2, column=0, columnspan=2, sticky='ew', pady=6)

        tk.Label(vg, text="Current:",
                 bg=self.bg_dark, fg=self.fg_normal,
                 font=('Arial', 9, 'bold')).grid(
                     row=3, column=0, sticky='w')
        self.lbl_current = tk.Label(vg, text="0.000 A",
                                    bg=self.bg_dark, fg='#bbbbbb',
                                    font=('Courier', 10))
        self.lbl_current.grid(row=3, column=1, sticky='w', padx=10)

        tk.Label(vg, text="Voltage:",
                 bg=self.bg_dark, fg=self.fg_normal,
                 font=('Arial', 9, 'bold')).grid(
                     row=4, column=0, sticky='w', pady=(4,0))
        self.lbl_voltage = tk.Label(vg, text="0.00 V",
                                    bg=self.bg_dark, fg='#bbbbbb',
                                    font=('Courier', 10))
        self.lbl_voltage.grid(row=4, column=1, sticky='w', padx=10, pady=(4,0))

        tk.Label(vg, text="Power:",
                 bg=self.bg_dark, fg=self.fg_normal,
                 font=('Arial', 9, 'bold')).grid(
                     row=5, column=0, sticky='w', pady=(4,0))
        self.lbl_power = tk.Label(vg, text="0.00 W",
                                  bg=self.bg_dark, fg='#bbbbbb',
                                  font=('Courier', 10))
        self.lbl_power.grid(row=5, column=1, sticky='w', padx=10, pady=(4,0))

        # DATA LOG
        log_frame = tk.LabelFrame(right, text="DATA LOG",
                                  bg=self.bg_dark, fg=self.fg_bright,
                                  font=('Arial', 9, 'bold'),
                                  bd=1, relief='solid')
        log_frame.pack(fill='both', expand=True)

        self.text_log = scrolledtext.ScrolledText(
            log_frame, height=12,
            font=('Courier', 8),
            bg='#1e1e1e', fg='#bbbbbb',
            insertbackground='#ffffff',
            selectbackground='#3a3a3a')
        self.text_log.pack(fill='both', expand=True, padx=5, pady=5)

        # ===== 상태바 =====
        self.status_bar = tk.Label(
            self.root, text="Ready",
            bd=1, relief='sunken', anchor='w',
            bg='#1e1e1e', fg='#bbbbbb', font=('Arial', 9))
        self.status_bar.pack(side='bottom', fill='x')

    # ── 포트 새로고침 ──
    def refresh_ports(self):
        ports = serial.tools.list_ports.comports()
        port_list = [f"{p.device} - {p.description}" for p in ports]
        self.combo_port['values'] = port_list
        if port_list:
            self.combo_port.current(0)
        self.status_bar.config(text=f"Found {len(port_list)} port(s)")

    def update_mode(self):
        self.auto_save_mode = self.mode_var.get()
        self.status_bar.config(
            text=f"Mode: {'NORMAL' if self.auto_save_mode == 'normal' else 'DEFECT'}")

    # ── 수집 시작 ──
    def start(self):
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
            self.last_sample_time = time.time()

            self.btn_start.config(state='disabled', bg='#2a2a2a')
            self.btn_stop.config(state='normal',   bg='#3a3a3a')

            mode_name = "NORMAL" if self.auto_save_mode == "normal" else "DEFECT"
            self.status_bar.config(text=f"Collecting {mode_name} from {port}")

            threading.Thread(target=self.collect_loop, daemon=True).start()
            self.update_timer()
            self.update_rate()

            if self.auto_save_var.get():
                self.schedule_auto_save()

            self.log(f"=== START: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')} ===")
            self.log(f"Mode: {mode_name}")
            self.log(f"{'time_s':>8} | {'ax':>7} {'ay':>7} {'az':>7} | "
                     f"{'gx':>7} {'gy':>7} {'gz':>7} | "
                     f"{'cur(A)':>7} {'vol(V)':>6} {'pwr(W)':>6}")
            self.log("-" * 85)

        except Exception as e:
            messagebox.showerror("Error", f"Failed to open port:\n{str(e)}")

    # ── 수집 루프 ──
    def collect_loop(self):
        while self.is_running and self.serial_port and self.serial_port.is_open:
            try:
                if self.serial_port.in_waiting:
                    line = self.serial_port.readline().decode(
                        'utf-8', errors='ignore').strip()

                    # CSV 형식 파싱
                    # 형식: ax,ay,az,gx,gy,gz,current,voltage,power
                    values = line.split(',')
                    if len(values) == 9:
                        try:
                            data = [float(v) for v in values]
                            ax, ay, az, gx, gy, gz, cur, vol, pwr = data

                            # 경과 시간 추가
                            t = round(time.time() - self.collect_start_time, 3)

                            # 저장 (timestamp 포함)
                            self.data_buffer.append([t, ax, ay, az,
                                                     gx, gy, gz,
                                                     cur, vol, pwr])
                            self.sample_count += 1

                            self.root.after(0, self.update_display,
                                           t, ax, ay, az,
                                           gx, gy, gz,
                                           cur, vol, pwr)
                        except ValueError:
                            pass

                time.sleep(0.001)

            except Exception as e:
                print(f"Collection error: {e}")
                break

    # ── 화면 업데이트 ──
    def update_display(self, t, ax, ay, az, gx, gy, gz, cur, vol, pwr):
        # MPU6050
        self.lbl_accel.config(
            text=f"X:{ax:7.0f}  Y:{ay:7.0f}  Z:{az:7.0f}")
        self.lbl_gyro.config(
            text=f"X:{gx:7.0f}  Y:{gy:7.0f}  Z:{gz:7.0f}")

        # INA219
        self.lbl_current.config(text=f"{cur:.3f} A")
        self.lbl_voltage.config(text=f"{vol:.2f} V")
        self.lbl_power.config(text=f"{pwr:.2f} W")

        # 샘플 개수
        self.lbl_samples.config(text=f"{len(self.data_buffer):,}")

        # 로그 (100개마다)
        if len(self.data_buffer) % 100 == 0:
            self.log(f"{t:8.3f} | "
                     f"{ax:7.0f} {ay:7.0f} {az:7.0f} | "
                     f"{gx:7.0f} {gy:7.0f} {gz:7.0f} | "
                     f"{cur:7.3f} {vol:6.2f} {pwr:6.2f}")

    # ── 타이머 ──
    def update_timer(self):
        if self.is_running:
            elapsed = int(time.time() - self.start_time)
            h = elapsed // 3600
            m = (elapsed % 3600) // 60
            s = elapsed % 60
            self.lbl_time.config(text=f"{h:02d}:{m:02d}:{s:02d}")

            if self.auto_save_var.get():
                ns = 300 - (elapsed % 300)
                self.lbl_next_save.config(
                    text=f"{ns//60:02d}:{ns%60:02d}")
            else:
                self.lbl_next_save.config(text="--:--")

            self.root.after(1000, self.update_timer)

    # ── 샘플링 속도 ──
    def update_rate(self):
        if self.is_running:
            now = time.time()
            diff = now - self.last_sample_time
            if diff >= 1.0:
                self.lbl_rate.config(
                    text=f"{self.sample_count / diff:.1f} Hz")
                self.sample_count = 0
                self.last_sample_time = now
            self.root.after(100, self.update_rate)

    # ── 자동 저장 ──
    def schedule_auto_save(self):
        if self.is_running and self.auto_save_var.get():
            self.auto_save()
            self.auto_save_timer = self.root.after(
                300000, self.schedule_auto_save)

    def auto_save(self):
        if not self.data_buffer:
            return
        ts = datetime.now().strftime('%Y%m%d_%H%M%S')
        fname = f"{self.auto_save_mode}_{ts}.csv"
        try:
            with open(fname, 'w', newline='') as f:
                w = csv.writer(f)
                w.writerow(['time_s', 'ax', 'ay', 'az',
                            'gx', 'gy', 'gz',
                            'current', 'voltage', 'power'])
                w.writerows(self.data_buffer)
            self.log(f"[AUTO SAVE] {fname} - {len(self.data_buffer):,} samples")
            self.status_bar.config(text=f"Auto saved: {fname}")
            self.data_buffer = []
        except Exception as e:
            self.log(f"[ERROR] Auto save failed: {e}")

    # ── 수동 저장 ──
    def manual_save(self):
        if not self.data_buffer:
            messagebox.showwarning("Warning", "No data to save")
            return
        ts = datetime.now().strftime('%Y%m%d_%H%M%S')
        fname = f"{self.auto_save_mode}_manual_{ts}.csv"
        try:
            with open(fname, 'w', newline='') as f:
                w = csv.writer(f)
                w.writerow(['time_s', 'ax', 'ay', 'az',
                            'gx', 'gy', 'gz',
                            'current', 'voltage', 'power'])
                w.writerows(self.data_buffer)
            messagebox.showinfo(
                "Saved",
                f"File: {fname}\nSamples: {len(self.data_buffer):,}")
            self.log(f"[MANUAL SAVE] {fname} - {len(self.data_buffer):,} samples")
        except Exception as e:
            messagebox.showerror("Error", f"Save failed:\n{str(e)}")

    # ── 정지 ──
    def stop(self):
        self.is_running = False
        if self.auto_save_timer:
            self.root.after_cancel(self.auto_save_timer)
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.close()
        self.btn_start.config(state='normal',   bg='#3a3a3a')
        self.btn_stop.config(state='disabled', bg='#2a2a2a')
        self.status_bar.config(text="Stopped")
        self.log(f"=== STOP: {len(self.data_buffer):,} samples in buffer ===")

    def log(self, msg):
        self.text_log.insert('end', f"{msg}\n")
        self.text_log.see('end')
        lines = self.text_log.get('1.0', 'end').split('\n')
        if len(lines) > 1000:
            self.text_log.delete('1.0', '500.0')


if __name__ == "__main__":
    root = tk.Tk()
    app = MotorMonitor(root)
    root.mainloop()