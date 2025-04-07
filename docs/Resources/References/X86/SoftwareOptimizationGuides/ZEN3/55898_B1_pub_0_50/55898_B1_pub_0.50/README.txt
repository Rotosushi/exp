PPR Zip File README
-------------------

Contents
--------
  1. Acrobat Reader Configuration
      - Open Hyperlink document in new window
      - Show Navigation Pane
      - Show Toolbars
      - Show "Previous View" (left arrow) and "Next View" (right arrow) buttons
  2. Acrobat Reader Usage
      - Searching a multiple volume PPR
      - Cross-References and Hyperlinks
  3. PDF Organization


1. Acrobat Reader Configuration
-------------------------------
The following settings are recommended to optimize viewing the PPR pdfs.

  1.1 Open Hyperlink document in new window
  -----------------------------------------
  - Edit->Preferences:
    - Documents:
      - Open Settings:
        - Deselect: Open cross-document links in same window
          - Only when deselected are previously opened files visible in Windows pull-down. 
            This additional difference is not obvious from the option name.
            
  1.2 Show Navigation Pane
  ------------------------
  - If Navigation Pane is not shown:
    - View->Show/Hide->Navigation Panes->Show Navigation Pane
    - This allows users to jump to headings using bookmarks.
    
  1.3 Show Toolbars
  ------------------------  
  - If Toolbars is not shown:
    - View->Show/Hide->Toolbar Items->Show Toolbars
    - The toolbar is needed to see previous view and next view buttons.
    
  1.4 Show "Previous View" (left arrow) and "Next View" (right arrow) buttons
  ------------------------  
  - If "Previous View" (left arrow) and "Next View" (right arrow) buttons are not shown:
    - Right click on toolbar-> Page Navigation-> select "Previous View" and "Next View" items.

2. Acrobat Reader Usage
-----------------------

  2.1 Searching a multiple volume PPR
  -----------------------------------
  1. Open search by selecting Edit -> Advanced Search (Shift+Ctrl+F)
  2. Select "All PDF Documents in" and select "Browse for Location...".
      Selecting "All PDF Documents in" causes search to operate on all PDFs in the selected location.
      
  2.2 Cross-References and Hyperlinks
  -----------------------------------
  A cross-reference is a link to another location within the same PDF. 
  A hyperlink is a link to another PDF file that is underlined and blue (if uncolored).
  For cross-references, use "Previous View" to return from the current location to the previous location.
  Hyperlinks between documents leave the current location unchanged in the PDF that contained the hyperlink.
    
3. PDF Organization
-------------------
*** Note: Cross-references between volumes will not work until the zip file is extracted.

If the PPR consists of multiple volumes, then the volume is identified by the number suffix in the 
  file name. E.g. _1 indicates volume 1.
  
The Full table of contents for all volumes is located in volume 1. Each volume other than volume 1 has the table of
contents only for that volume.
 
