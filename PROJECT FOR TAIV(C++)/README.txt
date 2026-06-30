
TAIV ACCOUNT TRACKER - C++ DESKTOP APPLICATION
Created for Andrii Pavlenko

PROJECT PURPOSE
This is a C++ desktop app project that demonstrates account coordination, CRM hygiene,
client follow-up tracking, campaign notes, and sales/account workflow thinking.

It is designed as a small practical project you can show to Taiv.

WHAT THE APP DOES
The app tracks:
- Client / venue account name
- Venue type
- Contact person
- Email
- Phone
- Account status
- Priority
- Revenue opportunity
- Last follow-up
- Next follow-up
- Owner
- Campaign notes

FEATURES
- Add account
- Update selected account
- Delete selected account
- Clear form
- One-click "Today Follow-Up"
- Search across all account fields
- Show follow-ups due today or earlier
- Save data automatically to CSV
- Loads sample demo data on first launch
- Works like a normal Windows desktop app after it is built

WHERE DATA IS SAVED
The app saves data here:

C:\Users\<YourUser>\AppData\Roaming\TaivAccountTracker\accounts.csv

This means the app can be opened from a desktop shortcut and still keep the same data.

HOW TO BUILD WITH VISUAL STUDIO
1. Install Visual Studio Community or Visual Studio Build Tools.
2. Open "x64 Native Tools Command Prompt for VS".
3. Go to this project folder.
4. Run:

build_msvc.bat

5. After build finishes, run:

create_desktop_shortcut.ps1

If PowerShell blocks the script, run this inside PowerShell:

Set-ExecutionPolicy -Scope CurrentUser RemoteSigned

Then run:

.\create_desktop_shortcut.ps1

HOW TO BUILD WITH MINGW
1. Install MinGW-w64.
2. Make sure g++ is in PATH.
3. Open Command Prompt in this folder.
4. Run:

build_mingw.bat

5. Then run create_desktop_shortcut.ps1.

HOW TO PRESENT THIS TO TAIV
You can say:

"I built a small C++ desktop Account Tracker as a practical demo for Taiv.
It tracks venues, contacts, campaign status, revenue opportunity, follow-up dates,
priority, and account notes. The goal is to show how I think about account coordination,
CRM hygiene, follow-up discipline, and reducing missed client actions."

GOOD INTERVIEW LINE
"This is not meant to replace Taiv's real CRM. I built it to demonstrate how I think:
clean account data, clear next steps, follow-up dates, owner visibility, and simple workflow
tools that help sales and account teams move faster."
