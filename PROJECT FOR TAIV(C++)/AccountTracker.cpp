
// Taiv Account Tracker
// C++17 / Win32 desktop application
// Created for Andrii Pavlenko as a Taiv project demo.
//
// Build with Visual Studio Developer Command Prompt:
//   build_msvc.bat
//
// Or with MinGW:
//   build_mingw.bat

#define UNICODE
#define _UNICODE

#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")

using std::wstring;
using std::vector;

struct Account {
    wstring accountName;
    wstring venueType;
    wstring contactPerson;
    wstring email;
    wstring phone;
    wstring status;
    wstring priority;
    wstring revenueOpportunity;
    wstring lastFollowUp;
    wstring nextFollowUp;
    wstring owner;
    wstring notes;
};

HINSTANCE g_hInst;
HWND g_mainWindow;
HWND g_listView;
HWND g_summaryLabel;
HWND g_searchBox;

HWND eAccountName, eVenueType, eContactPerson, eEmail, ePhone;
HWND cStatus, cPriority;
HWND eRevenueOpportunity, eLastFollowUp, eNextFollowUp, eOwner, eNotes;

vector<Account> g_accounts;
int g_selectedIndex = -1;
bool g_showDueOnly = false;

const int ID_LIST = 100;
const int ID_ADD = 101;
const int ID_UPDATE = 102;
const int ID_DELETE = 103;
const int ID_CLEAR = 104;
const int ID_SHOW_DUE = 105;
const int ID_SHOW_ALL = 106;
const int ID_SEARCH = 107;
const int ID_TODAY = 108;

const wchar_t* STATUS_OPTIONS[] = {
    L"New Lead",
    L"Contacted",
    L"Meeting Booked",
    L"Proposal Sent",
    L"Active Campaign",
    L"Follow-Up Needed",
    L"Renewal Opportunity",
    L"Closed Won",
    L"Closed Lost"
};

const wchar_t* PRIORITY_OPTIONS[] = {
    L"Low",
    L"Medium",
    L"High",
    L"Urgent"
};

wstring GetWindowTextString(HWND hwnd) {
    int len = GetWindowTextLengthW(hwnd);
    wstring text(len, L'\0');
    GetWindowTextW(hwnd, text.data(), len + 1);
    return text;
}

void SetWindowTextSafe(HWND hwnd, const wstring& text) {
    SetWindowTextW(hwnd, text.c_str());
}

wstring ToLower(wstring s) {
    std::transform(s.begin(), s.end(), s.begin(), ::towlower);
    return s;
}

wstring Trim(const wstring& s) {
    size_t start = s.find_first_not_of(L" \t\r\n");
    if (start == wstring::npos) return L"";
    size_t end = s.find_last_not_of(L" \t\r\n");
    return s.substr(start, end - start + 1);
}

wstring TodayDate() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    wchar_t buffer[20];
    swprintf_s(buffer, L"%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
    return buffer;
}

std::string WideToUtf8(const wstring& w) {
    if (w.empty()) return "";
    int size = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string result(size - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, result.data(), size, nullptr, nullptr);
    return result;
}

wstring Utf8ToWide(const std::string& s) {
    if (s.empty()) return L"";
    int size = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    wstring result(size - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, result.data(), size);
    return result;
}

wstring GetDataFolder() {
    wchar_t path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, path))) {
        wstring folder = wstring(path) + L"\\TaivAccountTracker";
        CreateDirectoryW(folder.c_str(), NULL);
        return folder;
    }

    return L".";
}

wstring GetCsvPath() {
    return GetDataFolder() + L"\\accounts.csv";
}

std::string EscapeCsv(const wstring& value) {
    std::string utf8 = WideToUtf8(value);
    bool needsQuotes = utf8.find(',') != std::string::npos ||
                       utf8.find('"') != std::string::npos ||
                       utf8.find('\n') != std::string::npos ||
                       utf8.find('\r') != std::string::npos;

    std::string out;
    for (char ch : utf8) {
        if (ch == '"') out += "\"\"";
        else out += ch;
    }

    if (needsQuotes) return "\"" + out + "\"";
    return out;
}

vector<std::string> ParseCsvLine(const std::string& line) {
    vector<std::string> result;
    std::string current;
    bool inQuotes = false;

    for (size_t i = 0; i < line.size(); ++i) {
        char ch = line[i];

        if (inQuotes) {
            if (ch == '"') {
                if (i + 1 < line.size() && line[i + 1] == '"') {
                    current += '"';
                    ++i;
                } else {
                    inQuotes = false;
                }
            } else {
                current += ch;
            }
        } else {
            if (ch == '"') {
                inQuotes = true;
            } else if (ch == ',') {
                result.push_back(current);
                current.clear();
            } else {
                current += ch;
            }
        }
    }

    result.push_back(current);
    return result;
}

void SaveAccounts() {
    std::ofstream file(WideToUtf8(GetCsvPath()), std::ios::binary);
    file << "Account Name,Venue Type,Contact Person,Email,Phone,Status,Priority,Revenue Opportunity,Last Follow-Up,Next Follow-Up,Owner,Campaign Notes\n";

    for (const auto& a : g_accounts) {
        file << EscapeCsv(a.accountName) << ","
             << EscapeCsv(a.venueType) << ","
             << EscapeCsv(a.contactPerson) << ","
             << EscapeCsv(a.email) << ","
             << EscapeCsv(a.phone) << ","
             << EscapeCsv(a.status) << ","
             << EscapeCsv(a.priority) << ","
             << EscapeCsv(a.revenueOpportunity) << ","
             << EscapeCsv(a.lastFollowUp) << ","
             << EscapeCsv(a.nextFollowUp) << ","
             << EscapeCsv(a.owner) << ","
             << EscapeCsv(a.notes) << "\n";
    }
}

Account MakeSampleAccount(
    const wstring& name,
    const wstring& type,
    const wstring& contact,
    const wstring& status,
    const wstring& priority,
    const wstring& revenue,
    const wstring& next,
    const wstring& notes
) {
    Account a;
    a.accountName = name;
    a.venueType = type;
    a.contactPerson = contact;
    a.email = L"";
    a.phone = L"";
    a.status = status;
    a.priority = priority;
    a.revenueOpportunity = revenue;
    a.lastFollowUp = TodayDate();
    a.nextFollowUp = next;
    a.owner = L"Andrii";
    a.notes = notes;
    return a;
}

void CreateSampleData() {
    g_accounts.clear();
    g_accounts.push_back(MakeSampleAccount(
        L"Downtown Sports Bar",
        L"Sports Bar",
        L"Decision Maker",
        L"Contacted",
        L"High",
        L"$12,000",
        L"2026-07-05",
        L"Potential Taiv venue account. Needs follow-up about TV advertising and campaign options."
    ));

    g_accounts.push_back(MakeSampleAccount(
        L"Northside Restaurant",
        L"Restaurant",
        L"General Manager",
        L"Meeting Booked",
        L"Medium",
        L"$8,500",
        L"2026-07-08",
        L"Demo booked. Track next steps, campaign notes, and onboarding questions."
    ));

    g_accounts.push_back(MakeSampleAccount(
        L"Campus Pub",
        L"Pub / Sports Venue",
        L"Owner",
        L"Follow-Up Needed",
        L"Urgent",
        L"$15,000",
        L"2026-07-01",
        L"High priority account. Confirm decision maker and campaign interest."
    ));

    SaveAccounts();
}

void LoadAccounts() {
    g_accounts.clear();
    std::ifstream file(WideToUtf8(GetCsvPath()), std::ios::binary);

    if (!file.good()) {
        CreateSampleData();
        return;
    }

    std::string line;
    bool first = true;

    while (std::getline(file, line)) {
        if (first) {
            first = false;
            continue;
        }

        if (line.empty()) continue;

        auto cols = ParseCsvLine(line);
        if (cols.size() < 12) continue;

        Account a;
        a.accountName = Utf8ToWide(cols[0]);
        a.venueType = Utf8ToWide(cols[1]);
        a.contactPerson = Utf8ToWide(cols[2]);
        a.email = Utf8ToWide(cols[3]);
        a.phone = Utf8ToWide(cols[4]);
        a.status = Utf8ToWide(cols[5]);
        a.priority = Utf8ToWide(cols[6]);
        a.revenueOpportunity = Utf8ToWide(cols[7]);
        a.lastFollowUp = Utf8ToWide(cols[8]);
        a.nextFollowUp = Utf8ToWide(cols[9]);
        a.owner = Utf8ToWide(cols[10]);
        a.notes = Utf8ToWide(cols[11]);

        g_accounts.push_back(a);
    }
}

void AddComboItems(HWND combo, const wchar_t* const* items, int count, const wchar_t* defaultValue) {
    for (int i = 0; i < count; ++i) {
        SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)items[i]);
    }
    SetWindowTextW(combo, defaultValue);
}

HWND CreateLabel(HWND parent, const wchar_t* text, int x, int y, int w, int h) {
    return CreateWindowW(
        L"STATIC",
        text,
        WS_CHILD | WS_VISIBLE,
        x, y, w, h,
        parent,
        NULL,
        g_hInst,
        NULL
    );
}

HWND CreateEdit(HWND parent, int x, int y, int w, int h, bool multiline = false) {
    DWORD style = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL;
    if (multiline) {
        style = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL;
    }

    return CreateWindowW(
        L"EDIT",
        L"",
        style,
        x, y, w, h,
        parent,
        NULL,
        g_hInst,
        NULL
    );
}

HWND CreateButton(HWND parent, const wchar_t* text, int id, int x, int y, int w, int h) {
    return CreateWindowW(
        L"BUTTON",
        text,
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x, y, w, h,
        parent,
        (HMENU)id,
        g_hInst,
        NULL
    );
}

HWND CreateCombo(HWND parent, int x, int y, int w, int h) {
    return CreateWindowW(
        L"COMBOBOX",
        L"",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        x, y, w, h,
        parent,
        NULL,
        g_hInst,
        NULL
    );
}

void SetFontForWindow(HWND hwnd, HFONT font) {
    SendMessageW(hwnd, WM_SETFONT, (WPARAM)font, TRUE);
}

Account GetFormAccount() {
    Account a;
    a.accountName = Trim(GetWindowTextString(eAccountName));
    a.venueType = Trim(GetWindowTextString(eVenueType));
    a.contactPerson = Trim(GetWindowTextString(eContactPerson));
    a.email = Trim(GetWindowTextString(eEmail));
    a.phone = Trim(GetWindowTextString(ePhone));
    a.status = Trim(GetWindowTextString(cStatus));
    a.priority = Trim(GetWindowTextString(cPriority));
    a.revenueOpportunity = Trim(GetWindowTextString(eRevenueOpportunity));
    a.lastFollowUp = Trim(GetWindowTextString(eLastFollowUp));
    a.nextFollowUp = Trim(GetWindowTextString(eNextFollowUp));
    a.owner = Trim(GetWindowTextString(eOwner));
    a.notes = Trim(GetWindowTextString(eNotes));
    return a;
}

void SetFormAccount(const Account& a) {
    SetWindowTextSafe(eAccountName, a.accountName);
    SetWindowTextSafe(eVenueType, a.venueType);
    SetWindowTextSafe(eContactPerson, a.contactPerson);
    SetWindowTextSafe(eEmail, a.email);
    SetWindowTextSafe(ePhone, a.phone);
    SetWindowTextSafe(cStatus, a.status.empty() ? L"New Lead" : a.status);
    SetWindowTextSafe(cPriority, a.priority.empty() ? L"Medium" : a.priority);
    SetWindowTextSafe(eRevenueOpportunity, a.revenueOpportunity);
    SetWindowTextSafe(eLastFollowUp, a.lastFollowUp);
    SetWindowTextSafe(eNextFollowUp, a.nextFollowUp);
    SetWindowTextSafe(eOwner, a.owner);
    SetWindowTextSafe(eNotes, a.notes);
}

void ClearForm() {
    g_selectedIndex = -1;
    SetWindowTextSafe(eAccountName, L"");
    SetWindowTextSafe(eVenueType, L"");
    SetWindowTextSafe(eContactPerson, L"");
    SetWindowTextSafe(eEmail, L"");
    SetWindowTextSafe(ePhone, L"");
    SetWindowTextSafe(cStatus, L"New Lead");
    SetWindowTextSafe(cPriority, L"Medium");
    SetWindowTextSafe(eRevenueOpportunity, L"");
    SetWindowTextSafe(eLastFollowUp, L"");
    SetWindowTextSafe(eNextFollowUp, L"");
    SetWindowTextSafe(eOwner, L"Andrii");
    SetWindowTextSafe(eNotes, L"");
    ListView_SetItemState(g_listView, -1, 0, LVIS_SELECTED);
}

wstring AccountSearchText(const Account& a) {
    return ToLower(
        a.accountName + L" " +
        a.venueType + L" " +
        a.contactPerson + L" " +
        a.email + L" " +
        a.phone + L" " +
        a.status + L" " +
        a.priority + L" " +
        a.revenueOpportunity + L" " +
        a.lastFollowUp + L" " +
        a.nextFollowUp + L" " +
        a.owner + L" " +
        a.notes
    );
}

bool IsDue(const Account& a) {
    if (a.nextFollowUp.empty()) return false;
    return a.nextFollowUp <= TodayDate();
}

void AddListColumn(const wchar_t* title, int index, int width) {
    LVCOLUMNW col = {};
    col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    col.pszText = (LPWSTR)title;
    col.cx = width;
    col.iSubItem = index;
    ListView_InsertColumn(g_listView, index, &col);
}

void AddListItem(int displayRow, int originalIndex, const Account& a) {
    LVITEMW item = {};
    item.mask = LVIF_TEXT | LVIF_PARAM;
    item.iItem = displayRow;
    item.iSubItem = 0;
    item.pszText = (LPWSTR)a.accountName.c_str();
    item.lParam = originalIndex;

    ListView_InsertItem(g_listView, &item);
    ListView_SetItemText(g_listView, displayRow, 1, (LPWSTR)a.venueType.c_str());
    ListView_SetItemText(g_listView, displayRow, 2, (LPWSTR)a.contactPerson.c_str());
    ListView_SetItemText(g_listView, displayRow, 3, (LPWSTR)a.status.c_str());
    ListView_SetItemText(g_listView, displayRow, 4, (LPWSTR)a.priority.c_str());
    ListView_SetItemText(g_listView, displayRow, 5, (LPWSTR)a.revenueOpportunity.c_str());
    ListView_SetItemText(g_listView, displayRow, 6, (LPWSTR)a.lastFollowUp.c_str());
    ListView_SetItemText(g_listView, displayRow, 7, (LPWSTR)a.nextFollowUp.c_str());
    ListView_SetItemText(g_listView, displayRow, 8, (LPWSTR)a.owner.c_str());
}

void UpdateSummary() {
    int total = (int)g_accounts.size();
    int high = 0;
    int active = 0;
    int follow = 0;
    int due = 0;

    for (const auto& a : g_accounts) {
        if (a.priority == L"High" || a.priority == L"Urgent") high++;
        if (a.status == L"Active Campaign") active++;
        if (a.status == L"Follow-Up Needed") follow++;
        if (IsDue(a)) due++;
    }

    wchar_t buffer[512];
    swprintf_s(
        buffer,
        L"Total accounts: %d    |    High/Urgent: %d    |    Active Campaigns: %d    |    Follow-Up Needed: %d    |    Due Today/Earlier: %d    |    Data: %s",
        total,
        high,
        active,
        follow,
        due,
        GetCsvPath().c_str()
    );

    SetWindowTextW(g_summaryLabel, buffer);
}

void RefreshList() {
    ListView_DeleteAllItems(g_listView);

    wstring search = ToLower(Trim(GetWindowTextString(g_searchBox)));
    int displayRow = 0;

    for (int i = 0; i < (int)g_accounts.size(); ++i) {
        const Account& a = g_accounts[i];

        if (g_showDueOnly && !IsDue(a)) continue;

        if (!search.empty()) {
            if (AccountSearchText(a).find(search) == wstring::npos) continue;
        }

        AddListItem(displayRow, i, a);
        displayRow++;
    }

    UpdateSummary();
}

void AddAccount() {
    Account a = GetFormAccount();

    if (a.accountName.empty()) {
        MessageBoxW(g_mainWindow, L"Account Name is required.", L"Missing Information", MB_ICONWARNING);
        return;
    }

    if (a.status.empty()) a.status = L"New Lead";
    if (a.priority.empty()) a.priority = L"Medium";
    if (a.owner.empty()) a.owner = L"Andrii";

    g_accounts.push_back(a);
    SaveAccounts();
    ClearForm();
    RefreshList();

    MessageBoxW(g_mainWindow, L"Account added successfully.", L"Saved", MB_OK | MB_ICONINFORMATION);
}

void UpdateAccount() {
    if (g_selectedIndex < 0 || g_selectedIndex >= (int)g_accounts.size()) {
        MessageBoxW(g_mainWindow, L"Select an account first.", L"No Selection", MB_ICONWARNING);
        return;
    }

    Account a = GetFormAccount();

    if (a.accountName.empty()) {
        MessageBoxW(g_mainWindow, L"Account Name is required.", L"Missing Information", MB_ICONWARNING);
        return;
    }

    g_accounts[g_selectedIndex] = a;
    SaveAccounts();
    RefreshList();

    MessageBoxW(g_mainWindow, L"Account updated successfully.", L"Updated", MB_OK | MB_ICONINFORMATION);
}

void DeleteAccount() {
    if (g_selectedIndex < 0 || g_selectedIndex >= (int)g_accounts.size()) {
        MessageBoxW(g_mainWindow, L"Select an account first.", L"No Selection", MB_ICONWARNING);
        return;
    }

    int result = MessageBoxW(
        g_mainWindow,
        L"Are you sure you want to delete this account?",
        L"Confirm Delete",
        MB_YESNO | MB_ICONQUESTION
    );

    if (result == IDYES) {
        g_accounts.erase(g_accounts.begin() + g_selectedIndex);
        g_selectedIndex = -1;
        SaveAccounts();
        ClearForm();
        RefreshList();
    }
}

void SelectListItem(LPARAM lParam) {
    int originalIndex = (int)lParam;
    if (originalIndex >= 0 && originalIndex < (int)g_accounts.size()) {
        g_selectedIndex = originalIndex;
        SetFormAccount(g_accounts[originalIndex]);
    }
}

void CreateMainControls(HWND hwnd) {
    HFONT titleFont = CreateFontW(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                  DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

    HFONT normalFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                   OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                   DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

    HWND title = CreateLabel(hwnd, L"Taiv Account Tracker", 20, 15, 500, 30);
    SetFontForWindow(title, titleFont);

    HWND sub = CreateLabel(hwnd, L"Client / venue account tracking, CRM hygiene, follow-ups, campaign notes, and sales pipeline support.", 20, 45, 900, 25);
    SetFontForWindow(sub, normalFont);

    int labelX = 20;
    int inputX = 155;
    int y = 90;
    int labelW = 125;
    int inputW = 245;
    int rowH = 24;
    int gap = 35;

    const wchar_t* labels[] = {
        L"Account Name",
        L"Venue Type",
        L"Contact Person",
        L"Email",
        L"Phone",
        L"Status",
        L"Priority",
        L"Revenue Opp.",
        L"Last Follow-Up",
        L"Next Follow-Up",
        L"Owner",
        L"Campaign Notes"
    };

    for (int i = 0; i < 12; ++i) {
        CreateLabel(hwnd, labels[i], labelX, y + i * gap, labelW, rowH);
    }

    eAccountName = CreateEdit(hwnd, inputX, y + 0 * gap, inputW, rowH);
    eVenueType = CreateEdit(hwnd, inputX, y + 1 * gap, inputW, rowH);
    eContactPerson = CreateEdit(hwnd, inputX, y + 2 * gap, inputW, rowH);
    eEmail = CreateEdit(hwnd, inputX, y + 3 * gap, inputW, rowH);
    ePhone = CreateEdit(hwnd, inputX, y + 4 * gap, inputW, rowH);
    cStatus = CreateCombo(hwnd, inputX, y + 5 * gap, inputW, 200);
    cPriority = CreateCombo(hwnd, inputX, y + 6 * gap, inputW, 160);
    eRevenueOpportunity = CreateEdit(hwnd, inputX, y + 7 * gap, inputW, rowH);
    eLastFollowUp = CreateEdit(hwnd, inputX, y + 8 * gap, inputW, rowH);
    eNextFollowUp = CreateEdit(hwnd, inputX, y + 9 * gap, inputW, rowH);
    eOwner = CreateEdit(hwnd, inputX, y + 10 * gap, inputW, rowH);
    eNotes = CreateEdit(hwnd, inputX, y + 11 * gap, inputW, 95, true);

    AddComboItems(cStatus, STATUS_OPTIONS, 9, L"New Lead");
    AddComboItems(cPriority, PRIORITY_OPTIONS, 4, L"Medium");
    SetWindowTextSafe(eOwner, L"Andrii");

    int buttonY = 610;
    CreateButton(hwnd, L"Add Account", ID_ADD, 20, buttonY, 120, 30);
    CreateButton(hwnd, L"Update", ID_UPDATE, 150, buttonY, 120, 30);
    CreateButton(hwnd, L"Delete", ID_DELETE, 280, buttonY, 120, 30);
    CreateButton(hwnd, L"Clear Form", ID_CLEAR, 20, buttonY + 40, 120, 30);
    CreateButton(hwnd, L"Today Follow-Up", ID_TODAY, 150, buttonY + 40, 250, 30);

    CreateLabel(hwnd, L"Search:", 430, 85, 60, 24);
    g_searchBox = CreateEdit(hwnd, 490, 82, 320, 26);
    CreateButton(hwnd, L"Due Follow-Ups", ID_SHOW_DUE, 825, 80, 140, 30);
    CreateButton(hwnd, L"Show All", ID_SHOW_ALL, 975, 80, 100, 30);

    g_listView = CreateWindowW(
        WC_LISTVIEWW,
        L"",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | WS_BORDER,
        430, 120, 830, 520,
        hwnd,
        (HMENU)ID_LIST,
        g_hInst,
        NULL
    );

    ListView_SetExtendedListViewStyle(g_listView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);

    AddListColumn(L"Account", 0, 145);
    AddListColumn(L"Venue Type", 1, 110);
    AddListColumn(L"Contact", 2, 125);
    AddListColumn(L"Status", 3, 130);
    AddListColumn(L"Priority", 4, 80);
    AddListColumn(L"Revenue", 5, 90);
    AddListColumn(L"Last Follow-Up", 6, 105);
    AddListColumn(L"Next Follow-Up", 7, 105);
    AddListColumn(L"Owner", 8, 80);

    g_summaryLabel = CreateLabel(hwnd, L"", 430, 655, 830, 60);

    for (HWND h : {
        eAccountName, eVenueType, eContactPerson, eEmail, ePhone, cStatus, cPriority,
        eRevenueOpportunity, eLastFollowUp, eNextFollowUp, eOwner, eNotes, g_searchBox,
        g_listView, g_summaryLabel
    }) {
        SetFontForWindow(h, normalFont);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        CreateMainControls(hwnd);
        LoadAccounts();
        RefreshList();
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_ADD:
            AddAccount();
            break;

        case ID_UPDATE:
            UpdateAccount();
            break;

        case ID_DELETE:
            DeleteAccount();
            break;

        case ID_CLEAR:
            ClearForm();
            break;

        case ID_TODAY:
            SetWindowTextSafe(eLastFollowUp, TodayDate());
            break;

        case ID_SHOW_DUE:
            g_showDueOnly = true;
            RefreshList();
            break;

        case ID_SHOW_ALL:
            g_showDueOnly = false;
            SetWindowTextSafe(g_searchBox, L"");
            RefreshList();
            break;

        case ID_SEARCH:
            break;

        default:
            if ((HWND)lParam == g_searchBox && HIWORD(wParam) == EN_CHANGE) {
                RefreshList();
            }
            break;
        }
        return 0;

    case WM_NOTIFY: {
        LPNMHDR hdr = (LPNMHDR)lParam;

        if (hdr->hwndFrom == g_listView && hdr->code == LVN_ITEMCHANGED) {
            LPNMLISTVIEW p = (LPNMLISTVIEW)lParam;

            if ((p->uNewState & LVIS_SELECTED) && p->iItem >= 0) {
                LVITEMW item = {};
                item.mask = LVIF_PARAM;
                item.iItem = p->iItem;
                ListView_GetItem(g_listView, &item);
                SelectListItem(item.lParam);
            }
        }
        return 0;
    }

    case WM_SIZE: {
        RECT rc;
        GetClientRect(hwnd, &rc);
        int width = rc.right - rc.left;
        int height = rc.bottom - rc.top;

        if (g_listView) {
            MoveWindow(g_listView, 430, 120, max(500, width - 455), max(360, height - 220), TRUE);
        }

        if (g_summaryLabel) {
            MoveWindow(g_summaryLabel, 430, height - 70, max(500, width - 455), 60, TRUE);
        }

        return 0;
    }

    case WM_DESTROY:
        SaveAccounts();
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    g_hInst = hInstance;

    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icc);

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"TaivAccountTrackerWindowClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassW(&wc);

    g_mainWindow = CreateWindowExW(
        0,
        wc.lpszClassName,
        L"Taiv Account Tracker - Andrii Pavlenko",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1300,
        760,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!g_mainWindow) return 0;

    ShowWindow(g_mainWindow, nCmdShow);
    UpdateWindow(g_mainWindow);

    MSG msg = {};
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return 0;
}
