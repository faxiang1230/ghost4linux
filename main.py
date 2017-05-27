#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#

# Creates three "windows" that the user can navigate through using Back and Next - buttons.

from Tkinter import * 
import Tkinter as tkinter
import commands
from _threading_local import local
btn1 = IntVar()
class BatchIndiv():
    def __init__(self, master):
        self.master=master
        self.startwindow()
    def get_config(self):
        userip = configIP.get()
        print "userIP:" + userip
        
    def create_widgets_in_first_frame(self):
        # Create the label for the frame
        first_window_label = Label(self.first_frame, text='Welcome to G4L')
        first_window_label.grid(column=0, row=0, pady=10, padx=10, sticky=(tkinter.N))

        # Create the button for the frame
        first_window_quit_button = tkinter.Button(self.first_frame, text = "Quit", command = self.quit_program)
        first_window_quit_button.grid(column=0, row=1, pady=10, sticky=(tkinter.N))
        first_window_next_button = tkinter.Button(self.first_frame, text = "Next", command = self.call_second_frame_on_top)
        first_window_next_button.grid(column=1, row=1, pady=10, sticky=(tkinter.N))

    def create_widgets_in_second_frame(self):
        # Create the label for the frame
        second_window_label = tkinter.Label(self.second_frame, text='config your ftp parameter')
        second_window_label.grid(column=0, row=0, pady=10, padx=10, sticky=(tkinter.N))

        configIPLabel = Label(self.second_frame, text='IP:')
        configIPLabel.grid(column=0, row = 1)
        global configIP
        configIP = Entry(self.second_frame)
        configIP.grid(column=1, row = 1)
        
        configUSERLabel = Label(self.second_frame, text='username:')
        configUSERLabel.grid(column=0, row = 2)
        global configuser
        configuser = Entry(self.second_frame)
        configuser.grid(column=1, row = 2)
        
        configPASSWDLabel = Label(self.second_frame, text='passwd:')
        configPASSWDLabel.grid(column=0, row = 3)
        global configpw
        configpw = Entry(self.second_frame)
        configpw.grid(column=1, row = 3)
        
        configPORTLabel = Label(self.second_frame, text='port:')
        configPORTLabel.grid(column=0, row = 4)
        global configport
        configport = Entry(self.second_frame)
        configport.grid(column=1, row = 4)
         
        # Create the button for the frame
        second_window_back_button = tkinter.Button(self.second_frame, text = "Back", command = self.call_first_frame_on_top)
        second_window_back_button.grid(column=0, row=5, pady=10, sticky=(tkinter.N))
        second_window_next_button = tkinter.Button(self.second_frame, text = "Next", command = self.call_third_frame_on_top)
        second_window_next_button.grid(column=1, row=5, pady=10, sticky=(tkinter.N))

    def create_widgets_in_third_frame(self):
        # Create the label for the frame
        third_window_label = tkinter.Label(self.third_frame, text='BACKUP or RESTORE')
        third_window_label.grid(column=0, row=0, pady=10, padx=10, sticky=(tkinter.N))

        # Create the button for the frame
        third_window_back_button = tkinter.Button(self.third_frame, text = "BackUP", command = self.select_dev())
        third_window_back_button.grid(column=0, row=1, pady=10, sticky=(tkinter.N))
        third_window_quit_button = tkinter.Button(self.third_frame, text = "RESTORE", command = self.select_dev)
        third_window_quit_button.grid(column=1, row=1, pady=10, sticky=(tkinter.N))
    def backup_dev(self):
        print btn1.get()
    def select_dev(self):
        ret,result=commands.getstatusoutput('blkid|awk -F ":" \'{print $1}\'')
        print result
        fourth_frame=tkinter.Frame(self.root_window, width=self.window_width, height=self.window_heigth)
        fourth_frame['borderwidth'] = 2
        fourth_frame['relief'] = 'sunken'
        fourth_frame.grid(column=0, row=0, padx=20, pady=5, sticky=(tkinter.W, tkinter.N, tkinter.E))
        num = 0
        for tmp in result.split():
            mRadiobtn = Radiobutton(fourth_frame,text=tmp,value=num,variable=btn1)
            mRadiobtn.grid(column=0,row=num)
            num = num + 1
      
        third_window_quit_button = tkinter.Button(fourth_frame, text = "Next", command = self.backup_dev)
        third_window_quit_button.grid(column=0, row=num, pady=10, sticky=(tkinter.N))
        self.third_frame.grid_forget()
        fourth_frame.grid(column=0, row=0, padx=20, pady=5, sticky=(tkinter.W, tkinter.N, tkinter.E))
    def call_first_frame_on_top(self):
        # This function can be called only from the second window.
        # Hide the second window and show the first window.
        self.second_frame.grid_forget()
        self.first_frame.grid(column=0, row=0, padx=20, pady=5, sticky=(tkinter.W, tkinter.N, tkinter.E))

    def call_second_frame_on_top(self):
        # This function can be called from the first and third windows.
        # Hide the first and third windows and show the second window.
        self.first_frame.grid_forget()
        self.third_frame.grid_forget()
        self.second_frame.grid(column=0, row=0, padx=20, pady=5, sticky=(tkinter.W, tkinter.N, tkinter.E))

    def call_third_frame_on_top(self):
        # This function can only be called from the second window.
        # Hide the second window and show the third window.
        self.get_config()
        self.second_frame.grid_forget()
        self.third_frame.grid(column=0, row=0, padx=20, pady=5, sticky=(tkinter.W, tkinter.N, tkinter.E))

    def quit_program(self):
        self.root_window.destroy()

    ###############################
    # Main program starts here :) #
    ###############################

    # Create the root GUI window.
    root_window = tkinter.Tk()

    # Define window size
    window_width = 200
    window_heigth = 100

    # Create frames inside the root window to hold other GUI elements. All frames must be created in the main program, otherwise they are not accessible in functions. 
    first_frame=tkinter.Frame(root_window, width=window_width, height=window_heigth)
    first_frame['borderwidth'] = 2
    first_frame['relief'] = 'sunken'
    first_frame.grid(column=0, row=0, padx=20, pady=5, sticky=(tkinter.W, tkinter.N, tkinter.E))

    second_frame=tkinter.Frame(root_window, width=window_width, height=window_heigth)
    second_frame['borderwidth'] = 2
    second_frame['relief'] = 'sunken'
    second_frame.grid(column=0, row=0, padx=20, pady=5, sticky=(tkinter.W, tkinter.N, tkinter.E))

    third_frame=tkinter.Frame(root_window, width=window_width, height=window_heigth)
    third_frame['borderwidth'] = 2
    third_frame['relief'] = 'sunken'
    third_frame.grid(column=0, row=0, padx=20, pady=5, sticky=(tkinter.W, tkinter.N, tkinter.E))

    # Create all widgets to all frames
    create_widgets_in_third_frame()
    create_widgets_in_second_frame()
    create_widgets_in_first_frame()

    # Hide all frames in reverse order, but leave first frame visible (unhidden).
    third_frame.grid_forget()
    second_frame.grid_forget()

    # Start tkinter event - loop
    root_window.mainloop()  

BatchIndiv()