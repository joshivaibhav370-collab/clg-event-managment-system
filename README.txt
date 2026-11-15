CampusConnect - College Event Management (C++ backend + HTML frontend)

HOW TO RUN (Windows, needs MinGW g++):
  1. Double-click build.bat   -> creates CampusConnect.exe
  2. Double-click run.bat     -> starts server and opens http://localhost:8080
  Do NOT open the HTML files directly (file://) - they must come from the server.

HOW IT WORKS:
  Browser page --fetch("/api/...")--> main.cpp (tiny web server)
             --> CampusConnectSystem (C++ classes) --> JSON back to page

API:
  /api/events  /api/students  /api/registrations  /api/waiting  /api/stats
  /api/addEvent?id=&name=&date=&venue=&capacity=
  /api/addStudent?id=&name=&dept=&semester=
  /api/register?student=&event=
  /api/cancel?student=&event=
  /api/undo

DATA STRUCTURES:
  Linked list  -> confirmed registrations (RegistrationList)
  Queue (FIFO) -> waiting list (WaitingQueue)
  Stack (LIFO) -> undo history (ActionStack)
  Inheritance  -> Person -> Student

Connected to C++: Dashboard, Events, Students, Registration.
Still frontend-only demo data: Schedule, Venues, Volunteers, Attendance, Results, Reports.
Data is in memory: it resets when the server is closed.
