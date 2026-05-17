<div align="center">

# 🏢 Staywise – Smart Campus Hostel ERP System

[![Built With C++](https://img.shields.io/badge/Backend-C++17-00599C.svg?style=for-the-badge&logo=c%2B%2B)](https://isocpp.org/)
[![Vanilla JS](https://img.shields.io/badge/Frontend-Vanilla_JS-F7DF1E.svg?style=for-the-badge&logo=javascript)](https://developer.mozilla.org/en-US/docs/Web/JavaScript)
[![License](https://img.shields.io/badge/License-MIT-blue.svg?style=for-the-badge)](#)

*An algorithmic resource planning platform for campus accommodation management, powered by a custom C++ HTTP server and advanced Data Structures.*

</div>

---

## 📖 Project Overview
**Staywise** is a comprehensive Campus Hostel ERP system designed to automate student housing management through intelligent algorithms. Moving beyond traditional CRUD applications, Staywise leverages fundamental Data Structures and Algorithms (DSA) to handle complex constraints such as roommate compatibility, dynamic waitlist resolution, cyclic roommate swaps, and multi-semester renewal workflows. 

## 🚨 Real-World Problem Solved
Managing campus accommodation involves navigating compatibility disputes, inefficient vacancy tracking during mid-semester dropouts, and chaotic renewal processes. Staywise resolves these pain points by introducing an automated, constraint-satisfaction allocation engine. It minimizes manual administrative overhead, resolves multi-way swap requests seamlessly, and ensures fair room assignments through strict preference enforcement and risk-based priority queues.

---

## 🏗️ System Architecture

### Frontend Architecture and UI Modules
The frontend is a modular Single Page Application (SPA) utilizing vanilla web technologies for a lightweight, high-performance dashboard. The UI aligns with the core backend capabilities:

* **Add / Edit Resident**: A comprehensive onboarding module capturing personal details, academic standing, habits (sleep, study, cleanliness), and strict roommate/room preferences.
* **Waitlist Management & Smart Allocation**: The primary control interface that visualizes the Priority Queue. It allows admins to trigger the backend's batch processing engine to allocate rooms based on calculated fairness scores and risk levels.
* **Manual Room Assignment**: An override interface permitting administrators to bypass automated logic and directly assign students to specific vacant rooms.
* **Recover Vacancies**: A garbage collection routine that identifies high-risk no-shows or dropouts, instantly reclaiming beds and triggering waitlist evaluations.
* **Check Match**: A standalone tool utilizing the compatibility engine to instantly calculate a match score between any two residents.
* **Swap Room System**: An advanced module where admins can link residents wanting to swap rooms. It processes multi-node cyclic swaps and includes an undo stack for reversions.
* **Semester Renewal**: The lifecycle management module handling academic term transitions. It processes academic eligibility, automates graduating cohort evictions, retains students with backlogs, and promotes waitlisted students.
* **Reports Dashboard**: An analytics view providing real-time occupancy metrics and exporting TXT-based system records.

### Backend Architecture and Services
The backend is a high-performance, monolithic C++ engine running its own **custom Socket HTTP Server** (`<sys/socket.h>`). It operates entirely in-memory for lightning-fast algorithmic execution, persisting state to flat-file text databases (`rooms.txt`, `students.txt`).

---

## 🧠 Core Workflows & Algorithmic Engines

### 1. Smart Allocation Workflow
The system runs a greedy constraint-satisfaction algorithm sorted by an AVL Tree. It matches students based on a composite score (CGPA, seniority, special needs, and risk penalty) while strictly enforcing gender limits and requested room capacities.

### 2. DFS-based Roommate Swap Engine
For room swap requests, Staywise utilizes a **Depth-First Search (DFS)** algorithm on a directed graph mapping resident swap desires. This enables the system to find and execute optimal, multi-node swap cycles without causing cascading conflicts.

### 3. Room Compatibility Engine
The engine employs a scoring matrix based on behavioral habit vectors (study habits, sleep schedules, cleanliness, food preferences) to calculate precise match scores, utilizing **Dynamic Programming (Memoization)** to cache previously computed pairs.

### 4. Waitlist Management System
A dynamic **Priority Queue (Max-Heap)** driven waitlist. It calculates a priority score factoring in wait time, fairness score (rejections), seniority, and risk, ensuring deterministic and fair processing. It includes a starvation-prevention fairness queue.

### 5. Vacancy Recovery System
A background process that scans for "High Risk" students (calculated via DP memoization based on distance and past no-show patterns). It vacates high-risk profiles and optimally fills the void from the waitlist.

### 6. Semester Renewal Workflow
Handles state transitions between academic terms by assessing boolean flags (`examPassed`, `hasBacklog`, `placementExtension`). It safely processes checkouts for eligible Year-4 students and upgrades academic years for continuing residents.

---

## 🛠️ Data Structures & Algorithms Used

| Component / Feature | Core Data Structure | Algorithm / Approach | Purpose in ERP |
| :--- | :--- | :--- | :--- |
| **Waitlist Management** | **Priority Queue (Max-Heap)** | Custom Comparator Sorting | Ensures fair processing of pending requests based on a dynamic priority composite score. |
| **Student Ranking** | **AVL Tree** | Self-Balancing BST | Highly optimized sorting of student records by CGPA and composite metrics for the greedy allocation engine. |
| **Roommate Swap Engine** | **Directed Graph** | DFS Cycle Detection | Finding optimal multi-way room swaps to resolve cyclic exchange requests cleanly. |
| **Swap Reversion** | **Stack** | LIFO State Management | Stores recent swap states to allow administrators to "Undo Last Swap". |
| **Compatibility & Risk** | **Hash Maps (O(1))** | DP Memoization | Caching expensive habit-vector calculations and heuristic risk predictions for instant retrieval. |
| **Fairness Rotation** | **Queue** | FIFO Processing | Prevents waitlist starvation by rotating skipped students and boosting their fairness scores. |

---

## 💻 Tech Stack
- **Frontend Core**: HTML5, CSS3, Vanilla JavaScript (ES6+)
- **Backend Core Engine**: C++17 (Custom standard library socket server)
- **Data Persistence**: Custom File I/O (TXT based flat-file database for high-throughput localized data)
- **Networking**: `<sys/socket.h>`, `<netinet/in.h>` (No external frameworks like Crow or Oat++)

---

## 📂 Folder Structure

```text
smart-campus-hostel/
├── backend/
│   ├── main.cpp              # C++ Core Engine & Custom HTTP Server
│   └── hostel_server         # Compiled Executable (Binary)
├── frontend/
│   ├── index.html            # Main Application UI
│   ├── style.css             # Styling & Theming
│   └── script.js             # Client-side Logic & DOM Manipulation
├── sample_data/
│   ├── rooms.txt             # Initial Room Configuration Data
│   ├── students.txt          # Student Profiles Database
│   └── settings.txt          # System Configuration Parameters
├── exported_reports/         # Auto-generated System Reports
│   ├── analytics_report.txt
│   ├── room_allocations.txt
│   ├── student_records.txt
│   └── waitlist_report.txt
├── screenshots/              # Application Previews and UI Demos
└── README.md                 # Project Documentation
```

---

## 🚀 Installation & Setup Guide

### Prerequisites
- Unix-based OS (macOS/Linux) or WSL for Windows (required for POSIX socket headers)
- C++17 Compatible Compiler (e.g., `g++`, `clang++`)
- Any Modern Web Browser

### Backend Compilation & Execution
1. Open your terminal and navigate to the backend directory:
   ```bash
   cd smart-campus-hostel/backend
   ```
2. Compile the high-performance core engine:
   ```bash
   g++ -std=c++17 main.cpp -o hostel_server
   ```
3. Run the executable. It will automatically create missing data directories and start the HTTP server on port 8080.
   ```bash
   ./hostel_server
   ```

### Frontend Launch
1. With the backend running, open your web browser.
2. Navigate directly to the custom server address:
   ```text
   http://localhost:8080
   ```
   *(No external live server is needed, the C++ binary serves the frontend files natively).*

---

**Team Members:**
- Prathamesh More
- Sneha Chaturvedi
- Swaraj Jadhav
- Prince Singh

---

<div align="center">
  &copy; 2026 Prathamesh More. All Rights Reserved.<br>
  <i>Staywise - Elevating Campus Living through Intelligent Automation.</i>
</div>
