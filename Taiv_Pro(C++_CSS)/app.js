const STATUS_OPTIONS = [
  "New Lead",
  "Contacted",
  "Meeting Booked",
  "Proposal Sent",
  "Active Campaign",
  "Follow-Up Needed",
  "Renewal Opportunity",
  "Closed Won",
  "Closed Lost"
];

const PRIORITY_OPTIONS = ["Low", "Medium", "High", "Urgent"];

const STORAGE_KEY = "taiv-account-tracker-studio-v1";

const demoAccounts = [
  {
    id: crypto.randomUUID(),
    accountName: "Downtown Sports Bar",
    venueType: "Sports Bar",
    contactPerson: "Decision Maker",
    email: "manager@downtownsports.example",
    phone: "204-555-0141",
    status: "Contacted",
    priority: "High",
    revenueOpportunity: 12000,
    lastFollowUp: today(),
    nextFollowUp: addDays(3),
    owner: "Andrii",
    notes: "Potential Taiv venue account. Needs follow-up about TV advertising, campaign options, screen count and decision maker details."
  },
  {
    id: crypto.randomUUID(),
    accountName: "Northside Restaurant",
    venueType: "Restaurant",
    contactPerson: "General Manager",
    email: "gm@northside.example",
    phone: "204-555-0179",
    status: "Meeting Booked",
    priority: "Medium",
    revenueOpportunity: 8500,
    lastFollowUp: today(),
    nextFollowUp: addDays(6),
    owner: "Andrii",
    notes: "Demo booked. Track next steps, campaign notes, onboarding questions and proposal follow-up."
  },
  {
    id: crypto.randomUUID(),
    accountName: "Campus Pub",
    venueType: "Pub / Sports Venue",
    contactPerson: "Owner",
    email: "owner@campuspub.example",
    phone: "204-555-0190",
    status: "Follow-Up Needed",
    priority: "Urgent",
    revenueOpportunity: 15000,
    lastFollowUp: addDays(-4),
    nextFollowUp: today(),
    owner: "Andrii",
    notes: "High priority account. Confirm decision maker, campaign interest, venue traffic and next appointment."
  },
  {
    id: crypto.randomUUID(),
    accountName: "West End Grill",
    venueType: "Restaurant / Bar",
    contactPerson: "Manager",
    email: "manager@westendgrill.example",
    phone: "204-555-0163",
    status: "Proposal Sent",
    priority: "High",
    revenueOpportunity: 18000,
    lastFollowUp: addDays(-2),
    nextFollowUp: addDays(2),
    owner: "Andrii",
    notes: "Proposal sent. Need follow-up on package fit, screens, traffic, campaign expectations and advertiser opportunity."
  }
];

let accounts = loadAccounts();
let editingId = null;
let dueOnly = false;

const els = {
  navItems: document.querySelectorAll(".nav-item"),
  views: document.querySelectorAll(".view"),
  kpiAccounts: document.getElementById("kpiAccounts"),
  kpiPipeline: document.getElementById("kpiPipeline"),
  kpiDue: document.getElementById("kpiDue"),
  kpiHot: document.getElementById("kpiHot"),
  followupQueue: document.getElementById("followupQueue"),
  statusMix: document.getElementById("statusMix"),
  rows: document.getElementById("accountRows"),
  search: document.getElementById("searchInput"),
  statusFilter: document.getElementById("statusFilter"),
  priorityFilter: document.getElementById("priorityFilter"),
  ownerFilter: document.getElementById("ownerFilter"),
  dialog: document.getElementById("accountDialog"),
  form: document.getElementById("accountForm"),
  modalTitle: document.getElementById("modalTitle"),
  deleteBtn: document.getElementById("deleteBtn"),
  toast: document.getElementById("toast")
};

function today() {
  return new Date().toISOString().slice(0, 10);
}

function addDays(days) {
  const d = new Date();
  d.setDate(d.getDate() + days);
  return d.toISOString().slice(0, 10);
}

function loadAccounts() {
  const saved = localStorage.getItem(STORAGE_KEY);
  if (!saved) {
    localStorage.setItem(STORAGE_KEY, JSON.stringify(demoAccounts));
    return [...demoAccounts];
  }

  try {
    return JSON.parse(saved);
  } catch {
    return [...demoAccounts];
  }
}

function saveAccounts() {
  localStorage.setItem(STORAGE_KEY, JSON.stringify(accounts));
}

function money(value) {
  const n = Number(value || 0);
  return n.toLocaleString("en-CA", {
    style: "currency",
    currency: "CAD",
    maximumFractionDigits: 0
  });
}

function isDue(account) {
  return account.nextFollowUp && account.nextFollowUp <= today() && !["Closed Won", "Closed Lost"].includes(account.status);
}

function isHot(account) {
  return ["High", "Urgent"].includes(account.priority);
}

function switchView(view) {
  els.navItems.forEach(btn => btn.classList.toggle("active", btn.dataset.view === view));
  els.views.forEach(section => section.classList.remove("active"));
  document.getElementById(`${view}View`).classList.add("active");
}

function setupSelectors() {
  STATUS_OPTIONS.forEach(s => {
    els.statusFilter.add(new Option(s, s));
    els.form.status.add(new Option(s, s));
  });

  PRIORITY_OPTIONS.forEach(p => {
    els.priorityFilter.add(new Option(p, p));
    els.form.priority.add(new Option(p, p));
  });
}

function filteredAccounts() {
  const search = (els.search?.value || "").toLowerCase().trim();
  const status = els.statusFilter?.value || "All";
  const priority = els.priorityFilter?.value || "All";
  const owner = (els.ownerFilter?.value || "").toLowerCase().trim();

  return accounts.filter(a => {
    const haystack = Object.values(a).join(" ").toLowerCase();

    if (dueOnly && !isDue(a)) return false;
    if (search && !haystack.includes(search)) return false;
    if (status !== "All" && a.status !== status) return false;
    if (priority !== "All" && a.priority !== priority) return false;
    if (owner && !(a.owner || "").toLowerCase().includes(owner)) return false;

    return true;
  });
}

function renderKpis() {
  const pipeline = accounts
    .filter(a => a.status !== "Closed Lost")
    .reduce((sum, a) => sum + Number(a.revenueOpportunity || 0), 0);

  els.kpiAccounts.textContent = accounts.length;
  els.kpiPipeline.textContent = money(pipeline);
  els.kpiDue.textContent = accounts.filter(isDue).length;
  els.kpiHot.textContent = accounts.filter(isHot).length;
}

function renderFollowups() {
  const due = accounts
    .filter(isDue)
    .sort((a, b) => a.nextFollowUp.localeCompare(b.nextFollowUp));

  if (!due.length) {
    els.followupQueue.innerHTML = `<div class="empty">No overdue follow-ups. Pipeline is clean.</div>`;
    return;
  }

  els.followupQueue.innerHTML = due.map(account => `
    <div class="queue-item" data-id="${account.id}">
      <div>
        <h3>${escapeHtml(account.accountName)}</h3>
        <p>${escapeHtml(account.contactPerson || "No contact")} • ${escapeHtml(account.status)} • ${escapeHtml(account.notes || "No notes")}</p>
      </div>
      <span class="due-date">${escapeHtml(account.nextFollowUp)}</span>
    </div>
  `).join("");

  document.querySelectorAll(".queue-item").forEach(item => {
    item.addEventListener("click", () => openEdit(item.dataset.id));
  });
}

function renderStatusMix() {
  const counts = STATUS_OPTIONS.map(status => ({
    status,
    count: accounts.filter(a => a.status === status).length
  })).filter(x => x.count > 0);

  const max = Math.max(1, ...counts.map(x => x.count));

  if (!counts.length) {
    els.statusMix.innerHTML = `<div class="empty">No account data yet.</div>`;
    return;
  }

  els.statusMix.innerHTML = counts.map(x => `
    <div class="status-bar">
      <label>${escapeHtml(x.status)}</label>
      <strong>${x.count}</strong>
      <div class="bar-track">
        <div class="bar-fill" style="width:${Math.round((x.count / max) * 100)}%"></div>
      </div>
    </div>
  `).join("");
}

function chipClass(type, value) {
  if (type === "status") {
    if (value === "Closed Won") return "chip won";
    if (value === "Closed Lost") return "chip lost";
    return "chip status";
  }

  return `chip priority-${String(value).toLowerCase()}`;
}

function renderRows() {
  const list = filteredAccounts();

  if (!list.length) {
    els.rows.innerHTML = `<tr><td colspan="8" class="empty">No accounts match this view.</td></tr>`;
    return;
  }

  els.rows.innerHTML = list.map(account => `
    <tr>
      <td>
        <div class="account-cell">
          <strong>${escapeHtml(account.accountName)}</strong>
          <small>${escapeHtml(account.venueType || "No venue type")}</small>
        </div>
      </td>
      <td>
        <div class="contact-cell">
          <strong>${escapeHtml(account.contactPerson || "No contact")}</strong>
          <small>${escapeHtml(account.email || account.phone || "No contact info")}</small>
        </div>
      </td>
      <td><span class="${chipClass("status", account.status)}">${escapeHtml(account.status)}</span></td>
      <td><span class="${chipClass("priority", account.priority)}">${escapeHtml(account.priority)}</span></td>
      <td><strong>${money(account.revenueOpportunity)}</strong></td>
      <td>${isDue(account) ? `<span class="due-date">${escapeHtml(account.nextFollowUp)}</span>` : escapeHtml(account.nextFollowUp || "—")}</td>
      <td>${escapeHtml(account.owner || "—")}</td>
      <td>
        <div class="row-actions">
          <button class="row-btn" data-edit="${account.id}">Edit</button>
        </div>
      </td>
    </tr>
  `).join("");

  document.querySelectorAll("[data-edit]").forEach(btn => {
    btn.addEventListener("click", () => openEdit(btn.dataset.edit));
  });
}

function renderAll() {
  renderKpis();
  renderFollowups();
  renderStatusMix();
  renderRows();
}

function openNew() {
  editingId = null;
  els.form.reset();
  els.modalTitle.textContent = "New account";
  els.deleteBtn.style.display = "none";
  els.form.status.value = "New Lead";
  els.form.priority.value = "Medium";
  els.form.lastFollowUp.value = today();
  els.form.nextFollowUp.value = addDays(3);
  els.form.owner.value = "Andrii";
  refreshCustomSelects();
  refreshCustomSelects();
  els.dialog.showModal();
}

function openEdit(id) {
  const account = accounts.find(a => a.id === id);
  if (!account) return;

  editingId = id;
  els.modalTitle.textContent = "Edit account";
  els.deleteBtn.style.display = "inline-flex";

  for (const [key, value] of Object.entries(account)) {
    if (els.form.elements[key]) els.form.elements[key].value = value ?? "";
  }

  els.dialog.showModal();
}

function closeModal() {
  els.dialog.close();
}

function formToAccount() {
  const data = Object.fromEntries(new FormData(els.form).entries());
  return {
    id: editingId || crypto.randomUUID(),
    accountName: data.accountName.trim(),
    venueType: data.venueType.trim(),
    contactPerson: data.contactPerson.trim(),
    email: data.email.trim(),
    phone: data.phone.trim(),
    status: data.status,
    priority: data.priority,
    revenueOpportunity: Number(data.revenueOpportunity || 0),
    lastFollowUp: data.lastFollowUp || today(),
    nextFollowUp: data.nextFollowUp || addDays(3),
    owner: data.owner.trim() || "Andrii",
    notes: data.notes.trim()
  };
}

function saveForm() {
  const account = formToAccount();

  if (editingId) {
    accounts = accounts.map(a => a.id === editingId ? account : a);
    toast("Account updated");
  } else {
    accounts.unshift(account);
    toast("Account added");
  }

  saveAccounts();
  closeModal();
  renderAll();
}

function deleteCurrent() {
  if (!editingId) return;

  accounts = accounts.filter(a => a.id !== editingId);
  saveAccounts();
  closeModal();
  renderAll();
  toast("Account deleted");
}

function markToday() {
  els.form.lastFollowUp.value = today();
  els.form.status.value = "Contacted";
  toast("Marked contacted today");
}

function exportCsv() {
  const rows = filteredAccounts();
  const headers = [
    "Account Name",
    "Venue Type",
    "Contact Person",
    "Email",
    "Phone",
    "Status",
    "Priority",
    "Revenue Opportunity",
    "Last Follow-Up",
    "Next Follow-Up",
    "Owner",
    "Campaign Notes"
  ];

  const csv = [
    headers.join(","),
    ...rows.map(a => [
      a.accountName,
      a.venueType,
      a.contactPerson,
      a.email,
      a.phone,
      a.status,
      a.priority,
      a.revenueOpportunity,
      a.lastFollowUp,
      a.nextFollowUp,
      a.owner,
      a.notes
    ].map(csvCell).join(","))
  ].join("\n");

  const blob = new Blob([csv], { type: "text/csv;charset=utf-8" });
  const url = URL.createObjectURL(blob);
  const link = document.createElement("a");
  link.href = url;
  link.download = "taiv-account-tracker-export.csv";
  link.click();
  URL.revokeObjectURL(url);
  toast("CSV exported");
}

function csvCell(value) {
  const s = String(value ?? "");
  return `"${s.replaceAll('"', '""')}"`;
}

function resetDemo() {
  if (!confirm("Reset to demo data? This will replace current local data.")) return;
  accounts = demoAccounts.map(a => ({ ...a, id: crypto.randomUUID() }));
  saveAccounts();
  dueOnly = false;
  renderAll();
  toast("Demo reset");
}

function toast(message) {
  els.toast.textContent = message;
  els.toast.classList.add("show");
  setTimeout(() => els.toast.classList.remove("show"), 1800);
}

function escapeHtml(value) {
  return String(value ?? "").replace(/[&<>"']/g, char => ({
    "&": "&amp;",
    "<": "&lt;",
    ">": "&gt;",
    '"': "&quot;",
    "'": "&#039;"
  }[char]));
}

function bindEvents() {
  els.navItems.forEach(btn => {
    btn.addEventListener("click", () => switchView(btn.dataset.view));
  });

  document.getElementById("newAccountBtn").addEventListener("click", openNew);
  document.getElementById("exportBtn").addEventListener("click", exportCsv);
  document.getElementById("seedDemoBtn").addEventListener("click", resetDemo);
  document.getElementById("showDueBtn").addEventListener("click", () => {
    dueOnly = true;
    switchView("accounts");
    renderAll();
  });

  document.getElementById("closeModalBtn").addEventListener("click", closeModal);
  document.getElementById("cancelBtn").addEventListener("click", closeModal);
  document.getElementById("deleteBtn").addEventListener("click", deleteCurrent);
  document.getElementById("markTodayBtn").addEventListener("click", markToday);

  els.form.addEventListener("submit", event => {
    event.preventDefault();
    saveForm();
  });

  [els.search, els.statusFilter, els.priorityFilter, els.ownerFilter].forEach(el => {
    el.addEventListener("input", () => {
      dueOnly = false;
      renderRows();
    });
    el.addEventListener("change", () => {
      dueOnly = false;
      renderRows();
    });
  });
}


function enhanceSelects() {
  document.querySelectorAll("select").forEach(select => {
    if (select.dataset.enhanced === "true") return;

    select.dataset.enhanced = "true";
    select.classList.add("native-hidden");

    const wrapper = document.createElement("div");
    wrapper.className = "custom-select";

    const trigger = document.createElement("button");
    trigger.type = "button";
    trigger.className = "custom-select-trigger";
    trigger.innerHTML = `
      <span class="custom-select-value"></span>
      <span class="custom-select-arrow" aria-hidden="true"></span>
    `;

    const menu = document.createElement("div");
    menu.className = "custom-select-menu";

    select.parentNode.insertBefore(wrapper, select.nextSibling);
    wrapper.appendChild(trigger);
    wrapper.appendChild(menu);

    function rebuildOptions() {
      menu.innerHTML = "";
      Array.from(select.options).forEach(option => {
        const item = document.createElement("button");
        item.type = "button";
        item.className = "custom-select-option";
        item.textContent = option.textContent;
        item.dataset.value = option.value;

        if (option.value === select.value) item.classList.add("selected");

        item.addEventListener("click", () => {
          select.value = option.value;
          select.dispatchEvent(new Event("change", { bubbles: true }));
          select.dispatchEvent(new Event("input", { bubbles: true }));
          sync();
          closeAllCustomSelects();
        });

        menu.appendChild(item);
      });
    }

    function sync() {
      const selected = select.options[select.selectedIndex];
      trigger.querySelector(".custom-select-value").textContent = selected ? selected.textContent : "Select";
      menu.querySelectorAll(".custom-select-option").forEach(item => {
        item.classList.toggle("selected", item.dataset.value === select.value);
      });
    }

    trigger.addEventListener("click", event => {
      event.stopPropagation();
      const wasOpen = wrapper.classList.contains("open");
      closeAllCustomSelects();
      if (!wasOpen) wrapper.classList.add("open");
    });

    select.addEventListener("change", sync);

    rebuildOptions();
    sync();

    select._customSelectRebuild = () => {
      rebuildOptions();
      sync();
    };
  });
}

function refreshCustomSelects() {
  document.querySelectorAll("select").forEach(select => {
    if (select._customSelectRebuild) select._customSelectRebuild();
  });
}

function closeAllCustomSelects() {
  document.querySelectorAll(".custom-select.open").forEach(el => el.classList.remove("open"));
}

document.addEventListener("click", closeAllCustomSelects);
document.addEventListener("keydown", event => {
  if (event.key === "Escape") closeAllCustomSelects();
});

setupSelectors();
enhanceSelects();
bindEvents();
renderAll();
