# Leafpad
GTK+ based simple text editor

## Description

Leafpad is a simple GTK+ text editor that emphasizes simplicity. As development
focuses on keeping weight down to a minimum, only the most essential features
are implemented in the editor. Leafpad is simple to use, is easily compiled,
requires few libraries, and starts up quickly.

This program is released under the GNU General Public License (GPL) version 2,
see the file `COPYING` for more information.


## Features

  * Codeset option (Some OpenI18N registered)
  * Auto codeset detection (UTF-8 and some codesets)
  * Unlimitted Undo/Redo
  * Auto/Multi-line Indent
  * Display line numbers
  * Drag and Drop
  * Printing


## Installation
Leafpad requires GTK+-2.x.x libraries.
Printing feature requires libgnomeprintui-2.2 optionally.

Simple install procedure:
```
 $ tar xzvf leafpad-x.x.x.tar.gz       # unpack the sources
 $ cd leafpad-x.x.x                    # change to the toplevel directory
 $ ./configure                         # run the `configure' script
 $ make                                # build Leafpad
 [ Become root if necessary ]
 # make install-strip                  # install Leafpad
```
See the file `INSTALL` for more detailed information.


## Key bindings

|Key|Action|
|:-|:-|
| <kbd>Ctrl</kbd> + <kbd>N</kbd> | New | 
| <kbd>Ctrl</kbd> + <kbd>O</kbd> | Open | 
| <kbd>Ctrl</kbd> + <kbd>S</kbd> | Save | 
| <kbd>Shift</kbd> + <kbd>Ctrl</kbd> + <kbd>S</kbd> |  Save As | 
| <kbd>Ctrl</kbd> + <kbd>W</kbd> | Close | 
| <kbd>Ctrl</kbd> + <kbd>P</kbd> | Print | 
| <kbd>Ctrl</kbd> + <kbd>Q</kbd> | Quit | 
| <kbd>Ctrl</kbd> + <kbd>Z</kbd> | Undo | 
| <kbd>Shift</kbd> + <kbd>Ctrl</kbd> + <kbd>Z</kbd>  <br> <kbd>Ctrl</kbd> + <kbd>Y</kbd> |  Redo | 
| <kbd>Ctrl</kbd> + <kbd>X</kbd> | Cut | 
| <kbd>Ctrl</kbd> + <kbd>C</kbd> | Copy | 
| <kbd>Ctrl</kbd> + <kbd>V</kbd> | Paste | 
| <kbd>Ctrl</kbd> + <kbd>A</kbd> | Select All | 
| <kbd>Ctrl</kbd> + <kbd>F</kbd> | Find | 
| <kbd>Ctrl</kbd> + <kbd>G</kbd> <br> <kbd>F3</kbd> | Find Next | 
|  <kbd>Shift</kbd> + <kbd>Ctrl</kbd> + <kbd>G</kbd> <br>  <kbd>Shift</kbd> + <kbd>F3</kbd> |Find Previous | 
| <kbd>Ctrl</kbd> + <kbd>H</kbd> <br> <kbd>Ctrl</kbd> + <kbd>R</kbd> | Replace | 
| <kbd>Ctrl</kbd> + <kbd>J</kbd> | Jump To | 
| <kbd>Ctrl</kbd> + <kbd>T</kbd> | Always on Top | 
| <kbd>Ctrl</kbd> + <kbd>Tab</kbd> | Toggle tab width | 
| <kbd>Tab</kbd> with selection bound <br> <kbd>Ctrl</kbd> + <kbd>]</kbd> | Multi-line indent |
| <kbd>Shift</kbd> + <kbd>Tab</kbd> with selection bound <br> <kbd>Ctrl</kbd> + <kbd>[</kbd>   | Multi-line unindent | 
