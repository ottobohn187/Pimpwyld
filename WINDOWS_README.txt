PIMP WYLD 3.0 - BIG TEN EDITION
Windows 10/11, 64-bit

HOW TO RUN
1. Extract this entire ZIP to a folder.
2. Double-click RUN_PIMP_WYLD.bat.

You can also run PimpWyld30-Windows-x64.exe directly. The batch file merely
keeps the console window open when the game finishes.

This is a new, native Windows console build. It does not require DOSBox or the
original DOS executable. It accepts an optional deterministic random seed from
Command Prompt:

    PimpWyld30-Windows-x64.exe --seed 1

BUILD STATUS
- Cross-compiled from portable C for 64-bit Windows using LLVM/MinGW (UCRT).
- Recreates the original colored 80-column dashboard, boxed conditions/stats/
  girls panels, command rows, immediate single-key commands, and lettered
  drive/campus screens. For the intended layout, keep the console at least
  110 columns wide and 42 rows tall; the included launcher sets that size
  automatically so the expanded ANSI scenes remain visible.
- Dashboard output is capped at 78 printable columns to prevent Windows
  Terminal from wrapping the right border onto following lines.
- The executable is unsigned, so Windows SmartScreen may identify it as an
  unrecognized app. Scan it and run it only if you trust this build.
- Static checks and the same source's automated self-test passed. The Windows
  executable was inspected as a valid x86-64 console PE file, but could not be
  launched in the Linux build environment because Wine was unavailable.

FIDELITY NOTE
This is a clean-room playable reconstruction based on analysis and live runs
of PWYLD20.EXE. It is not the original Borland source and does not yet reproduce
every random formula, event, or screen transition exactly. Its primary menus
are now modeled directly on captures of the live DOS executable.

GAMEPLAY REPAIRS
- Drug purchases A-J, including Crystal, are covered by automated tests.
- Buy and sell actions show a receipt or a clear reason for failure.
- Riot panic markets, random attacks, gun deals, mugging, and trenchcoat deals
  can occur while traveling. The recovered EXE and its briefing file directly
  support these event types; exact probabilities remain reconstructed.

VERSION 3.0
- The campaign now covers all 18 Big Ten universities and lasts 60 days.
- Every command displays its key: (B)uy, (S)ell, (D)rive, (R)iot, and so on.
- Campus travel costs $5-$25 using approximate campus coordinates and
  great-circle distance; the drive screen shows the fare before travel.
- Campus security affects prices and combat difficulty.
- Riot actions build unrest and lead to forward-facing ANSI/JRPG combat.
- YOU, ACE, and NYX each take a turn and have separate HP bars. Enforcers face
  the player with weapons; if YOU reach zero HP, the riot is lost.
- Attack, Gun, Defend, Rally, and Escape create distinct combat choices.
- More riot actions increase the chance of random fights while traveling.
- A lay requires and consumes a condom and contributes to the Impact Score.
- All companion characters and relationship encounters are explicitly adults.
- At the end, the Impact Report scores anarchy, riots, lays, status, fights,
  surviving health, and net worth. Campuses in anarchy are worth the most.
- The opening screen and random trenchcoat encounter now have full ANSI art.
- Press (T)est on the dashboard for a nine-scene Event Test gallery. It shows
  every special-event presentation without changing cash, inventory, health,
  days, campuses, or the random event sequence.
- The dashboard retains the original GIRLS panel label; all relationship
  characters in Version 3.0 are adults.

The readable source and reverse-engineering notes are included under source\.
