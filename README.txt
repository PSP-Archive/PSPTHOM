
    Welcome to PSPTHOM

Original Authors of THOM

  Sylvain Huet    (see http://www.sylvain-huet.com)
  Eric Botcazou   (see http://nostalgies.thomsonistes.org/)

Author of the PSP port version 

  Ludovic.Jacomme also known as Zx-81 (see http://zx81.zx81.free.fr)


1. INTRODUCTION
   ------------

  Thom is one of the best emulator of the Thomson home computer running
  on MacOS, Windows and Unix. The emulator faithfully imitates the
  TO7-70 model (see http://nostalgies.thomsonistes.org/)

  PSPTHOM is a port on PSP of the version 1.5.5 of Thom.

  Special thanks to Danzel and Jeff Chen for their virtual keyboard,
  and to all PSPSDK developpers.

  Special thanks to Gelon & Rawen for their eboot's icons.

  Thanks to XazZ for his comments, feedback and suggestions.

  Special thanks to NeXuS for his nice music 

  (see http://www.jamendo.com/en/artist/nexus for more music)

  This package is under GPL Copyright, read COPYING file for
  more information about it.


2. INSTALLATION
   ------------

  Unzip the zip file, and copy the content of the directory fw5.x
  on the directory psp/game5xx (depending on your custom firmware version).

  It has been developped on linux for Firmware 5.0-M33

  For any comments or questions on this version, please visit 
  http://zx81.zx81.free.fr or http://zx81.dcemu.co.uk


3. CONTROL
   ------------

3.1 - Virtual keyboard

  In the THOM emulator window, there are three different mapping (standard,
  left trigger, and right Trigger mappings).  You can toggle between while
  playing inside the emulator using the two PSP trigger keys.

  -------------------------------------
  PSP        TO7-70          (standard)

  Square     Space
  Triangle   ENTER
  Circle     1
  Cross      Joystick Fire
  Up         Up
  Down       Down
  Left       Left 
  Right      Right

  Analog     Joystick

  -------------------------------------
  PSP        TO7-70   (left trigger)
    
  Square     FPS  
  Triangle   LOAD Snapshot
  Circle     Swap digital / Analog
  Cross      SAVE Snapshot
  Up         Inc delta Y
  Down       Dec delta Y
  Left       Render mode
  Right      Render mode
  
  -------------------------------------
  PSP        TO7-70   (right trigger)
    
  Square     Stop 
  Triangle   Return
  Circle     1
  Cross      Auto-fire
  Up         Up
  Down       Down
  Left       Dec Fire   
  Right      Inc Fire   
  
  Analog     Joystick
      
  Press Start+L+R to exit and return to eloader.
  Press Select    to enter in emulator main menu.
  Press Start     open/close the On-Screen keyboard
  
  In the main menu
  
  RTrigger   Reset the emulator
  
  Triangle   Go Up directory
  Cross      Valid
  Circle     Valid
  Square     Go Back to the emulator window
  
  The On-Screen Keyboard of "Danzel" and "Jeff Chen"
  
  Use Analog stick to choose one of the 9 squares, and use Triangle, Square,
  Cross and Circle to choose one of the 4 letters of the highlighted square.
  
  Use LTrigger and RTrigger to see other 9 squares figures.


3.2 - IR keyboard

  You can also use IR keyboard. Edit the pspirkeyb.ini file
  to specify your IR keyboard model, and modify eventually
  layout keyboard files in the keymap directory.

  The following mapping is done :

  IR-keyboard   PSP

  Cursor        Digital Pad

  Tab           Start
  Ctrl-W        Start

  Escape        Select
  Ctrl-Q        Select

  Ctrl-E        Triangle
  Ctrl-X        Cross
  Ctrl-S        Square
  Ctrl-F        Circle
  Ctrl-Z        L-trigger
  Ctrl-C        R-trigger

  In the emulator window you can use the IR keyboard to
  enter letters, special characters and digits.


4. LOADING TAPE FILES (K7)
   ------------

  If you want to load tape image in the virtual drive of your emulator,
  you have to put your tape file (with .zip or .k7 file extension) on your PSP
  memory stick in the 'k7' directory. 

  Then, while inside THOM emulator, just press SELECT to enter in the emulator 
  main menu, choose "Load K7", and then using the file selector choose one tape 
  file to load in your emulator.

  You can use the virtual keyboard in the file requester menu to choose the
  first letter of the game you search (it might be useful when you have tons of
  games in the same folder). Entering several time the same letter let you
  choose sequentially files beginning with the given letter. You can use the
  space key of the virtual keyboard to load the tape file.

  To run the game of your tape file, you have to use the virtual keyboard
  (press START key) and type the TO7-70 command 'LOADM"",,R' followed by 
  ENTER (Triangle).

  You can also use directly the shortcut in the emulator menu (Command LOADM option)

  To RUN BASIC program, you have to type the TO7-70 command 'RUN""' followed
  by ENTER (Triangle)

  You can also use directly the shortcut in the emulator menu (Command RUN option)

  It may happen that you need to rewind the tape using the Rewind K7 menu ...

  The command 'LOAD""' (without M) is sometimes needed, when LOADM command
  doesn't work.

  If you use the Basic rom 1.0 128, then you will have to put the string
  "CASS:" in your 'LOADM', 'LOAD' or 'RUN' command as follows :

    LOADM"CASS:",,R 
    LOAD"CASS:",,R
    RUN"CASS:"


5. LOADING DISK FILES (SAP)
   ------------

  If you want to load tape image in the virtual disk drive of your emulator,
  you have to put your disk file (with .sap file extension) on your PSP
  memory stick in the 'disc' directory. 

  Then, while inside THOM emulator, just press SELECT to enter in the emulator 
  main menu, choose "Load Disc 0" (or Disc 1), and then using the file 
  selector choose one disc file to load in your emulator.

  Use the same commands as described in the "Load tape" section.

  If you want to specify the command to run for given games then
  you can do it in the run.txt, using the following syntax :

    tapename=LOAD"CASS:RunName",,R
    tapename=LOADM"CASS:RunName",,R
    tapename=RUN"CASS:RunName"
    diskname=LOAD"RunName",,R
    diskname=LOADM"RunName",,R
    diskname=RUN"RunName"

6. LOADING SNAPSHOT FILES (STZ)
   ------------

  I've modified original THOM emulator to add a save state feature. The save
  state format is specific to PSPTHOM, but it might be useful to run previously
  loaded games (using K7 and disk menu).

   
7. LOADING KEY MAPPING FILES
   ------------

  For given games, the default keyboard mapping between PSP Keys and TO7-70 keys,
  is not suitable, and the game can't be played on PSPTHOM.

  To overcome the issue, you can write your own mapping file. Using notepad for
  example you can edit a file with the .kbd extension and put it in the kbd 
  directory.

  For the exact syntax of those mapping files, have a look on sample files already
  presents in the kbd directory (default.kbd etc ...).

  After writting such keyboard mapping file, you can load them using 
  the main menu inside the emulator.

  If the keyboard filename is the same as the tape or disk file (.k7 or .sap)
  then when you load this tape file, the corresponding keyboard file is 
  automatically loaded !

  You can now use the Keyboard menu and edit, load and save your
  keyboard mapping files inside the emulator. The Save option save the .kbd
  file in the kbd directory using the "Game Name" as filename. The game name
  is displayed on the right corner in the emulator menu.

8. CHEAT CODE (.CHT)
----------
  
  You can use cheat codes with PSP-Thom.  You can add your own cheat codes in
  the cheat.txt file and then import them in the cheat menu.  All cheat codes
  you have specified for a game can be save in a CHT file in 'cht' folder.
  Those cheat codes would then be automatically loaded when you start the game.
  
  The CHT file format is the following :
  #
  # Enable, Address, Value, Comment
  #
  1,36f,3,Cheat comment
  
  Using the Cheat menu you can search for modified bytes in RAM between current
  time and the last time you saved the RAM. It might be very usefull to find
  "poke" address by yourself, monitoring for example life numbers.
  
  To find a new "poke address" you can proceed as follow :
  
  Let's say you're playing Pulsar and you want to find the memory address where
  "number lives" is stored.
  
  . Start a new game in Pulsar
  . Enter in the cheat menu. 
  . Choose Save Ram to save initial state of the memory. 
  . Specify the number of lives you want to find in
    "Scan Old Value" field.
    (for Pulsar the initial lives number is 5)
  . Go back to the game and loose a life.
  . Enter in the cheat menu. 
  . Specify the number of lives you want to find in
    "Scan New Value" field.
    (for Pulsar the lives number is now 4)
  . In Add Cheat you have now one matching Address
    (for Pulsar it's 6C51)
  . Specify the Poke value you want (for example 5) 
    and add a new cheat with this address / value.
  
  The cheat is now activated in the cheat list and you can save it using the
  "Save cheat" menu.
  
  Let's enjoy Pulsar with infinite life !!

9. COMMENTS
------------

  You can write your own comments for games using the "Comment" menu.  The first
  line of your comments would then be displayed in the file requester menu while
  selecting the given file name (Tape, Disk, keyboard, settings).


10. SETTINGS
------------

You can modify several settings value in the settings menu of this emulator.
The following parameters are available :

  Sound enable : 
    enable or disable the sound

  Active Joystick : 
    Joystick player, it could be 1 or 2

  Speed limiter :
    limit the speed to a given fps value

  Skip frame : 
    to skip frame and increase emulator speed

  Display fps : 
    display real time fps value 

  Render mode : 
    many render modes are available with different
    geometry that should covered all games
    requirements

  Delta Y : 
    move the center of the screen vertically

  Vsync : 
    wait for vertical signal between each frame displayed

  Swap Analog/Cursor : 
    swap key mapping between PSP analog pad and PSP
    digital pad

  Auto fire period : 
    auto fire period

  Auto fire mode : 
    auto fire mode active or not

  Display LR led : 
    draw a small blue rectangle in top of the screen
    when trigger keys are pressed

  Clock frequency : 
    PSP clock frequency, by default the value is set
    to 222Mhz, and should be enough for most of all
    games.
    But if you encounter graphical glitches then you
    may increase the PSP clock frequency for a better 
    emulation experience.

  Basic version :
    You may specify the basic version you want, with
    floppy or tape support.

11. COMPILATION
   ------------

  It has been developped under Linux using gcc with PSPSDK. 
  To rebuild the homebrew run the Makefile in the src archive.

  In the src/doc directory you will find original readme and license files
  of THOM.
