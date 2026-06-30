POLISHED DROPDOWN VERSION
- Native white browser dropdowns replaced with custom dark dropdown menus.
- Improved table spacing, hover states, chips, scrollbar styling and date picker visuals.

o

WHY THIS VERSION EXISTS
The previous C++ Win32 version works, but native Win32 controls look old and harsh.
This version is designed to look like a polished startup dashboard:
- smoother dark SaaS design
- Taiv-inspired purple/blue accent
- softer cards and rounded layout
- clean table
- real date picker
- status chips
- KPI cards
- follow-up queue
- export CSV
- local data saving

HOW TO RUN
Option 1:
Double-click index.html

Option 2:
Right-click index.html -> Open with -> Chrome / Edge

HOW TO MAKE A DESKTOP SHORTCUT
1. Run create_desktop_shortcut.ps1 with PowerShell.
2. It will create a shortcut called "Taiv Account Tracker Studio" on your Desktop.

If PowerShell blocks the script, run:
powershell -ExecutionPolicy Bypass -File create_desktop_shortcut.ps1

DATA STORAGE
The app saves data in the browser's localStorage.
That means data stays on the same browser/computer unless you reset demo data or clear browser storage.



FEATURES
- Create/edit/delete account
- Dashboard KPI cards
- Follow-up due queue
- Account table with filters
- Date picker for follow-up dates
- Status and priority chips
- Export current view to CSV
- Playbook view explaining the workflow
