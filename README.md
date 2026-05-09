# Smart Campus Hostel Allocation and Resource Optimization System

## 1. Problem Statement
Hostel allocation in educational institutions is often a manual, time-consuming process fraught with inefficiencies. Traditional systems struggle with maintaining fairness, resolving complex room swap requests, predicting no-shows, and matching roommates based on compatibility. Furthermore, the lack of an intelligent waitlist mechanism often results in sub-optimal utilization of hostel resources. There is a critical need for an automated, data-driven system capable of handling complex constraints such as gender-specific accommodations, dynamic priority queues, and real-time resource optimization.

## 2. Objective
The objective of this project is to design and implement a robust, highly efficient menu-driven C++ console application that automates and optimizes hostel allocation and management. By leveraging advanced Data Structures and Algorithms (DSA), the system aims to maximize resource utilization, ensure fairness in allocations, intelligently handle waitlists, provide a safe mechanism for room swaps, and generate comprehensive analytical reports for administrative decision-making.

## 3. Key Features
- **Smart Room Allocation:** Automated assignment of rooms based on priority criteria, minimizing vacant slots.
- **Waitlist Management:** Dynamic handling of unallocated students using priority-based structures.
- **Fairness Handling:** Ensuring equitable distribution of resources over time via queue rotation.
- **No-Show Prediction:** Algorithmic risk prediction to anticipate and mitigate the impact of student no-shows.
- **Compatibility-Based Roommate Matching:** Intelligent pairing of students based on behavioral and academic compatibility scores.
- **Graph-Based Room Swaps:** Resolution of multi-party room swap requests using graph cycle detection.
- **Vacancy Recovery:** Automated identification and reallocation of newly vacated rooms.
- **Semester Hostel Renewal:** Streamlined processing for continuing students extending their stay.
- **Report Export System:** Automated generation of detailed text-based reports for analytics and record-keeping.

## 4. Innovative Features

### Gender-Aware Smart Student ID Validation
The system employs a strict validation mechanism at the data ingestion layer. It parses and validates student IDs using specific formats: `SM01` for male students and `SF01` for female students. This structural validation ensures data integrity, prevents cross-gender allocation errors natively, and strictly rejects duplicate student entries, maintaining a consistent and reliable data state.

### Compatibility-Aware Intelligent Room Allocation System
Moving beyond basic priority or first-come-first-serve algorithms, the system implements a quantitative compatibility scoring engine. It evaluates multidimensional data points of students to generate a compatibility score. When allocating shared rooms, the system prioritizes matching students with high mutual compatibility, thereby reducing subsequent room swap requests and fostering a better living environment.

## 5. Data Structures and Algorithms Used

| Concept | Application in System |
| :--- | :--- |
| **AVL Tree** | Maintains the central student database. Enables self-balancing, ensuring $O(\log n)$ time complexity for search, insertion, and deletion of student records based on Student IDs. |
| **Linked List** | Utilized in the underlying representation of adjacency lists for the swap graph and dynamic chaining. |
| **Queue** | Manages the standard processing lines, specifically utilized in the fairness queue rotation to handle requests chronologically. |
| **Priority Queue** | Powers the intelligent waitlist management. Students are dequeued based on a dynamically calculated priority score rather than mere arrival time. |
| **Stack** | Implements the Undo Swap Mechanism. Stores the history of room swaps, allowing the system to perform an $O(1)$ rollback to previous states. |
| **Graph** | Models the complex room swap ecosystem. Students represent nodes, and swap requests represent directed edges. |
| **DFS (Depth-First Search)** | Operates on the swap graph to detect cycles. A detected cycle indicates a perfectly resolvable multi-way room swap without leaving any student displaced. |
| **Dynamic Programming (Memoization)** | Optimizes the calculation of risk prediction and compatibility scores by caching overlapping subproblem results, significantly reducing computational overhead. |
| **Greedy Algorithm** | Drives the primary room allocation logic. It consistently makes the locally optimal choice by assigning the highest priority student to the best available room, aiming for global resource optimization. |
| **Sorting** | Employed during the generation of exported reports to present data sequentially (e.g., ranking students by CGPA or ordering room allocations). |

## 6. System Architecture
The application follows a monolithic, modular architecture designed for high throughput in a console environment. It consists of the following layers:
- **Data Ingestion Layer:** Parses raw text files (`rooms.txt`, `students.txt`), applies validation rules, and constructs initial data structures.
- **Core Processing Engine:** Houses the implementations of all algorithmic modules (Allocation, Swaps, Predictions).
- **State Management:** Utilizes the aforementioned data structures to hold the application state in memory securely.
- **I/O & Reporting Layer:** Handles the menu-driven user interface and handles text-based serialization for report generation.

## 7. Menu Driven Modules
The system exposes its functionalities through an interactive, text-based console menu. Typical options include, but are not limited to:
- Initializing the allocation engine.
- Viewing current room occupancy and vacancies.
- Processing the waitlist.
- Initiating and evaluating room swap requests.
- Triggering fairness rotations.
- Generating predictive analytics.
- Exporting system state to reports.

## 8. Input File Structure
The system relies on structured text files for initial data loading, ensuring persistence across sessions.
- **`students.txt`:** Contains student demographic and metric data. Expected format handles fields such as ID, Name, Gender, CGPA, and specific behavioral flags used for compatibility and risk scoring.
- **`rooms.txt`:** Defines the physical infrastructure, detailing Room ID, Capacity, Gender classification, and current occupancy status.

## 9. Output Reports
To facilitate administrative review, the system automatically serializes its internal state into formatted text files within the `exported_reports/` directory:
- `analytics_report.txt`: High-level metrics, occupancy rates, and predicted risk factors.
- `room_allocations.txt`: A comprehensive mapping of all assigned rooms and their occupants.
- `student_records.txt`: A clean dump of all processed student profiles.
- `waitlist_report.txt`: The current state of the priority queue, showing pending allocations.

## 10. Validation Features
- **Format Constraints:** Strict adherence to `SM`/`SF` prefixes for IDs.
- **Duplicate Prevention:** The AVL tree structure intrinsically prevents the insertion of duplicate Student IDs.
- **Data Bounds:** Numerical inputs (e.g., CGPA, behavioral scores) are validated against defined acceptable ranges during ingestion.

## 11. Fairness Mechanism
To prevent starvation of specific student groups within the waitlist, the system incorporates a fairness queue rotation algorithm. Periodically, the system boosts the priority of long-standing waitlisted students, ensuring that high-priority new arrivals do not perpetually bypass older, slightly lower-priority requests.

## 12. Swap Safety Rules
The swap rollback system is heavily constrained to prevent invalid states:
- Swaps must be strictly within the same gender classification.
- A multi-way swap is only executed if a complete cycle is detected via DFS.
- The Stack-based undo mechanism allows administrators to revert the most recent swap securely if human error occurs.

## 13. Technologies Used
- **Language:** ISO C++ (C++11/C++14 standards)
- **Environment:** Native OS Console / Terminal
- **Storage:** Flat File System (.txt)
- **Architecture Paradigm:** Monolithic / Procedural with Object-Oriented elements

## 14. Complexity Analysis

| Operation | Time Complexity | Space Complexity |
| :--- | :--- | :--- |
| Search/Insert Student | $O(\log n)$ | $O(n)$ |
| Priority Waitlist Insertion | $O(\log k)$ | $O(k)$ |
| Cycle Detection (DFS) | $O(V + E)$ | $O(V)$ |
| Undo Swap | $O(1)$ | $O(S)$ |
| Report Generation | $O(n \log n)$ | $O(n)$ |

*(Where $n$ is total students, $k$ is waitlisted students, $V$/$E$ are graph vertices/edges for swaps, and $S$ is the number of historical swaps)*

## 15. Future Enhancements
- Migration from flat files to an embedded SQL database (e.g., SQLite).
- Integration of a graphical user interface (GUI) using frameworks like Qt.
- Implementation of a REST API to decouple the backend logic from the presentation layer.
- Enhancement of the prediction algorithms using machine learning models instead of static DP heuristics.

## 16. Folder Structure

```plaintext
SMART-CAMPUS-HOSTEL/
│
├── exported_reports/
│   ├── analytics_report.txt
│   ├── room_allocations.txt
│   ├── student_records.txt
│   └── waitlist_report.txt
│
├── sample_data/
│   ├── rooms.txt
│   └── students.txt
│
├── screenshots/
│
├── main.cpp
├── README.md
└── .gitignore
```

## 17. Conclusion
The Smart Campus Hostel Allocation and Resource Optimization System provides a comprehensive, algorithmic approach to facility management. By applying classical computer science concepts to a tangible administrative bottleneck, the application ensures high operational efficiency, strict data integrity, and equitable resource distribution, serving as a robust foundational architecture for institutional scalability.
