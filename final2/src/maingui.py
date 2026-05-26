import tkinter as tk
from tkinter import ttk, messagebox, scrolledtext
import os
import time
from datetime import datetime

# ---------------------------- DATA LAYER (compatible with C files) ----------------------------
USER_FILE = "users.txt"
ITEM_FILE = "items.txt"
CLAIM_FILE = "claims.txt"
MSG_FILE = "messages.txt"
LOG_FILE = "logs.txt"

def read_users():
    """Return dict: username -> {'password': pwd, 'role': role, 'points': int}"""
    users = {}
    if not os.path.exists(USER_FILE):
        return users
    with open(USER_FILE, 'r') as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            parts = line.split(',')
            if len(parts) == 4:
                user, pwd, role, pts = parts
                users[user] = {'password': pwd, 'role': role, 'points': int(pts)}
    return users

def write_users(users):
    with open(USER_FILE, 'w') as f:
        for user, data in users.items():
            f.write(f"{user},{data['password']},{data['role']},{data['points']}\n")

def get_user_points(username):
    users = read_users()
    return users.get(username, {}).get('points', 0)

def update_user_points(username, delta):
    users = read_users()
    if username in users:
        users[username]['points'] += delta
        write_users(users)
        return True
    return False

def next_id(file_path):
    if not os.path.exists(file_path):
        return 1
    with open(file_path, 'r') as f:
        lines = f.readlines()
    max_id = 0
    for line in lines:
        line = line.strip()
        if not line:
            continue
        parts = line.split('|')
        if parts and parts[0].isdigit():
            max_id = max(max_id, int(parts[0]))
    return max_id + 1

def log_action(action):
    with open(LOG_FILE, 'a') as f:
        f.write(f"{int(time.time())} - {action}\n")

def save_item(owner, typ, name, category, desc, bounty=0):
    item_id = next_id(ITEM_FILE)
    with open(ITEM_FILE, 'a') as f:
        f.write(f"{item_id}|{owner}|{typ}|{name}|{category}|{desc}|{bounty}\n")
    log_action(f"Item created: {item_id} by {owner}")
    return item_id

def list_items():
    items = []
    if not os.path.exists(ITEM_FILE):
        return items
    with open(ITEM_FILE, 'r') as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            parts = line.split('|')
            if len(parts) == 7:
                items.append({
                    'id': int(parts[0]),
                    'owner': parts[1],
                    'type': parts[2],
                    'name': parts[3],
                    'category': parts[4],
                    'desc': parts[5],
                    'bounty': int(parts[6])
                })
    return items

def get_item_by_id(item_id):
    for item in list_items():
        if item['id'] == item_id:
            return item
    return None

def save_claim(item_id, claimant):
    claim_id = next_id(CLAIM_FILE)
    with open(CLAIM_FILE, 'a') as f:
        f.write(f"{claim_id}|{item_id}|{claimant}|PENDING|{int(time.time())}\n")
    log_action(f"Claim {claim_id} on item {item_id} by {claimant}")
    return claim_id

def list_claims():
    claims = []
    if not os.path.exists(CLAIM_FILE):
        return claims
    with open(CLAIM_FILE, 'r') as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            parts = line.split('|')
            if len(parts) == 5:
                claims.append({
                    'id': int(parts[0]),
                    'item_id': int(parts[1]),
                    'claimant': parts[2],
                    'status': parts[3],
                    'timestamp': int(parts[4])
                })
    return claims

def update_claim_status(claim_id, new_status):
    claims = list_claims()
    updated = False
    with open(CLAIM_FILE, 'w') as f:
        for claim in claims:
            if claim['id'] == claim_id:
                claim['status'] = new_status
                updated = True
            f.write(f"{claim['id']}|{claim['item_id']}|{claim['claimant']}|{claim['status']}|{claim['timestamp']}\n")
    if updated:
        log_action(f"Claim {claim_id} status -> {new_status}")
    return updated

def approve_claim_with_points(claim_id):
    """Transfer bounty points from owner to claimant, then approve claim."""
    claims = list_claims()
    claim = next((c for c in claims if c['id'] == claim_id), None)
    if not claim or claim['status'] != 'PENDING':
        return False
    item = get_item_by_id(claim['item_id'])
    if not item:
        return False
    owner = item['owner']
    claimant = claim['claimant']
    bounty = item['bounty']

    if owner == claimant:
        return False

    users = read_users()
    if owner not in users or claimant not in users:
        return False

    if bounty > 0:
        if users[owner]['points'] < bounty:
            # insufficient points -> auto reject
            update_claim_status(claim_id, 'REJECTED')
            send_message("system", claimant, f"Claim rejected: owner has insufficient points.", "CLAIM_UPDATE")
            return False
        users[owner]['points'] -= bounty
        users[claimant]['points'] += bounty
        write_users(users)
        send_message("system", claimant, f"Your claim on item '{item['name']}' was APPROVED. You gained {bounty} points.", "CLAIM_UPDATE")
        send_message("system", owner, f"User {claimant} claimed your item '{item['name']}'. {bounty} points deducted.", "CLAIM_UPDATE")
    else:
        send_message("system", claimant, f"Your claim on item '{item['name']}' was APPROVED.", "CLAIM_UPDATE")

    update_claim_status(claim_id, 'APPROVED')
    return True

def send_message(from_user, to_user, msg, msg_type="NORMAL"):
    msg_id = next_id(MSG_FILE)
    with open(MSG_FILE, 'a') as f:
        f.write(f"{msg_id}|{from_user}|{to_user}|{msg}|{int(time.time())}|{msg_type}\n")
    log_action(f"Message from {from_user} to {to_user}")

def get_messages_for_user(username):
    messages = []
    if not os.path.exists(MSG_FILE):
        return messages
    with open(MSG_FILE, 'r') as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            parts = line.split('|')
            if len(parts) == 6:
                mid, from_u, to_u, msg, ts, typ = parts
                if to_u == username:
                    messages.append({
                        'id': int(mid),
                        'from': from_u,
                        'message': msg,
                        'timestamp': int(ts),
                        'type': typ
                    })
    messages.sort(key=lambda x: x['timestamp'])
    return messages

def get_point_requests():
    """Return list of point requests (message objects)."""
    if not os.path.exists(MSG_FILE):
        return []
    requests = []
    with open(MSG_FILE, 'r') as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            parts = line.split('|')
            if len(parts) == 6:
                mid, from_u, to_u, msg, ts, typ = parts
                if to_u == 'admin' and typ == 'POINTS_REQUEST':
                    requests.append({
                        'id': int(mid),
                        'from': from_u,
                        'message': msg,
                        'timestamp': int(ts),
                        'type': typ
                    })
    return requests

def delete_message_by_id(msg_id):
    lines = []
    if not os.path.exists(MSG_FILE):
        return
    with open(MSG_FILE, 'r') as f:
        lines = f.readlines()
    with open(MSG_FILE, 'w') as f:
        for line in lines:
            parts = line.strip().split('|')
            if parts and int(parts[0]) != msg_id:
                f.write(line)

# ---------------------------- GUI APPLICATION ----------------------------
class LostFoundApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Lost & Found System")
        self.root.geometry("900x650")
        self.current_user = None
        self.init_files()
        self.show_login()

    def init_files(self):
        # Ensure files exist, create if missing
        if not os.path.exists(USER_FILE):
            with open(USER_FILE, 'w') as f:
                f.write("admin,admin123,ADMIN,500\n")
        if not os.path.exists(ITEM_FILE):
            open(ITEM_FILE, 'a').close()
        if not os.path.exists(CLAIM_FILE):
            open(CLAIM_FILE, 'a').close()
        if not os.path.exists(MSG_FILE):
            open(MSG_FILE, 'a').close()
        if not os.path.exists(LOG_FILE):
            open(LOG_FILE, 'a').close()

    def clear_frame(self):
        for widget in self.root.winfo_children():
            widget.destroy()

    def show_login(self):
        self.clear_frame()
        tk.Label(self.root, text="Lost & Found System", font=("Arial", 20)).pack(pady=20)

        frame = tk.Frame(self.root)
        frame.pack(pady=20)

        tk.Label(frame, text="Username:").grid(row=0, column=0, padx=5, pady=5)
        self.login_user = tk.Entry(frame)
        self.login_user.grid(row=0, column=1, padx=5, pady=5)

        tk.Label(frame, text="Password:").grid(row=1, column=0, padx=5, pady=5)
        self.login_pass = tk.Entry(frame, show="*")
        self.login_pass.grid(row=1, column=1, padx=5, pady=5)

        def do_login():
            uname = self.login_user.get()
            pwd = self.login_pass.get()
            users = read_users()
            if uname in users and users[uname]['password'] == pwd:
                self.current_user = uname
                if users[uname]['role'] == 'ADMIN':
                    self.show_admin_dashboard()
                else:
                    self.show_user_dashboard()
            else:
                messagebox.showerror("Error", "Invalid credentials")

        def do_signup():
            self.show_signup()

        tk.Button(frame, text="Login", command=do_login, width=15).grid(row=2, column=0, pady=10)
        tk.Button(frame, text="Signup", command=do_signup, width=15).grid(row=2, column=1, pady=10)

    def show_signup(self):
        self.clear_frame()
        tk.Label(self.root, text="Signup", font=("Arial", 20)).pack(pady=20)

        frame = tk.Frame(self.root)
        frame.pack(pady=20)

        tk.Label(frame, text="Username:").grid(row=0, column=0, padx=5, pady=5)
        signup_user = tk.Entry(frame)
        signup_user.grid(row=0, column=1, padx=5, pady=5)

        tk.Label(frame, text="Password:").grid(row=1, column=0, padx=5, pady=5)
        signup_pass = tk.Entry(frame, show="*")
        signup_pass.grid(row=1, column=1, padx=5, pady=5)

        def do_register():
            uname = signup_user.get()
            pwd = signup_pass.get()
            if not uname or len(pwd) < 4:
                messagebox.showerror("Error", "Username required and password min 4 chars")
                return
            users = read_users()
            if uname in users:
                messagebox.showerror("Error", "Username already exists")
                return
            users[uname] = {'password': pwd, 'role': 'USER', 'points': 0}
            write_users(users)
            messagebox.showinfo("Success", "Account created! Please login.")
            self.show_login()

        tk.Button(frame, text="Register", command=do_register).grid(row=2, column=0, columnspan=2, pady=10)
        tk.Button(self.root, text="Back to Login", command=self.show_login).pack()

    # ------------------------- USER DASHBOARD -------------------------
    def show_user_dashboard(self):
        self.clear_frame()
        tk.Label(self.root, text=f"Welcome {self.current_user}", font=("Arial", 18)).pack(pady=5)
        points = get_user_points(self.current_user)
        tk.Label(self.root, text=f"Your Points: {points}", font=("Arial", 12), fg="blue").pack()

        notebook = ttk.Notebook(self.root)
        notebook.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)

        # Tab 1: My Items
        tab_items = tk.Frame(notebook)
        notebook.add(tab_items, text="My Items")
        self.build_user_items_tab(tab_items)

        # Tab 2: Report Item
        tab_report = tk.Frame(notebook)
        notebook.add(tab_report, text="Report Item")
        self.build_report_tab(tab_report)

        # Tab 3: Search & Claim
        tab_search = tk.Frame(notebook)
        notebook.add(tab_search, text="Search & Claim")
        self.build_search_claim_tab(tab_search)

        # Tab 4: Inbox
        tab_inbox = tk.Frame(notebook)
        notebook.add(tab_inbox, text="Inbox")
        self.build_inbox_tab(tab_inbox)

        # Tab 5: Request Points
        tab_req = tk.Frame(notebook)
        notebook.add(tab_req, text="Request Points")
        self.build_request_tab(tab_req)

        # Tab 6: My Claims
        tab_myclaims = tk.Frame(notebook)
        notebook.add(tab_myclaims, text="My Claims")
        self.build_my_claims_tab(tab_myclaims)

        tk.Button(self.root, text="Logout", command=self.show_login, bg="red", fg="white").pack(pady=5)

    def build_user_items_tab(self, parent):
        frame = tk.Frame(parent)
        frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        tk.Label(frame, text="Items You Reported", font=("Arial", 12)).pack(pady=5)
        self.user_items_list = scrolledtext.ScrolledText(frame, width=80, height=20)
        self.user_items_list.pack(fill=tk.BOTH, expand=True)
        def refresh():
            self.user_items_list.delete(1.0, tk.END)
            items = list_items()
            for item in items:
                if item['owner'] == self.current_user:
                    self.user_items_list.insert(tk.END, f"ID:{item['id']} | {item['type']} | {item['name']} | Bounty:{item['bounty']}\n")
                    self.user_items_list.insert(tk.END, f"  {item['desc']}\n\n")
            if self.user_items_list.get(1.0, tk.END).strip() == "":
                self.user_items_list.insert(tk.END, "No items reported.")
        refresh()
        tk.Button(frame, text="Refresh", command=refresh).pack(pady=5)

    def build_report_tab(self, parent):
        frame = tk.Frame(parent)
        frame.pack(pady=10)
        tk.Label(frame, text="Report Lost/Found Item", font=("Arial", 12)).pack()

        fields = tk.Frame(frame)
        fields.pack(pady=10)

        tk.Label(fields, text="Type:").grid(row=0, column=0, sticky="e")
        typ_var = tk.StringVar(value="LOST")
        ttk.Combobox(fields, textvariable=typ_var, values=["LOST", "FOUND"]).grid(row=0, column=1, padx=5)

        tk.Label(fields, text="Item Name:").grid(row=1, column=0, sticky="e")
        name_entry = tk.Entry(fields, width=30)
        name_entry.grid(row=1, column=1, padx=5)

        tk.Label(fields, text="Category:").grid(row=2, column=0, sticky="e")
        cat_entry = tk.Entry(fields, width=30)
        cat_entry.grid(row=2, column=1, padx=5)

        tk.Label(fields, text="Description:").grid(row=3, column=0, sticky="e")
        desc_entry = tk.Entry(fields, width=30)
        desc_entry.grid(row=3, column=1, padx=5)

        bounty_frame = tk.Frame(fields)
        bounty_frame.grid(row=4, column=0, columnspan=2, pady=5)
        tk.Label(bounty_frame, text="Bounty (points if LOST):").pack(side=tk.LEFT)
        bounty_entry = tk.Entry(bounty_frame, width=10)
        bounty_entry.pack(side=tk.LEFT, padx=5)

        def do_report():
            typ = typ_var.get()
            name = name_entry.get()
            cat = cat_entry.get()
            desc = desc_entry.get()
            if not name or not cat:
                messagebox.showerror("Error", "Name and Category required")
                return
            bounty = 0
            if typ == "LOST":
                try:
                    bounty = int(bounty_entry.get())
                    if bounty < 0:
                        bounty = 0
                except:
                    bounty = 0
                if get_user_points(self.current_user) < bounty:
                    messagebox.showerror("Error", f"Insufficient points. You have {get_user_points(self.current_user)} points.")
                    return
            save_item(self.current_user, typ, name, cat, desc, bounty)
            messagebox.showinfo("Success", "Item reported")
            name_entry.delete(0, tk.END)
            cat_entry.delete(0, tk.END)
            desc_entry.delete(0, tk.END)
            bounty_entry.delete(0, tk.END)

        tk.Button(frame, text="Submit", command=do_report).pack(pady=10)

    def build_search_claim_tab(self, parent):
        notebook = ttk.Notebook(parent)
        notebook.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        # Search sub-tab
        search_tab = tk.Frame(notebook)
        notebook.add(search_tab, text="Search")
        keyword_entry = tk.Entry(search_tab)
        keyword_entry.pack(pady=5)
        search_result = scrolledtext.ScrolledText(search_tab, width=80, height=20)
        search_result.pack(fill=tk.BOTH, expand=True)
        def do_search():
            keyword = keyword_entry.get().lower()
            search_result.delete(1.0, tk.END)
            items = list_items()
            found = False
            for item in items:
                if keyword in item['name'].lower() or keyword in item['category'].lower() or keyword in item['desc'].lower():
                    search_result.insert(tk.END, f"ID:{item['id']} | {item['type']} | {item['name']} | Bounty:{item['bounty']}\n")
                    search_result.insert(tk.END, f"Owner: {item['owner']}\n{item['desc']}\n\n")
                    found = True
            if not found:
                search_result.insert(tk.END, "No matching items.")
        tk.Button(search_tab, text="Search", command=do_search).pack(pady=5)

        # Claim sub-tab
        claim_tab = tk.Frame(notebook)
        notebook.add(claim_tab, text="Claim Item")
        tk.Label(claim_tab, text="Enter Item ID to claim:").pack(pady=5)
        claim_id_entry = tk.Entry(claim_tab)
        claim_id_entry.pack(pady=5)
        def do_claim():
            try:
                item_id = int(claim_id_entry.get())
            except:
                messagebox.showerror("Error", "Invalid Item ID")
                return
            item = get_item_by_id(item_id)
            if not item:
                messagebox.showerror("Error", "Item not found")
                return
            if item['owner'] == self.current_user:
                messagebox.showerror("Error", "You cannot claim your own item")
                return
            # check duplicate pending
            for c in list_claims():
                if c['item_id'] == item_id and c['claimant'] == self.current_user and c['status'] == 'PENDING':
                    messagebox.showerror("Error", "You already have a pending claim on this item")
                    return
            claim_id = save_claim(item_id, self.current_user)
            messagebox.showinfo("Success", f"Claim submitted! Claim ID: {claim_id}")
            claim_id_entry.delete(0, tk.END)
        tk.Button(claim_tab, text="Submit Claim", command=do_claim).pack(pady=5)

    def build_inbox_tab(self, parent):
        inbox_text = scrolledtext.ScrolledText(parent, width=80, height=25)
        inbox_text.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        def refresh():
            inbox_text.delete(1.0, tk.END)
            msgs = get_messages_for_user(self.current_user)
            for m in msgs:
                dt = datetime.fromtimestamp(m['timestamp']).strftime("%Y-%m-%d %H:%M")
                inbox_text.insert(tk.END, f"From: {m['from']} [{dt}]\n{m['message']}\n\n")
        refresh()
        tk.Button(parent, text="Refresh", command=refresh).pack(pady=5)

    def build_request_tab(self, parent):
        frame = tk.Frame(parent)
        frame.pack(pady=10)
        tk.Label(frame, text="Request Points from Admin", font=("Arial", 12)).pack()
        inner = tk.Frame(frame)
        inner.pack(pady=10)
        tk.Label(inner, text="Points:").grid(row=0, column=0)
        pts_entry = tk.Entry(inner, width=10)
        pts_entry.grid(row=0, column=1, padx=5)
        tk.Label(inner, text="Reason:").grid(row=1, column=0)
        reason_entry = tk.Entry(inner, width=40)
        reason_entry.grid(row=1, column=1, padx=5)
        def send():
            try:
                pts = int(pts_entry.get())
                if pts <= 0:
                    raise ValueError
            except:
                messagebox.showerror("Error", "Enter valid positive points")
                return
            reason = reason_entry.get().strip()
            if not reason:
                reason = "No reason given"
            msg = f"{pts}|{reason}"
            send_message(self.current_user, "admin", msg, "POINTS_REQUEST")
            messagebox.showinfo("Sent", "Request sent to admin")
            pts_entry.delete(0, tk.END)
            reason_entry.delete(0, tk.END)
        tk.Button(frame, text="Send Request", command=send).pack(pady=10)

    def build_my_claims_tab(self, parent):
        claims_text = scrolledtext.ScrolledText(parent, width=80, height=20)
        claims_text.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        def refresh():
            claims_text.delete(1.0, tk.END)
            claims = list_claims()
            for c in claims:
                if c['claimant'] == self.current_user:
                    item = get_item_by_id(c['item_id'])
                    item_name = item['name'] if item else "Unknown"
                    claims_text.insert(tk.END, f"Claim ID:{c['id']} | Item:{item_name} (ID:{c['item_id']}) | Status:{c['status']}\n")
            if claims_text.get(1.0, tk.END).strip() == "":
                claims_text.insert(tk.END, "No claims made.")
        refresh()
        tk.Button(parent, text="Refresh", command=refresh).pack(pady=5)

    # ------------------------- ADMIN DASHBOARD -------------------------
    def show_admin_dashboard(self):
        self.clear_frame()
        tk.Label(self.root, text=f"Admin Panel - {self.current_user}", font=("Arial", 18)).pack(pady=5)

        notebook = ttk.Notebook(self.root)
        notebook.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)

        # Tab: All Items
        tab_items = tk.Frame(notebook)
        notebook.add(tab_items, text="All Items")
        self.build_admin_items_tab(tab_items)

        # Tab: Manage Claims
        tab_claims = tk.Frame(notebook)
        notebook.add(tab_claims, text="Manage Claims")
        self.build_admin_claims_tab(tab_claims)

        # Tab: Point Requests
        tab_req = tk.Frame(notebook)
        notebook.add(tab_req, text="Point Requests")
        self.build_admin_requests_tab(tab_req)

        # Tab: Send Message
        tab_msg = tk.Frame(notebook)
        notebook.add(tab_msg, text="Send Message")
        self.build_admin_send_tab(tab_msg)

        # Tab: Logs
        tab_logs = tk.Frame(notebook)
        notebook.add(tab_logs, text="Logs")
        self.build_logs_tab(tab_logs)

        tk.Button(self.root, text="Logout", command=self.show_login, bg="red", fg="white").pack(pady=5)

    def build_admin_items_tab(self, parent):
        items_text = scrolledtext.ScrolledText(parent, width=90, height=25)
        items_text.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        def refresh():
            items_text.delete(1.0, tk.END)
            items = list_items()
            for item in items:
                items_text.insert(tk.END, f"ID:{item['id']} | Owner:{item['owner']} | {item['type']} | {item['name']} | Bounty:{item['bounty']}\n")
                items_text.insert(tk.END, f"  {item['desc']}\n\n")
        refresh()
        tk.Button(parent, text="Refresh", command=refresh).pack(pady=5)

    def build_admin_claims_tab(self, parent):
        # Treeview for claims
        columns = ("Claim ID", "Item ID", "Item Name", "Claimant", "Status", "Bounty")
        tree = ttk.Treeview(parent, columns=columns, show="headings", height=15)
        for col in columns:
            tree.heading(col, text=col)
            tree.column(col, width=100)
        tree.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        scrollbar = ttk.Scrollbar(parent, orient=tk.VERTICAL, command=tree.yview)
        tree.configure(yscrollcommand=scrollbar.set)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)

        def refresh():
            for row in tree.get_children():
                tree.delete(row)
            claims = list_claims()
            for claim in claims:
                item = get_item_by_id(claim['item_id'])
                item_name = item['name'] if item else "Unknown"
                bounty = item['bounty'] if item else 0
                tree.insert("", tk.END, values=(claim['id'], claim['item_id'], item_name, claim['claimant'], claim['status'], bounty))
        refresh()

        def approve():
            selected = tree.selection()
            if not selected:
                messagebox.showerror("Error", "Select a claim")
                return
            claim_id = int(tree.item(selected[0])['values'][0])
            if approve_claim_with_points(claim_id):
                messagebox.showinfo("Success", "Claim approved and points transferred")
            else:
                messagebox.showerror("Error", "Approval failed (maybe already processed or insufficient owner points)")
            refresh()

        def reject():
            selected = tree.selection()
            if not selected:
                messagebox.showerror("Error", "Select a claim")
                return
            claim_id = int(tree.item(selected[0])['values'][0])
            if update_claim_status(claim_id, "REJECTED"):
                messagebox.showinfo("Info", "Claim rejected")
            else:
                messagebox.showerror("Error", "Rejection failed")
            refresh()

        btn_frame = tk.Frame(parent)
        btn_frame.pack(pady=5)
        tk.Button(btn_frame, text="Approve", command=approve, bg="green", fg="white").pack(side=tk.LEFT, padx=5)
        tk.Button(btn_frame, text="Reject", command=reject, bg="orange").pack(side=tk.LEFT, padx=5)
        tk.Button(btn_frame, text="Refresh", command=refresh).pack(side=tk.LEFT, padx=5)

    def build_admin_requests_tab(self, parent):
        requests_list = scrolledtext.ScrolledText(parent, width=90, height=20)
        requests_list.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        def refresh():
            requests_list.delete(1.0, tk.END)
            reqs = get_point_requests()
            for r in reqs:
                dt = datetime.fromtimestamp(r['timestamp']).strftime("%Y-%m-%d %H:%M")
                requests_list.insert(tk.END, f"ID:{r['id']} | From: {r['from']} [{dt}]\n{r['message']}\n\n")
            if not reqs:
                requests_list.insert(tk.END, "No pending point requests.")
        refresh()

        frame = tk.Frame(parent)
        frame.pack(pady=5)
        tk.Label(frame, text="Enter Message ID to award:").pack(side=tk.LEFT)
        msg_id_entry = tk.Entry(frame, width=10)
        msg_id_entry.pack(side=tk.LEFT, padx=5)

        def award():
            try:
                msg_id = int(msg_id_entry.get())
            except:
                messagebox.showerror("Error", "Invalid ID")
                return
            reqs = get_point_requests()
            found = None
            for r in reqs:
                if r['id'] == msg_id:
                    found = r
                    break
            if not found:
                messagebox.showerror("Error", "Request not found")
                return
            parts = found['message'].split('|')
            if len(parts) < 2:
                messagebox.showerror("Error", "Malformed request")
                return
            try:
                pts = int(parts[0])
            except:
                messagebox.showerror("Error", "Invalid points")
                return
            # Award points
            if update_user_points(found['from'], pts):
                delete_message_by_id(msg_id)
                send_message("admin", found['from'], f"Your request for {pts} points has been granted.", "POINTS_GRANTED")
                messagebox.showinfo("Success", f"Awarded {pts} points to {found['from']}")
                refresh()
                msg_id_entry.delete(0, tk.END)
            else:
                messagebox.showerror("Error", "User not found")

        tk.Button(frame, text="Award Points", command=award).pack(side=tk.LEFT, padx=5)
        tk.Button(frame, text="Refresh", command=refresh).pack(side=tk.LEFT, padx=5)

    def build_admin_send_tab(self, parent):
        frame = tk.Frame(parent)
        frame.pack(pady=10)
        tk.Label(frame, text="To Username:").grid(row=0, column=0)
        to_entry = tk.Entry(frame, width=20)
        to_entry.grid(row=0, column=1, padx=5)
        tk.Label(frame, text="Message:").grid(row=1, column=0)
        msg_entry = tk.Entry(frame, width=50)
        msg_entry.grid(row=1, column=1, padx=5)
        def send():
            to = to_entry.get().strip()
            msg = msg_entry.get().strip()
            if not to or not msg:
                messagebox.showerror("Error", "All fields required")
                return
            users = read_users()
            if to not in users:
                messagebox.showerror("Error", "User not found")
                return
            send_message("admin", to, msg, "ADMIN_MSG")
            messagebox.showinfo("Sent", f"Message sent to {to}")
            to_entry.delete(0, tk.END)
            msg_entry.delete(0, tk.END)
        tk.Button(frame, text="Send", command=send).grid(row=2, column=0, columnspan=2, pady=10)

    def build_logs_tab(self, parent):
        logs_text = scrolledtext.ScrolledText(parent, width=90, height=25)
        logs_text.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        def refresh():
            logs_text.delete(1.0, tk.END)
            if os.path.exists(LOG_FILE):
                with open(LOG_FILE, 'r') as f:
                    for line in f:
                        parts = line.strip().split(' - ', 1)
                        if len(parts) == 2:
                            ts, act = parts
                            dt = datetime.fromtimestamp(int(ts)).strftime("%Y-%m-%d %H:%M:%S")
                            logs_text.insert(tk.END, f"[{dt}] {act}\n")
                        else:
                            logs_text.insert(tk.END, line)
        refresh()
        tk.Button(parent, text="Refresh", command=refresh).pack(pady=5)

# ---------------------------- MAIN ----------------------------
if __name__ == "__main__":
    root = tk.Tk()
    app = LostFoundApp(root)
    root.mainloop()