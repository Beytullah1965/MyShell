# Multi-Terminal GTK Application

This project is a GTK-based application that allows users to open multiple terminal windows, run commands, send private messages, and view command history. The application provides a graphical user interface (GUI) built with GTK, and ensures synchronization between terminal windows using shared memory and semaphores.

---

## **Main Features**

- **Multiple Terminals:** Multiple independent terminal windows can be opened.
- **Private Messaging:** Private messaging can be done between terminal windows.
- **Command History:** Users can view commands they have entered in the past.
- **PowerShell-Like Command Line:** A PowerShell-like prompt structure and command execution behavior.
- **Synchronization:** Synchronization between terminal windows is provided using shared memory and semaphores.

---

## **Features**

- Multiple terminal windows can be opened, each operating independently.
- Private messaging can be done between terminal windows. You can send a message to a specific terminal using the `@msg <terminal_id> <message>` command.
- Users can enter and execute commands using a PowerShell-like prompt (e.g., `PS C:\Users\>`).
- Commands entered are saved in the command history.
- Shared memory (shm) is used for data sharing between terminal windows.
- Semaphores are used to synchronize parallel operations.

---

## **Table of Contents**

1. **Dependencies**
2. **Installation**
3. **Usage**
4. **File Structure**
5. **Contributing**
6. **Application Details**
7. **Design Choices and Challenges**
8. **Performance Optimization**
9. **User Experience (UX) Improvements**

---

## **Dependencies**

The project uses the following tools and libraries:

- **GTK 3.x:** For GUI development. `(You can install it using sudo apt-get install libgtk-3-dev)`
- **POSIX Semaphores:** For synchronization between terminal windows.
- **Shared Memory (shm):** Facilitates data communication between terminal windows.
- **GDK-Pixbuf:** For terminal window icons and visual processing.
- **C Compiler (GCC or similar):** To compile the project.

### **Required Libraries for Installation:**

- **GTK+3:** `sudo apt-get install libgtk-3-dev`
- **GDK-Pixbuf:** `sudo apt-get install libgdk-pixbuf2.0-dev`
- **GCC:** `sudo apt-get install build-essential`

---

## **Usage**

1. **Launching the Application:** When the application starts, a terminal window will be opened.
2. **Add New Terminal:** You can add new terminal windows by clicking the "Add Terminal" button.
3. **Run Commands:** Commands can be written and executed using a PowerShell-like prompt (e.g., `PS C:\Users\>`).
4. **Private Messaging:** You can send messages to a specific terminal using the command `@msg <terminal_id> <message>`.
5. **Command History:** The commands you enter are saved in the history, and you can view them.
6. **GUI Customization:** You can customize the appearance of the terminal using CSS.

---

## **File Structure**

```bash
.
├── README.md               # Project documentation
├── Makefile                # Build file
├── main.c                  # Application entry point
├── model.c                 # Shared memory and semaphore operations
├── model.h                 # Header file for the model
├── controller.c            # Logic for processing commands
├── view.c                  # GUI and terminal window creation
├── view.h                  # View header file
└── terminal_icon.png       # Terminal window icon  

## Synchronization
The application ensures synchronization between terminal windows while allowing each terminal to operate independently by using shared memory (shm) and semaphores. Semaphores ensure that only one terminal can access the shared memory at a time.

## Command Execution
When users enter a command in the terminal window, the command is stored in shared memory and becomes valid for other terminal windows as well. The command is then processed, and its output is displayed in the terminal window.

## GUI Customization
The application is developed using GTK 3.x, and the appearance of the terminal windows can be customized using CSS. Users can change the background color, font, and other styling properties.

## Design Choices and Challenges

### Model-View-Controller (MVC) Design Pattern
The design of the application follows the MVC (Model-View-Controller) pattern. This design ensures that the code remains organized and sustainable.

- **Model:** Manages data such as shared memory and semaphores.
- **View:** Creates the user interface and communicates commands and messages to the user.
- **Controller:** Handles user interactions and controls the behavior of terminal windows based on data from the model.

### Challenges:
- Multiple terminal windows accessing shared memory simultaneously could lead to race conditions. Semaphores ensure that only one terminal can access the data at a time.
- Developing a GUI with GTK for terminal window-based applications can be complex.
- Proper management is required for command history and messaging systems.

## Performance Optimization
Performance is critical, especially when multiple terminal windows are running. The following methods are used to optimize the application's performance:

- **Shared Memory Usage:** Speeds up data sharing between terminal windows and eliminates unnecessary I/O operations.
- **Semaphore Management:** Ensures that each terminal window accesses shared memory only when necessary.
- **Resource Cleanup:** Resource management is regularly monitored.

## User Experience (UX) Improvements
User experience is essential for the application to be efficient and intuitive. UX improvements include:

- **Responsive Interface:** Terminal windows are resizable and customizable based on user needs.
- **Clear Feedback:** Visual feedback is provided to users to indicate that their commands are being processed.
- **Intuitive Command Input:** The PowerShell-like command structure makes it easy for users to quickly adapt.
- **Customization Options:** The visual appearance of the terminal windows can be customized using CSS.



## Conclusion
This project has enabled the development of an advanced multi-terminal application using shared memory and semaphores. Design choices, particularly the MVC pattern and resource management, have ensured that the application remains efficient and sustainable. The project has provided valuable insights into system programming and GUI development.


## Contributors
The following individuals contributed to this project:

- **Beytullah AYDIN** (23120205037)
- **Abdülhalık Enes BAKKAL** (22120205008)
- **Melih TURGUT** (23120205006)
- **Salih KARABULAK** (23120205050)



