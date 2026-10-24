# Plasma Script Debugger

Attaches VS Code to the Plasma editor's script debug adapter (127.0.0.1:4712, or the editor's `-scriptdebugport`).
Open the project folder, start "Attach to Plasma Editor" (F5), then play the scene in the editor. Breakpoints set
here and in the editor are the same set; a stop shows the call stack and variables in both.

The editor installs this extension and the project's launch configuration when it opens VS Code for an AngelScript
script.
