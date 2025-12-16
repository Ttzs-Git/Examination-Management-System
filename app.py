import ttkbootstrap as ttk
from ttkbootstrap.constants import *
from tkinter import messagebox, scrolledtext, simpledialog
import socket
import threading
import time

SERVER_IP = '127.0.0.1'
SERVER_PORT = 8888
DELIMITER = "$$$"  # 与 C 语言约定的结束符

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

    # ==================== 网络核心：缓冲接收器 ====================
    def recv_packet(self):
        """
        修复版：使用 self.buffer 持久化存储，防止丢包
        """
        while True:
            try:
                # 1. 先检查缓存区是否已经包含完整的包
                if b"$$$" in self.buffer:
                    # 切割数据：取第一个 $$$ 之前的部分作为本次消息
                    parts = self.buffer.split(b"$$$", 1) # 只切一刀
                    msg = parts[0]
                    self.buffer = parts[1] # 【关键】把剩下的数据塞回缓冲区，留给下次用
                    return msg.decode('utf-8', errors='ignore')
                
                # 2. 缓存区没有完整包，从网络读取
                chunk = self.sock.recv(4096)
                if not chunk: 
                    # 连接断开
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

        btn_frame = ttk.Frame(frame)
        btn_frame.pack(pady=40)

        # 按钮 1: 网络考试 (原“我是考生”)
        b1 = ttk.Button(btn_frame, text="参加网络考试", command=self.show_student_login, bootstyle="success", width=20)
        b1.pack(side=LEFT, padx=10, ipady=15)
        
        # 按钮 2: 【新增】本地练习
        b2 = ttk.Button(btn_frame, text="本地模拟练习", command=self.start_local_practice, bootstyle="info", width=20)
        b2.pack(side=LEFT, padx=10, ipady=15)
        
        # 按钮 3: 管理员
        b3 = ttk.Button(btn_frame, text="管理员后台", command=self.admin_login_dialog, bootstyle="danger", width=20)
        b3.pack(side=LEFT, padx=10, ipady=15)
        
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
        top_bar = ttk.Frame(self, padding=10, bootstyle="secondary")
        top_bar.pack(fill=X)
        ttk.Button(top_bar, text=" 返回", command=self.disconnect_and_return, bootstyle="light-outline").pack(side=LEFT)
        ttk.Button(top_bar, text="刷新数据", command=self.admin_refresh_data, bootstyle="info").pack(side=RIGHT)

        content = ttk.Frame(self, padding=20)
        content.pack(fill=BOTH, expand=True)

        left = ttk.Labelframe(content, text="添加考生", padding=15)
        left.pack(side=LEFT, fill=Y, padx=(0, 20))
        ttk.Label(left, text="学号:").pack(anchor=W); self.add_id_entry = ttk.Entry(left); self.add_id_entry.pack(fill=X, pady=5)
        ttk.Label(left, text="姓名:").pack(anchor=W); self.add_name_entry = ttk.Entry(left); self.add_name_entry.pack(fill=X, pady=5)
        ttk.Button(left, text=" 添加", command=self.admin_add_student, bootstyle="success").pack(fill=X, pady=20)

        right = ttk.Labelframe(content, text="考生监控", padding=15)
        right.pack(side=LEFT, fill=BOTH, expand=True)
        self.tree = ttk.Treeview(right, columns=("学号", "姓名", "状态", "成绩"), show="headings")
        for c in ("学号", "姓名", "状态", "成绩"): self.tree.heading(c, text=c); self.tree.column(c, width=100)
        self.tree.pack(fill=BOTH, expand=True)
        self.tree.bind("<Double-1>", self.admin_reset_student)
        
        self.admin_refresh_data()

    def admin_refresh_data(self):
        try:
            # 1. 发送请求
            self.sock.send(b"ADMIN_GET_STU")
            
            # 2. 接收数据 (使用之前修复过的 recv_packet)
            data = self.recv_packet()
            
            if data and data.startswith("STU_LIST|"):
                # 清空现有表格
                for item in self.tree.get_children(): 
                    self.tree.delete(item)
                
                # === 新增逻辑：先解析，再排序 ===
                parsed_students = []
                
                # C语言发来的格式: ID,Name,Taken,Score;ID,Name...
                raw_items = data[9:].split(';')
                
                for item in raw_items:
                    if not item: continue
                    parts = item.split(',')
                    if len(parts) < 4: continue # 防止脏数据
                    
                    sid, name, taken_str, score_str = parts
                    
                    # 将字符串转换为数字，以便正确排序
                    score = int(score_str)
                    is_taken = int(taken_str)
                    status_text = "已考" if is_taken else "未考"
                    
                    # 存入临时列表字典
                    parsed_students.append({
                        "id": sid,
                        "name": name,
                        "status": status_text,
                        "score": score,
                        "is_taken": is_taken
                    })
                
                # === 核心排序逻辑 ===
                # key参数定义排序规则：
                # 规则1: -x['score'] (分数由高到低，因为默认是升序，加负号变降序)
                # 规则2: -x['is_taken'] (同分情况下，已考的排前面)
                # 规则3: x['id'] (分数状态都一样，按学号从小到大排)
                parsed_students.sort(key=lambda x: (-x['score'], -x['is_taken'], x['id']))

                # 3. 将排序好的数据插入表格
                for stu in parsed_students:
                    self.tree.insert("", END, values=(stu['id'], stu['name'], stu['status'], stu['score']))
                    
        except Exception as e:
            messagebox.showerror("刷新失败", str(e))
            print(f"Refresh Error: {e}") 

    def admin_add_student(self):
        sid, name = self.add_id_entry.get(), self.add_name_entry.get()
        if not sid or not name: return
        self.sock.send(f"ADMIN_ADD_STU|{sid}|{name}".encode())
        if self.recv_packet() == "OK":
            messagebox.showinfo("OK", "添加成功"); self.admin_refresh_data()
        else: messagebox.showerror("Fail", "失败")

    def admin_reset_student(self, event):
        item = self.tree.selection()
        if not item: return
        vals = self.tree.item(item, "values")
        if messagebox.askyesno("Confirm", f"重置 {vals[1]}?"):
            self.sock.send(f"ADMIN_RESET_STU|{vals[0]}".encode())
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
        self.sock.send(f"LOGIN|{sid}".encode())
        threading.Thread(target=self.student_listen_loop, daemon=True).start()

    def student_listen_loop(self):
        while True:
            # 使用缓冲接收函数
            data = self.recv_packet()
            if not data: 
                print("连接已断开")
                break
            self.after(0, lambda: self.handle_student_packet(data))

    def handle_student_packet(self, data):
        # 【修复3】最后的防线：如果 data 依然是 None，直接返回不处理
        if not data: 
            return

        print(f"DEBUG RECEIVE: {data[:100]}...") 
        
        if data.startswith("LOGIN_OK"): 
            self.show_exam_view()
        elif data.startswith("LOGIN_FAIL"): 
            messagebox.showerror("Fail", data.split("|")[1])
            self.disconnect_and_return()
        elif data.startswith("QUE|"): 
            parts = data.split("|")
            # 简单的防崩溃检查，防止 split 后长度不够
            if len(parts) >= 2:
                self.update_question_ui(parts)
        elif data.startswith("MSG|"): 
            messagebox.showinfo("Info", data[4:])
        elif data.startswith("REPORT|"): 
            self.show_report_ui(data[7:])

    def show_exam_view(self):
        self.clear_ui()
        self.q_label = ttk.Label(self, text="Loading...", font=("Microsoft YaHei", 16), wraplength=800); self.q_label.pack(pady=30, padx=20)
        self.opt_frame = ttk.Frame(self); self.opt_frame.pack(pady=20)

    def safe_send_answer(self, choice):
        """
        发送答案时增加异常捕获。
        如果服务器已经因为考试结束而关闭了连接，
        这里会捕获错误，防止程序闪退。
        """
        try:
            if self.sock:
                self.sock.send(choice.encode())
        except Exception as e:
            print(f"停止发送 (考试已结束或连接断开): {e}")
    
    def update_question_ui(self, parts):
        self.q_label.config(text=parts[1])
        for w in self.opt_frame.winfo_children(): w.destroy()
        for i, opt in enumerate(parts[2:6]):
            l = ['A','B','C','D'][i]
            ttk.Button(self.opt_frame, text=f"{l}. {opt}", command=lambda x=l: self.safe_send_answer(x), width=40, bootstyle="info-outline").pack(pady=5)

    def show_report_ui(self, report):
        self.clear_ui()
        
        # 标题
        ttk.Label(self, text="AI 智能评估报告", font=("Microsoft YaHei", 18, "bold"), bootstyle="info").pack(pady=10)
        
        # === 【修复重点】给 ScrolledText 单独设置字体和字号 ===
        # font=("Microsoft YaHei", 12) : 使用微软雅黑，字号 12 (比默认的大且清晰)
        st = scrolledtext.ScrolledText(self, height=20, font=("Microsoft YaHei", 12))
        
        st.pack(fill=BOTH, expand=True, padx=20, pady=5)
        st.insert(END, report)
        
        # 设置为只读，防止用户误删报告内容
        st.config(state=DISABLED) 
        
        ttk.Button(self, text="退出", command=self.disconnect_and_return, bootstyle="danger").pack(pady=20) 

    def create_connection(self):
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.settimeout(2); self.sock.connect((SERVER_IP, SERVER_PORT)); self.sock.settimeout(None)
            return True
        except: messagebox.showerror("Err", "连接失败"); return False

    def disconnect_and_return(self):
        if self.sock: self.sock.close()
        self.show_main_menu()
    
    def start_local_practice(self):
        """不需要连接 C 服务器，直接读取本地 questions.txt"""
        try:
            with open("questions.txt", "r", encoding="utf-8") as f:
                lines = f.readlines()
            
            import random
            questions = []
            for line in lines:
                parts = line.strip().split("|")
                if len(parts) >= 6:
                    questions.append(parts)
            
            if not questions:
                messagebox.showerror("错误", "题库为空！")
                return

            # 随机抽 5 题
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
        
        # 进度显示
        ttk.Label(self, text=f"本地模拟模式 - 第 {self.local_idx + 1} / {len(self.local_questions)} 题", 
                  bootstyle="info").pack(pady=10)
        
        q_data = self.local_questions[self.local_idx]
        # q_data 结构: [Content, A, B, C, D, Answer]
        
        q_frame = ttk.Labelframe(self, text="题目", padding=15)
        q_frame.pack(fill=BOTH, expand=YES, padx=20)
        
        ttk.Label(q_frame, text=q_data[0], font=("WenQuanYi Micro Hei", 14), wraplength=700).pack(anchor=W)
        
        opt_frame = ttk.Frame(self)
        opt_frame.pack(pady=20)
        
        correct_ans = q_data[5]
        
        # 选项按钮
        for i, text in enumerate(q_data[1:5]):
            char = ['A', 'B', 'C', 'D'][i]
            ttk.Button(opt_frame, text=f"{char}. {text}", width=40, bootstyle="secondary-outline",
                       command=lambda c=char, ans=correct_ans: self.check_local_answer(c, ans)).pack(pady=5)

    def check_local_answer(self, user_choice, correct_choice):
        if user_choice == correct_choice:
            self.local_score += 10
            messagebox.showinfo("正确", "回答正确！ +10分")
        else:
            messagebox.showerror("错误", f"回答错误！正确答案是 {correct_choice}")
            
        self.local_idx += 1
        if self.local_idx < len(self.local_questions):
            self.show_local_exam_view()
        else:
            messagebox.showinfo("结束", f"模拟练习结束！\n你的得分: {self.local_score}")
            self.show_main_menu()

if __name__ == "__main__": ExamApp().mainloop()