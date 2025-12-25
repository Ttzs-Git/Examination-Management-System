# app.py
import ttkbootstrap as ttk
from ttkbootstrap.constants import *
from tkinter import messagebox, scrolledtext, simpledialog
import socket
import threading
# 图形化交互终端

SERVER_IP = '127.0.0.1' # 服务器地址 
SERVER_PORT = 8888 # TCP端口
DELIMITER = "$$$" # 消息分隔符
PASSWORD_ADMIN = "123456" # 管理员密码

class ExamApp(ttk.Window):
    # 单窗口多视图: 一个窗口清空后切换不同界面
    def __init__(self):
        super().__init__(themename="cosmo")
        
        # 全局样式配置: 字体 风格 行高 
        # 统一字体配置
        style = ttk.Style()
        self.default_font = ('Microsoft YaHei UI', 10)
        self.header_font = ('Microsoft YaHei UI', 24, "bold")
        self.sub_header_font = ('Microsoft YaHei UI', 14, "bold")
        style.configure('.', font=self.default_font)
        style.configure('Treeview', rowheight=30, font=self.default_font)
        style.configure('Treeview.Heading', font=('Microsoft YaHei UI', 11, "bold"))
        
        # 窗口基本位置
        self.title("C语言智能考试系统")
        self.geometry("1100x700") 
        self.place_window_center()
        
        # 状态变量初始虎啊
        self.sock = None
        self.buffer = b""
        self.lbl_stats = None
        self.lbl_stats_admin = None
        self.tree = None 
        self.show_main_role_select()

    def place_window_center(self):
        # 窗口居中算法
        self.update_idletasks()
        w, h = self.winfo_width(), self.winfo_height()
        
        x = int((self.winfo_screenwidth()/2) - (w/2))
        y = int((self.winfo_screenheight()/2) - (h/2))
        self.geometry(f"{w}x{h}+{x}+{y}") # 应用新位置

    # 销毁所有子组件
    def clear_ui(self):
        for widget in self.winfo_children(): 
            widget.destroy()
            # 重置组件引用
        self.lbl_stats = None
        self.lbl_stats_admin = None
        self.tree = None

    # 网络通信模块
    ## 创建连接
    def create_connection(self):
        if self.sock: return True 
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.settimeout(3) # 3秒超时
            self.sock.connect((SERVER_IP, SERVER_PORT))
            self.sock.settimeout(None) # 连接后取消超时
            self.buffer = b""
            return True
        except Exception as e:
            messagebox.showerror("连接错误", f"无法连接到服务器: {e}")
            return False

    ## 发送数据包（添加分隔符）
    def send_packet(self, text):
        if self.sock:
            try: self.sock.sendall((text + DELIMITER).encode('utf-8'))
            except Exception as e: print(f"[Send Error] {e}")

    ## 接收数据包（处理分包）
    def recv_packet(self):
        while True:
            try:
                # 检查缓冲区里有没有完整的包
                if b"$$$" in self.buffer:
                    parts = self.buffer.split(b"$$$", 1) # 只切第一个包
                    msg = parts[0]
                    self.buffer = parts[1] # 剩余部分留在缓冲区
                    return msg.decode('utf-8', errors='ignore')
                if not self.sock: return None # 连接已关闭
                
                # 继续接受数据
                chunk = self.sock.recv(4096)
                if not chunk: return None
                self.buffer += chunk # 拼接到缓冲区末尾
            except Exception as e: return None

    def disconnect_and_home(self):
        if self.sock:
            try: self.sock.close()
            except: pass
            self.sock = None
        self.show_main_role_select()

    # 主界面 
    def show_main_role_select(self):
        self.clear_ui()
        ## 标题栏
        header = ttk.Frame(self, padding=30, bootstyle="primary")
        header.pack(fill=X)
        ttk.Label(header, text="C语言智能考试系统", font=self.header_font, foreground="white", background="#2780e3").pack()
        ## 容器使用Grid(网格)布局
        container = ttk.Frame(self, padding=20)
        container.pack(expand=True, fill=BOTH)
        ## 配置三列等宽
        container.columnconfigure(0, weight=1)
        container.columnconfigure(1, weight=1)
        container.columnconfigure(2, weight=1)
        container.rowconfigure(0, weight=1)
        ## 创建角色卡片
        self._create_role_card(container, 0, "学生入口", "答题 · 练习 · 查分", "success", "🎓", self.show_student_menu)
        self._create_role_card(container, 1, "教师入口", "发布考试 · 监控 · 考务", "warning", "👨‍🏫", self.enter_teacher_portal)
        self._create_role_card(container, 2, "管理员后台", "题库维护 · 名单管理", "danger", "🔧", self.enter_admin_portal)
        
        ttk.Label(self, text="© 2025 Intelligent Exam System", font=("Arial", 9), bootstyle="secondary").pack(side=BOTTOM, pady=10)

    def _create_role_card(self, parent, col_idx, title, subtitle, color, icon, command):
        card = ttk.Labelframe(parent, padding=20, bootstyle=f"{color}") 
        card.grid(row=0, column=col_idx, padx=20, pady=20, sticky="nsew")
        
        inner = ttk.Frame(card)
        inner.pack(expand=True)
        
        ttk.Label(inner, text=icon, font=("Segoe UI Emoji", 50)).pack(pady=(10, 10))
        ttk.Label(inner, text=title, font=("Microsoft YaHei UI", 20, "bold"), bootstyle=color).pack(pady=10)
        ttk.Label(inner, text=subtitle, font=("Microsoft YaHei UI", 11), bootstyle="secondary").pack(pady=(0, 20))
        ttk.Button(inner, text="进入系统", command=command, bootstyle=color, width=15, cursor="hand2").pack(side=BOTTOM, pady=10)

    # 教师模块
    ## 建立连接
    def enter_teacher_portal(self):
        if self.create_connection():
            self.show_teacher_dashboard()

    ## 仪表盘: 展示列表
    def show_teacher_dashboard(self):
        self.clear_ui()
        
        nav = ttk.Frame(self, padding=10, bootstyle="warning")
        nav.pack(fill=X)
        ttk.Button(nav, text="⬅ 返回", command=self.disconnect_and_home, bootstyle="light", width=8).pack(side=LEFT)
        ttk.Label(nav, text="教师考务中心", font=self.sub_header_font, foreground="white", background="#ff9800").pack(side=LEFT, padx=20)
        
        tool_frame = ttk.Frame(self, padding=10)
        tool_frame.pack(fill=X)
        
        grp = ttk.Labelframe(tool_frame, text="考务控制", padding=5, bootstyle="warning")
        grp.pack(side=LEFT, fill=Y, padx=10)
        ttk.Button(grp, text="🚀 发布考试", command=self.teacher_start_exam, bootstyle="success").pack(side=LEFT, padx=5)
        ttk.Button(grp, text="⚙️ 设定题数", command=self.teacher_set_count, bootstyle="info").pack(side=LEFT, padx=5)
        
        ttk.Button(tool_frame, text="🔄 刷新列表", command=self.common_refresh_list, bootstyle="secondary").pack(side=RIGHT, padx=10)

        content = ttk.Frame(self, padding=15)
        content.pack(fill=BOTH, expand=True)
        
        self.lbl_stats = ttk.Label(content, text="加载中...", bootstyle="secondary")
        self.lbl_stats.pack(anchor=W, pady=(0, 5))

        table_frame = ttk.Frame(content)
        table_frame.pack(fill=BOTH, expand=True)
        ### 创建表格
        cols = ("排名", "学号", "姓名", "状态", "成绩")
        self.tree = ttk.Treeview(table_frame, columns=cols, show="headings", selectmode="browse")
        ### 配置列宽
        self.tree.column("排名", width=60, anchor=CENTER)
        self.tree.column("学号", width=150, anchor=CENTER)
        self.tree.column("姓名", width=150, anchor=CENTER)
        self.tree.column("状态", width=120, anchor=CENTER)
        self.tree.column("成绩", width=100, anchor=CENTER)
        ### 设置列标题
        for c in cols: self.tree.heading(c, text=c)
        ### 添加滚动条
        vsb = ttk.Scrollbar(table_frame, orient=VERTICAL, command=self.tree.yview)
        hsb = ttk.Scrollbar(table_frame, orient=HORIZONTAL, command=self.tree.xview)
        self.tree.configure(yscroll=vsb.set, xscroll=hsb.set)
        
        self.tree.pack(side=LEFT, fill=BOTH, expand=True)
        vsb.pack(side=RIGHT, fill=Y)
        hsb.pack(side=BOTTOM, fill=X) 
        ### 创建菜单
        self.teacher_menu = ttk.Menu(self, tearoff=0)
        self.teacher_menu.add_command(label="🔓 允许重考", command=self.teacher_reset_student)
        ### 绑定右键事件
        self.tree.bind("<Button-3>", lambda e: self.teacher_menu.post(e.x_root, e.y_root))
        
        self.common_refresh_list()

    ## 发布考试
    def teacher_start_exam(self):
        if messagebox.askyesno("确认", "确定要发布考试吗？"):
            self.send_packet("ADMIN_START_EXAM")
            if self.recv_packet() == "OK": messagebox.showinfo("成功", "考试已开始！")
    ## 设定题数
    def teacher_set_count(self):
        num = simpledialog.askinteger("设置", "输入单次考试题数:", minvalue=1, maxvalue=50)
        if num:
            self.send_packet(f"ADMIN_SET_COUNT|{num}")
            if self.recv_packet()=="OK": messagebox.showinfo("成功", f"已设置为 {num} 题"); self.common_refresh_list()
    ## 充值学生状态
    def teacher_reset_student(self):
        item = self.tree.selection()
        if item:
            vals = self.tree.item(item, "values")
            if messagebox.askyesno("重置", f"确定重置 {vals[2]} 吗？"):
                self.send_packet(f"ADMIN_RESET_STU|{vals[1]}")
                if self.recv_packet()=="OK": self.common_refresh_list()

    # 管理员模块
    def enter_admin_portal(self):
        pwd = simpledialog.askstring("管理员验证", "密码:", show="*")
        if pwd == PASSWORD_ADMIN:
            if self.create_connection(): self.show_admin_maintenance()
        elif pwd: messagebox.showerror("错误", "密码错误")

    def show_admin_maintenance(self):
        self.clear_ui()
        
        nav = ttk.Frame(self, padding=10, bootstyle="danger")
        nav.pack(fill=X)
        ttk.Button(nav, text="⬅ 退出", command=self.disconnect_and_home, bootstyle="light", width=8).pack(side=LEFT)
        ttk.Label(nav, text="后台管理系统", font=self.sub_header_font, foreground="white", background="#d9534f").pack(side=LEFT, padx=20)
        
        ### 创建可调整大小的分栏
        paned = ttk.Panedwindow(self, orient=HORIZONTAL)
        paned.pack(fill=BOTH, expand=True, padx=10, pady=10)
        
          ### 左侧面板  
        left_panel = ttk.Frame(paned, padding=5)
        paned.add(left_panel, weight=1)
        
        notebook = ttk.Notebook(left_panel, bootstyle="primary")
        notebook.pack(fill=BOTH, expand=True)
        
        # Tab 1: 录入
        tab_q = ttk.Frame(notebook, padding=10)
        notebook.add(tab_q, text="录入新题")
        ttk.Label(tab_q, text="题干:", bootstyle="primary").pack(anchor=W)
        
        ## 创建多行文本框（带滚动条）
        self.q_text = scrolledtext.ScrolledText(tab_q, height=5, width=38, font=("Microsoft YaHei", 10))
        self.q_text.pack(fill=X, pady=5)
        
        self.q_opts = []
        for c in ['A', 'B', 'C', 'D']:
            f = ttk.Frame(tab_q); f.pack(fill=X, pady=2)
            ttk.Label(f, text=f"{c}:", width=3).pack(side=LEFT)
            e = ttk.Entry(f); e.pack(side=LEFT, fill=X, expand=True)
            self.q_opts.append(e)
        ttk.Label(tab_q, text="答案:", bootstyle="primary").pack(anchor=W, pady=(5,0))
        self.q_ans = ttk.Entry(tab_q); self.q_ans.pack(fill=X)
        ttk.Button(tab_q, text="保存题目", command=self.admin_add_question, bootstyle="primary").pack(fill=X, pady=15)

        # Tab 2: 名单
        tab_s = ttk.Frame(notebook, padding=10)
        notebook.add(tab_s, text="添加考生")
        ttk.Label(tab_s, text="学号:").pack(anchor=W); self.add_sid = ttk.Entry(tab_s); self.add_sid.pack(fill=X, pady=5)
        ttk.Label(tab_s, text="姓名:").pack(anchor=W); self.add_name = ttk.Entry(tab_s); self.add_name.pack(fill=X, pady=5)
        ttk.Button(tab_s, text="添加考生", command=self.admin_add_student, bootstyle="success").pack(fill=X, pady=15)

        # 右侧面板
        right_panel = ttk.Frame(paned, padding=5)
        paned.add(right_panel, weight=3) # 给予更多权重
        
        # 1. 顶部统计
        r_header = ttk.Frame(right_panel)
        r_header.pack(fill=X, side=TOP)
        self.lbl_stats_admin = ttk.Label(r_header, text="数据加载中...", bootstyle="danger")
        self.lbl_stats_admin.pack(side=LEFT)
        
        # 【核心修复2】优先布局底部按钮，防止被挤出
        btn_bar = ttk.Frame(right_panel)
        btn_bar.pack(fill=X, side=BOTTOM, pady=5)
        
        ttk.Button(btn_bar, text="🗑️ 删除选中", command=self.admin_delete_student, bootstyle="danger").pack(side=LEFT, fill=X, expand=True, padx=(0,5))
        ttk.Button(btn_bar, text="🔄 刷新列表", command=self.common_refresh_list, bootstyle="secondary").pack(side=RIGHT, fill=X, expand=True, padx=(5,0))
        
        # 3. 中间表格 (最后占用剩余空间)
        t_frame = ttk.Frame(right_panel)
        t_frame.pack(fill=BOTH, expand=True, side=TOP, pady=5)
        
        cols = ("排名", "学号", "姓名", "状态", "成绩")
        self.tree = ttk.Treeview(t_frame, columns=cols, show="headings")
        
        self.tree.column("排名", width=50, anchor=CENTER)
        self.tree.column("学号", width=120, anchor=CENTER)
        self.tree.column("姓名", width=120, anchor=CENTER)
        self.tree.column("状态", width=100, anchor=CENTER)
        self.tree.column("成绩", width=80, anchor=CENTER)
        
        for c in cols: self.tree.heading(c, text=c)

        vsb = ttk.Scrollbar(t_frame, orient=VERTICAL, command=self.tree.yview)
        self.tree.configure(yscroll=vsb.set)
        self.tree.pack(side=LEFT, fill=BOTH, expand=True)
        vsb.pack(side=RIGHT, fill=Y)
        
        self.admin_menu = ttk.Menu(self, tearoff=0)
        self.admin_menu.add_command(label="❌ 永久删除", command=self.admin_delete_student)
        self.tree.bind("<Button-3>", lambda e: self.admin_menu.post(e.x_root, e.y_root))
        
        self.common_refresh_list()

    def admin_add_question(self):
        content = self.q_text.get("1.0", END).strip().replace('\n', ' ').replace('|', ' ')
        opts = [e.get().strip().replace('|', ' ') for e in self.q_opts]
        ans = self.q_ans.get().strip().upper()
        if not content or not all(opts) or not ans: messagebox.showwarning("提示", "请填写完整"); return
        self.send_packet(f"ADMIN_ADD_QUE|{content}|{opts[0]}|{opts[1]}|{opts[2]}|{opts[3]}|{ans}")
        if self.recv_packet()=="OK":
            messagebox.showinfo("成功", "题目已录入")
            self.q_text.delete("1.0", END); [e.delete(0, END) for e in self.q_opts]; self.q_ans.delete(0, END)
            self.common_refresh_list()
        else: messagebox.showerror("失败", "录入失败")

    def admin_add_student(self):
        sid, name = self.add_sid.get(), self.add_name.get()
        if not sid or not name: return
        self.send_packet(f"ADMIN_ADD_STU|{sid}|{name}")
        if self.recv_packet()=="OK":
            messagebox.showinfo("成功", "考生已添加")
            self.add_sid.delete(0, END); self.add_name.delete(0, END)
            self.common_refresh_list()
        else: messagebox.showerror("失败", "添加失败")

    def admin_delete_student(self):
        item = self.tree.selection()
        if item:
            vals = self.tree.item(item, "values")
            if messagebox.askyesno("删除", f"确认删除 {vals[2]} 吗？"):
                self.send_packet(f"ADMIN_DEL_STU|{vals[1]}")
                if self.recv_packet()=="OK": self.common_refresh_list()

    #通用刷新逻辑 
    def common_refresh_list(self):
        try:
            self.send_packet("ADMIN_GET_STU")
            data = self.recv_packet()
            
            if not data or not data.startswith("STU_LIST|"): return

            content = data[9:]
            if '|' in content:
                config, students_str = content.split('|', 1)
                q_total, q_exam = config.split(',')
                
                info_text = f"📊 题库总量: {q_total} | 本次考试题数: {q_exam}"
                
                # 检查组件存活
                if self.lbl_stats and self.lbl_stats.winfo_exists():
                    self.lbl_stats.config(text=info_text)
                if self.lbl_stats_admin and self.lbl_stats_admin.winfo_exists():
                    self.lbl_stats_admin.config(text=info_text)
                if not self.tree or not self.tree.winfo_exists():
                    return

                for i in self.tree.get_children(): self.tree.delete(i)
                
                lst = []
                for row in students_str.split(';'):
                    if not row.strip(): continue
                    parts = row.split(',')
                    if len(parts) < 4: continue
                    try:
                        lst.append({
                            "id": parts[0].strip(), "name": parts[1].strip(), 
                            "st": int(parts[2]), "sc": int(parts[3])
                        })
                    except: continue
                
                lst.sort(key=lambda x: (-x['sc'], -x['st'], x['id']))
                
                for i, x in enumerate(lst):
                    status = "✅ 已交卷" if x['st'] else "⏳ 未开始"
                    self.tree.insert("", END, values=(i+1, x['id'], x['name'], status, x['sc']))
                        
        except Exception as e:
            print(f"Refresh Error: {e}")

    #学生模块
    def show_student_menu(self):
        self.clear_ui()
        nav = ttk.Frame(self, padding=20, bootstyle="success")
        nav.pack(fill=X)
        ttk.Label(nav, text="学生考试终端", font=self.header_font, foreground="white", background="#28a745").pack(side=LEFT)
        
        f = ttk.Frame(self); f.pack(expand=True)
        btn_cfg = {"bootstyle": "success-outline", "width": 30, "padding": 10}
        
        ttk.Label(f, text="请选择操作:", font=self.sub_header_font).pack(pady=30)
        ttk.Button(f, text="💻 参加网络考试", command=self.student_login_ui, **btn_cfg).pack(pady=10)
        ttk.Button(f, text="📝 本地模拟练习", command=self.start_local_practice, **btn_cfg).pack(pady=10)
        ttk.Button(f, text="📊 查询成绩排名", command=self.student_query_score, **btn_cfg).pack(pady=10)
        ttk.Button(f, text="返回首页", command=self.show_main_role_select, bootstyle="secondary", width=30).pack(pady=30)

    def student_login_ui(self):
        self.clear_ui()
        f = ttk.Frame(self, padding=50); f.pack(expand=True)
        ttk.Label(f, text="考生身份验证", font=("Microsoft YaHei UI", 20, "bold"), bootstyle="success").pack(pady=20)
        ttk.Label(f, text="请输入学号:", font=("Arial", 12)).pack(anchor=W)
        self.stu_id_entry = ttk.Entry(f, width=25, font=("Arial", 14)); self.stu_id_entry.pack(pady=10)
        ttk.Button(f, text="连接考试服务器", command=self.student_do_connect, bootstyle="success", width=25).pack(pady=20)
        ttk.Button(f, text="取消", command=self.show_student_menu, bootstyle="link").pack()

    def student_do_connect(self):
        sid = self.stu_id_entry.get()
        if not sid: return
        if not self.create_connection(): return
        self.send_packet(f"LOGIN|{sid}")
        threading.Thread(target=self.student_listen_loop, daemon=True).start()

    def student_listen_loop(self):
        while True:
            data = self.recv_packet()
            if not data: break
            self.after(0, lambda: self.handle_student_data(data))

    def handle_student_data(self, data):
        if data.startswith("LOGIN_FAIL"):
            messagebox.showerror("登录失败", data.split("|")[1])
            self.disconnect_and_home()
        elif data.startswith("WAIT|"):
            self.show_waiting_screen(data.split("|")[1])
        elif data.startswith("QUE|"):
            self.update_question_ui(data.split("|"))
        elif data.startswith("MSG|"):
            messagebox.showinfo("提示", data[4:])
        elif data.startswith("REPORT|"):
            self.show_report_ui(data[7:])

    def show_waiting_screen(self, msg):
        self.clear_ui()
        f = ttk.Frame(self); f.pack(expand=True)
        ttk.Label(f, text="⏳", font=("Segoe UI Emoji", 64)).pack(pady=10)
        ttk.Label(f, text=msg, font=("Microsoft YaHei UI", 20), bootstyle="info").pack(pady=20)
        ttk.Progressbar(f, mode='indeterminate', length=300, bootstyle="info-striped").pack()
        ttk.Button(f, text="退出等待", command=self.disconnect_and_home, bootstyle="secondary-outline").pack(pady=30)

    def update_question_ui(self, parts):
        if not hasattr(self, 'q_label'): self.setup_exam_ui()
        self.q_label.config(text=parts[1])
        for w in self.opt_frame.winfo_children(): w.destroy()
        self.current_selection = set()
        for i, txt in enumerate(parts[2:6]):
            char = ['A','B','C','D'][i]
            btn = ttk.Button(self.opt_frame, text=f"{char}. {txt}", width=60, bootstyle="light")
            btn.configure(command=lambda b=btn, c=char: self.toggle_option(b, c))
            btn.pack(pady=8, ipady=8)

    def setup_exam_ui(self):
        self.clear_ui()
        ttk.Label(self, text="正在考试中...", font=("Arial", 12), bootstyle="danger").pack(pady=10)
        self.q_label = ttk.Label(self, text="Loading...", font=("Microsoft YaHei UI", 16), wraplength=900, justify=CENTER)
        self.q_label.pack(pady=30, padx=50)
        self.opt_frame = ttk.Frame(self); self.opt_frame.pack(pady=10)
        ttk.Button(self, text="提交本题", command=self.submit_answer, bootstyle="warning", width=20).pack(pady=30)

    def toggle_option(self, btn, char):
        if char in self.current_selection:
            self.current_selection.remove(char); btn.configure(bootstyle="light")
        else:
            self.current_selection.add(char); btn.configure(bootstyle="success")

    def submit_answer(self):
        if not self.current_selection: messagebox.showwarning("提示", "请选择选项"); return
        self.send_packet("".join(sorted(list(self.current_selection))))

    def show_report_ui(self, text):
        self.clear_ui()
        ttk.Label(self, text="📝 AI 智能评估报告", font=self.header_font, bootstyle="primary").pack(pady=20)
        st = scrolledtext.ScrolledText(self, font=("Microsoft YaHei", 11), height=20)
        st.pack(fill=BOTH, expand=True, padx=50, pady=10)
        st.insert(END, text); st.config(state=DISABLED)
        ttk.Button(self, text="退出系统", command=self.disconnect_and_home, bootstyle="danger", width=20).pack(pady=20)

    #辅助功能
    def student_query_score(self):
        if not self.create_connection(): return
        q = simpledialog.askstring("查分", "输入姓名或学号:")
        if q:
            self.send_packet(f"QUERY_SCORE|{q}")
            res = self.recv_packet()
            self.sock.close(); self.sock=None
            if res and res.startswith("SCORE_RESULT|"):
                _, n, s, r = res.split('|')
                messagebox.showinfo("成绩单", f"考生: {n}\n分数: {s}\n排名: {r}")
            elif res: messagebox.showinfo("提示", res.split('|')[1])

    def start_local_practice(self):
        try:
            with open("questions.txt","r",encoding="utf-8") as f: 
                lines=[l.strip().split('|') for l in f if len(l.strip().split('|'))>=6]
            import random; random.shuffle(lines); self.local_qs=lines[:5]
            self.local_idx=0; self.local_score=0; self.show_local_view()
        except: messagebox.showerror("错误", "题库读取失败")

    def show_local_view(self):
        self.clear_ui()
        q = self.local_qs[self.local_idx]
        ttk.Label(self, text=f"本地练习 {self.local_idx+1}/5", bootstyle="info").pack(pady=20)
        ttk.Label(self, text=q[0], font=("Microsoft YaHei UI", 16), wraplength=800).pack(pady=20)
        self.local_sel=set()
        for i,t in enumerate(q[1:5]):
            c=['A','B','C','D'][i]
            b=ttk.Button(self, text=f"{c}. {t}", width=50, bootstyle="light")
            b.configure(command=lambda btn=b, char=c: self.local_toggle(btn,char))
            b.pack(pady=5)
        ttk.Button(self, text="确定", command=lambda: self.check_local(q[5])).pack(pady=30)
        ttk.Button(self, text="退出", command=self.show_student_menu, bootstyle="link").pack()

    def local_toggle(self, btn, char):
        if char in self.local_sel: self.local_sel.remove(char); btn.configure(bootstyle="light")
        else: self.local_sel.add(char); btn.configure(bootstyle="success")

    def check_local(self, ans):
        u="".join(sorted(list(self.local_sel))); r="".join(sorted(list(ans.strip().upper())))
        if u==r: self.local_score+=10; messagebox.showinfo("正确","回答正确!")
        else: messagebox.showerror("错误",f"正确答案: {r}")
        self.local_idx+=1
        if self.local_idx<len(self.local_qs): self.show_local_view()
        else: messagebox.showinfo("结束",f"得分: {self.local_score}"); self.show_student_menu()

if __name__ == "__main__":
    app = ExamApp()
    app.mainloop()