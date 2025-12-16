# app.py
import ttkbootstrap as ttk
from ttkbootstrap.constants import *
from tkinter import messagebox, scrolledtext, simpledialog
import socket
import threading
import time

SERVER_IP = '127.0.0.1'
SERVER_PORT = 8888
DELIMITER = "$$$"  # 协议结束符

class ExamApp(ttk.Window):
    def __init__(self):
        super().__init__(themename="superhero")
        style = ttk.Style()
        my_font = ('Microsoft YaHei', 10)
        title_font = ('Microsoft YaHei', 28, "bold")
        style.configure('.', font=my_font)
        style.configure('TLabel', font=my_font)
        style.configure('TButton', font=my_font)
        style.configure('TEntry', font=my_font)
        style.configure('Treeview', font=my_font)
        style.configure('Treeview.Heading', font=my_font)
        
        self.title_font = title_font 
        self.title("智能考试系统")
        self.geometry("900x600")
        self.place_window_center()
        self.sock = None
        self.buffer = b""
        self.show_main_menu()

    def place_window_center(self):
        self.update_idletasks()
        w, h = self.winfo_width(), self.winfo_height()
        x = int((self.winfo_screenwidth()/2) - (w/2))
        y = int((self.winfo_screenheight()/2) - (h/2))
        self.geometry(f"{w}x{h}+{x}+{y}")

    def clear_ui(self):
        for widget in self.winfo_children(): widget.destroy()

    # ==================== 网络核心：缓冲发送与接收 ====================
    def send_packet(self, text):
        """统一发送函数，自动追加结束符"""
        if self.sock:
            try:
                # 确保发送的是 UTF-8 编码并加上 $$$
                self.sock.sendall((text + DELIMITER).encode('utf-8'))
            except Exception as e:
                print(f"Send Error: {e}")

    def recv_packet(self):
        """缓冲接收器，处理粘包"""
        while True:
            try:
                # 优先处理缓冲区已有的完整包
                if b"$$$" in self.buffer:
                    parts = self.buffer.split(b"$$$", 1)
                    msg = parts[0]
                    self.buffer = parts[1]
                    return msg.decode('utf-8', errors='ignore')
                
                if not self.sock: return None
                
                chunk = self.sock.recv(4096)
                if not chunk: 
                    # 【修复步骤 2】: 连接断开时，如果缓冲区还有数据（可能没有$$$了，但也得看看），或者直接返回None
                    # 对于本协议，没有$$$就是不完整，直接丢弃即可
                    return None 
                self.buffer += chunk
                
            except Exception as e:
                print(f"Recv Error: {e}")
                return None

    # ==================== 主菜单 ====================
    def show_main_menu(self):
        self.clear_ui()
        frame = ttk.Frame(self, padding=40)
        frame.pack(expand=True)
        ttk.Label(frame, text="C语言智能考试系统", font=('Microsoft YaHei', 28, "bold"), bootstyle="primary").pack(pady=20)
        btn_frame = ttk.Frame(frame); btn_frame.pack(pady=40)
        
        ttk.Button(btn_frame, text="参加网络考试", command=self.show_student_login, bootstyle="success", width=20).pack(side=LEFT, padx=10, ipady=15)
        ttk.Button(btn_frame, text="本地模拟练习", command=self.start_local_practice, bootstyle="info", width=20).pack(side=LEFT, padx=10, ipady=15)
        ttk.Button(btn_frame, text="管理员后台", command=self.admin_login_dialog, bootstyle="danger", width=20).pack(side=LEFT, padx=10, ipady=15)
        ttk.Label(frame, text="提示: 网络考试需先启动 C 后端 Server", bootstyle="secondary").pack(pady=20)

    # ==================== 管理员 ====================
    def admin_login_dialog(self):
        pwd = simpledialog.askstring("管理员验证", "请输入管理员密码:", show="*")
        if pwd == "123456": self.connect_for_admin()
        elif pwd: messagebox.showerror("错误", "密码错误")

    def connect_for_admin(self):
        if not self.create_connection(): return
        self.show_admin_panel()

    def show_admin_panel(self):
        self.clear_ui()
        top_bar = ttk.Frame(self, padding=10, bootstyle="secondary"); top_bar.pack(fill=X)
        ttk.Button(top_bar, text="返回", command=self.disconnect_and_return, bootstyle="light-outline").pack(side=LEFT)
        ttk.Label(top_bar, text="管理员控制台", font=("Microsoft YaHei", 12, "bold"), foreground="white").pack(side=LEFT, padx=20)
        
        ttk.Button(top_bar, text="录入新题", command=self.admin_add_question_dialog, bootstyle="warning").pack(side=RIGHT, padx=5)
        ttk.Button(top_bar, text="刷新", command=self.admin_refresh_data, bootstyle="info").pack(side=RIGHT, padx=5)

        content = ttk.Frame(self, padding=20); content.pack(fill=BOTH, expand=True)
        left_panel = ttk.Frame(content); left_panel.pack(side=LEFT, fill=Y, padx=(0, 20))

        setting_frame = ttk.Labelframe(left_panel, text="考试参数设置", padding=15, bootstyle="info")
        setting_frame.pack(fill=X, pady=(0, 20))
        self.lbl_q_count = ttk.Label(setting_frame, text="题库总数: 加载中...")
        self.lbl_q_count.pack(anchor=W)
        ttk.Label(setting_frame, text="单次考试题数:").pack(anchor=W, pady=(10,0))
        self.ent_exam_num = ttk.Entry(setting_frame, width=10); self.ent_exam_num.pack(fill=X, pady=5)
        ttk.Button(setting_frame, text="保存设置", command=self.admin_set_exam_num, bootstyle="success-outline").pack(fill=X)

        add_stu_frame = ttk.Labelframe(left_panel, text="添加考生", padding=15, bootstyle="success")
        add_stu_frame.pack(fill=X)
        ttk.Label(add_stu_frame, text="学号:").pack(anchor=W); self.add_id_entry = ttk.Entry(add_stu_frame); self.add_id_entry.pack(fill=X, pady=5)
        ttk.Label(add_stu_frame, text="姓名:").pack(anchor=W); self.add_name_entry = ttk.Entry(add_stu_frame); self.add_name_entry.pack(fill=X, pady=5)
        ttk.Button(add_stu_frame, text="添加", command=self.admin_add_student, bootstyle="success").pack(fill=X, pady=10)

        right_panel = ttk.Labelframe(content, text="考生列表 (右键可删除)", padding=15, bootstyle="secondary")
        right_panel.pack(side=LEFT, fill=BOTH, expand=True)

        self.tree = ttk.Treeview(right_panel, columns=("学号", "姓名", "状态", "成绩"), show="headings")
        for c in ("学号", "姓名", "状态", "成绩"): self.tree.heading(c, text=c); self.tree.column(c, width=100)
        self.tree.pack(fill=BOTH, expand=True)
        self.tree.bind("<Double-1>", self.admin_reset_student)
        self.tree.bind("<Button-3>", self.show_context_menu)
        
        self.context_menu = ttk.Menu(self, tearoff=0)
        self.context_menu.add_command(label="删除该考生", command=self.admin_delete_student)
        self.admin_refresh_data()
            
    def show_context_menu(self, event):
        item = self.tree.identify_row(event.y)
        if item:
            self.tree.selection_set(item)
            self.context_menu.post(event.x_root, event.y_root)
            
    def admin_refresh_data(self):
        try:
            self.send_packet("ADMIN_GET_STU")
            data = self.recv_packet()
            if data and data.startswith("STU_LIST|"):
                content = data[9:] 
                if '|' in content:
                    config_str, stu_data_str = content.split('|', 1)
                    total_q, exam_n = config_str.split(',')
                    self.lbl_q_count.config(text=f"题库总数: {total_q}")
                    
                    for item in self.tree.get_children(): self.tree.delete(item)
                    parsed_students = []
                    for item in stu_data_str.split(';'):
                        if not item: continue
                        parts = item.split(',')
                        if len(parts) < 4: continue
                        parsed_students.append({
                            "id": parts[0], "name": parts[1], 
                            "status": "已考" if int(parts[2]) else "未考",
                            "score": int(parts[3]), "is_taken": int(parts[2])
                        })
                    
                    parsed_students.sort(key=lambda x: (-x['score'], -x['is_taken'], x['id']))
                    for s in parsed_students:
                        self.tree.insert("", END, values=(s['id'], s['name'], s['status'], s['score']))
        except Exception as e: print(e) 

    def admin_add_student(self):
        sid, name = self.add_id_entry.get(), self.add_name_entry.get()
        if not sid or not name: return
        self.send_packet(f"ADMIN_ADD_STU|{sid}|{name}")
        if self.recv_packet() == "OK": messagebox.showinfo("OK", "添加成功"); self.admin_refresh_data()
        else: messagebox.showerror("Fail", "失败")

    def admin_reset_student(self, event):
        item = self.tree.selection()
        if not item: return
        vals = self.tree.item(item, "values")
        if messagebox.askyesno("Confirm", f"重置 {vals[1]}?"):
            self.send_packet(f"ADMIN_RESET_STU|{vals[0]}")
            if self.recv_packet() == "OK": self.admin_refresh_data()

    # ==================== 考生 ====================
    def show_student_login(self):
        self.clear_ui()
        frame = ttk.Frame(self, padding=40); frame.pack(expand=True)
        ttk.Label(frame, text="考生登录", font=("Bold", 20)).pack(pady=20)
        self.stu_id_entry = ttk.Entry(frame, width=20); self.stu_id_entry.pack(pady=10)
        ttk.Button(frame, text="进入", command=self.student_connect, bootstyle="success").pack(pady=20)
        ttk.Button(frame, text="返回", command=self.show_main_menu, bootstyle="secondary-outline").pack()

    def student_connect(self):
        sid = self.stu_id_entry.get()
        if not self.create_connection(): return
        self.send_packet(f"LOGIN|{sid}")
        threading.Thread(target=self.student_listen_loop, daemon=True).start()

    def student_listen_loop(self):
        while True:
            data = self.recv_packet()
            if not data: 
                print("连接已断开")
                break
            self.after(0, lambda: self.handle_student_packet(data))

    def handle_student_packet(self, data):
        if not data: return
        print(f"DEBUG RECEIVE: {data[:50]}...") 
        
        if data.startswith("LOGIN_OK"): 
            self.show_exam_view()
        elif data.startswith("LOGIN_FAIL"): 
            messagebox.showerror("Fail", data.split("|")[1])
            self.disconnect_and_return()
        elif data.startswith("QUE|"): 
            parts = data.split("|")
            if len(parts) >= 2: self.update_question_ui(parts)
        elif data.startswith("MSG|"): 
            messagebox.showinfo("Info", data[4:])
        elif data.startswith("REPORT|"): 
            self.show_report_ui(data[7:])

    def show_exam_view(self):
        self.clear_ui()
        # 顶部题目显示
        self.q_label = ttk.Label(self, text="Loading...", font=("Microsoft YaHei", 16), wraplength=800)
        self.q_label.pack(pady=30, padx=20)
        
        # 选项区域
        self.opt_frame = ttk.Frame(self)
        self.opt_frame.pack(pady=20)
        
        # 底部提交区域
        self.action_frame = ttk.Frame(self)
        self.action_frame.pack(pady=20)
        self.btn_submit = ttk.Button(self.action_frame, text="确认提交本题", command=self.submit_current_answer, bootstyle="warning", width=20)
        self.btn_submit.pack()

        # 【重要】初始化多选集合，防止 AttributeError
        self.current_selection = set()
        
    def safe_send_answer(self, choice):
        # 【修复】使用统一的 send_packet (带 $$$)
        self.send_packet(choice)
    
    def submit_current_answer(self):
        """提交答案"""
        if not self.current_selection:
            messagebox.showwarning("提示", "请至少选择一个选项！")
            return
        
        # 1. 将集合转为列表并排序 (确保 "BA" 变成 "AB")
        sorted_ans = sorted(list(self.current_selection))
        final_answer_str = "".join(sorted_ans) # e.g. "ABD"
        
        # 2. 发送给 C 服务器 (会自动加 $$$)
        self.send_packet(final_answer_str)
        
        
    def update_question_ui(self, parts):
        """
        parts: [QUE, 题干, A, B, C, D]
        """
        # ==================== 【修复开始】 ====================
        # 安全检查：防止 LOGIN_OK 处理慢于 QUE 包，导致 q_label 未创建就调用
        if not hasattr(self, 'q_label') or self.q_label is None:
            # 如果组件不存在，说明界面还没切换，强制切换一次
            self.show_exam_view()
        # ==================== 【修复结束】 ====================

        # 1. 更新题干
        try:
            self.q_label.config(text=parts[1])
        except Exception as e:
            print(f"UI Update Error: {e}")
            return

        # 2. 清空旧选项
        for w in self.opt_frame.winfo_children(): w.destroy()
        self.current_selection.clear() # 重置选中状态 (确保这一行存在)
        
        # 3. 动态生成 A, B, C, D 四个按钮
        options = parts[2:6] 
        for i, opt_text in enumerate(options):
            char = ['A', 'B', 'C', 'D'][i]
            
            btn = ttk.Button(
                self.opt_frame, 
                text=f"{char}. {opt_text}", 
                width=50, 
                bootstyle="secondary-outline"
            )
            btn.configure(command=lambda b=btn, c=char: self.toggle_option(b, c))
            btn.pack(pady=5, ipady=5)
    
    def toggle_option(self, btn, char):
        """切换选项的选中状态 (高亮/取消高亮)"""
        if char in self.current_selection:
            # 已经选中 -> 取消选中
            self.current_selection.remove(char)
            btn.configure(bootstyle="secondary-outline") # 变回灰色空心
        else:
            # 未选中 -> 选中
            self.current_selection.add(char)
            btn.configure(bootstyle="success") # 变为绿色实心
     
    def show_report_ui(self, report):
        self.clear_ui()
        ttk.Label(self, text="AI 智能评估报告", font=("Microsoft YaHei", 18, "bold"), bootstyle="info").pack(pady=10)
        st = scrolledtext.ScrolledText(self, height=20, font=("Microsoft YaHei", 12))
        st.pack(fill=BOTH, expand=True, padx=20, pady=5)
        st.insert(END, report)
        st.config(state=DISABLED) 
        ttk.Button(self, text="退出", command=self.disconnect_and_return, bootstyle="danger").pack(pady=20) 

    def create_connection(self):
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.settimeout(2); self.sock.connect((SERVER_IP, SERVER_PORT)); self.sock.settimeout(None)
            self.buffer = b"" # 清空 buffer
            return True
        except: messagebox.showerror("Err", "连接失败"); return False

    def disconnect_and_return(self):
        if self.sock: 
            try: self.sock.close()
            except: pass
            self.sock = None
        self.show_main_menu()
    
    def start_local_practice(self):
        try:
            with open("questions.txt", "r", encoding="utf-8") as f:
                lines = f.readlines()
            
            import random
            questions = []
            for line in lines:
                parts = line.strip().split("|")
                if len(parts) >= 6: questions.append(parts)
            
            if not questions:
                messagebox.showerror("错误", "题库为空！")
                return

            random.shuffle(questions)
            self.local_questions = questions[:5] 
            self.local_score = 0
            self.local_idx = 0
            self.show_local_exam_view()
            
        except FileNotFoundError:
            messagebox.showerror("错误", "找不到 questions.txt 文件")
        except Exception as e:
            messagebox.showerror("错误", str(e))

    def show_local_exam_view(self):
        self.clear_ui()
        
        # 1. 进度显示
        ttk.Label(self, text=f"本地模拟模式 - 第 {self.local_idx + 1} / {len(self.local_questions)} 题", 
                  bootstyle="info").pack(pady=10)
        
        # 获取当前题目数据 [内容, A, B, C, D, 答案]
        q_data = self.local_questions[self.local_idx]
        
        # 2. 题目区域
        q_frame = ttk.Labelframe(self, text="题目", padding=15)
        q_frame.pack(fill=BOTH, expand=YES, padx=20)
        ttk.Label(q_frame, text=q_data[0], font=("WenQuanYi Micro Hei", 14), wraplength=700).pack(anchor=W)
        
        # 3. 选项区域
        self.opt_frame = ttk.Frame(self)
        self.opt_frame.pack(pady=20)
        
        # 【关键】初始化本地多选集合
        self.local_selection = set()
        
        correct_ans = q_data[5] # 正确答案，如 "AB"
        
        # 生成选项按钮 (改为切换模式)
        for i, text in enumerate(q_data[1:5]):
            char = ['A', 'B', 'C', 'D'][i]
            btn = ttk.Button(self.opt_frame, text=f"{char}. {text}", width=50, bootstyle="secondary-outline")
            # 绑定切换逻辑
            btn.configure(command=lambda b=btn, c=char: self.local_toggle(b, c))
            btn.pack(pady=5)
            
        # 4. 提交按钮 (点击才算答题)
        ttk.Button(self, text="确认提交", width=20, bootstyle="warning",
                   command=lambda: self.check_local_answer(correct_ans)).pack(pady=20)


             
    def local_toggle(self, btn, char):
        """本地模式：切换选项选中状态"""
        if char in self.local_selection:
            self.local_selection.remove(char)
            btn.configure(bootstyle="secondary-outline") # 取消高亮
        else:
            self.local_selection.add(char)
            btn.configure(bootstyle="success") # 高亮选中
             
    def check_local_answer(self, correct_choice):
        """本地模式：核对答案"""
        # 1. 检查是否有选中
        if not hasattr(self, 'local_selection') or not self.local_selection:
            messagebox.showwarning("提示", "请至少选择一个选项！")
            return
            
        # 2. 获取用户选择并排序 (例如集合{'B','A'} -> "AB")
        user_choice = "".join(sorted(list(self.local_selection)))
        
        # 3. 处理正确答案 (去除首尾空格并转大写，排序防止顺序不同)
        clean_correct = "".join(sorted(list(correct_choice.strip().upper())))
        
        # 4. 比对
        if user_choice == clean_correct:
            self.local_score += 10
            messagebox.showinfo("正确", "回答正确！ +10分")
        else:
            messagebox.showerror("错误", f"回答错误！\n你的选择: {user_choice}\n正确答案: {clean_correct}")
            
        # 5. 进入下一题
        self.local_idx += 1
        if self.local_idx < len(self.local_questions):
            self.show_local_exam_view()
        else:
            messagebox.showinfo("结束", f"模拟练习结束！\n你的得分: {self.local_score}")
            self.show_main_menu()

    def admin_set_exam_num(self):
        num = self.ent_exam_num.get()
        if not num.isdigit(): return
        self.send_packet(f"ADMIN_SET_COUNT|{num}")
        res = self.recv_packet()
        if res == "OK": messagebox.showinfo("成功", "设置已更新"); self.admin_refresh_data()
        else: messagebox.showerror("失败", res.split('|')[1])

    def admin_delete_student(self):
        item = self.tree.selection()
        if not item: return
        vals = self.tree.item(item, "values")
        if messagebox.askyesno("危险操作", f"确定要永久删除考生 {vals[1]} ({vals[0]}) 吗？"):
            self.send_packet(f"ADMIN_DEL_STU|{vals[0]}")
            if self.recv_packet() == "OK": self.admin_refresh_data()
            else: messagebox.showerror("错误", "删除失败")

    def admin_add_question_dialog(self):
        win = ttk.Toplevel(self); win.title("录入新题"); win.geometry("500x600")
        ttk.Label(win, text="题干内容:").pack(anchor=W, padx=20, pady=5)
        t_content = scrolledtext.ScrolledText(win, height=4); t_content.pack(fill=X, padx=20)
        vars = []
        for opt in ['A', 'B', 'C', 'D']:
            ttk.Label(win, text=f"选项 {opt}:").pack(anchor=W, padx=20, pady=2)
            e = ttk.Entry(win); e.pack(fill=X, padx=20)
            vars.append(e)
        ttk.Label(win, text="正确答案 (如 A):").pack(anchor=W, padx=20, pady=5)
        t_ans = ttk.Entry(win); t_ans.pack(fill=X, padx=20)
        
        def submit():
            content = t_content.get("1.0", END).strip().replace('\n', ' ')
            opts = [v.get().strip() for v in vars]
            ans = t_ans.get().strip().upper()
            if not content or not all(opts) or not ans: messagebox.showwarning("提示", "所有字段都必须填写"); return
            safe_content = content.replace('|', ' ')
            msg = f"ADMIN_ADD_QUE|{safe_content}|{opts[0]}|{opts[1]}|{opts[2]}|{opts[3]}|{ans}"
            self.send_packet(msg)
            res = self.recv_packet()
            if res == "OK": messagebox.showinfo("成功", "题目录入成功！"); win.destroy(); self.admin_refresh_data()
            else: messagebox.showerror("失败", res)

        ttk.Button(win, text="提交保存", command=submit, bootstyle="warning").pack(pady=20)

if __name__ == "__main__": ExamApp().mainloop()