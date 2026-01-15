# 🌊 OceanRoute Nav — Advanced Maritime Navigation Optimizer

<p align="center">
  <img width="600" alt="OceanRoute Nav Preview" src="file_00000000f85471faa93d3060352b01e0">
</p>

<p align="center">
  <i>Optimizing maritime routes with intelligence, efficiency, and real-time visualization.</i>
</p>

---

## 📌 Project Overview
**OceanRoute Nav** is a cutting-edge maritime navigation and logistics visualization system developed in **C++** using **SFML**. It optimizes cargo ship routes between international ports while balancing **travel time, fuel cost, and docking constraints**. This project highlights advanced use of **data structures**, **graph algorithms**, and **real-time visual feedback**, providing both functionality and an intuitive user interface.

---

## ✨ Key Features

### 🗺 Route Data Representation
- Ports and sea routes are modeled as **graphs** with vertices (ports) and edges (routes).  
- Each edge stores detailed voyage info: shipping company, cost, duration, and schedule.  
- Enables realistic modeling of international shipping networks.

### 🚢 Ship Route Booking
- Users select origin, destination, and date.  
- Both direct and connecting routes are visualized.  
- Feasibility checks ensure **docking availability** and smooth **cargo transfers**.

### ⚡ Optimal Path Finding
- Implements **Dijkstra’s** and **A\*** algorithms for:
  - Shortest-time routes  
  - Cost-effective routes  
- Real-time **visual feedback** highlights the algorithm’s decision-making process.

### ⚙ Custom Ship Preferences
- Filter routes by:
  - Preferred shipping companies  
  - Ports to avoid  
  - Maximum voyage duration  
- Enhances user control and operational flexibility.

### 🏗 Docking & Layover Management
- Each port maintains a **first-come-first-serve queue**.  
- Dynamic visualization of **ship arrivals, departures, and layovers**.  

### 🔄 Multi-leg Route Management
- Each leg of a journey is stored in a **linked list**, allowing:
  - Easy dynamic modification  
  - Step-by-step visualization of multi-stop voyages  

### 🔍 Graphical Query & Subgraph Generation
- Filter routes by shipping company, cost, or port status.  
- Inactive or congested ports are **visually dimmed**, simplifying analysis.  

### 🎨 Advanced Visual Feedback
- Step-by-step **algorithm animations**  
- Dynamic **highlighting of ports and routes**  
- Intuitive **SFML-based UI** for real-time monitoring

---

## 🛠 Technologies Used
| Category | Technology |
|----------|-----------|
| **Language** | C++ |
| **Graphics & UI** | SFML |
| **Data Structures** | Graphs, Linked Lists, Queues, Priority Queues, Trees |
| **Algorithms** | Dijkstra, A* |
| **Development Environment** | Visual Studio, Code::Blocks, GCC |

---

## 📂 Project File Structure
OceanRoute_Nav/
│
├── src/ → All C++ source files (main.cpp, modules, utilities)
├── SFML/ → SFML library setup and dependencies
├── Data/ → Routes.txt, PortCharges.txt
├── Images/ → Port and route assets
├── Font/ → Fonts for display
├── Music/ → Background and sound effects
├── Videos/ → Demonstration recordings
├── Others/ → Helper files and additional assets
├── Pdf/ → Documentation and reports
├── main.cpp → Entry point
├── RouteManager.cpp → Route calculation & booking logic
├── Graph.cpp → Graph & algorithm implementations
└── .gitignore → Standard Git ignore


---

## 🚀 Getting Started

### 1. Clone the repository
git clone https://github.com/your-username/OceanRoute_Nav.git
cd OceanRoute_Nav

2. Compile the project
g++ src/*.cpp -o OceanRouteNav -lsfml-graphics -lsfml-window -lsfml-system

3. Run the executable
./OceanRouteNav


📊 Learning Outcomes

Mastery of graph-based data structures for real-world routing problems

Practical implementation of pathfinding algorithms (Dijkstra & A*)

Real-time SFML graphical rendering

Dynamic management of queues and linked lists for multi-leg voyages

Integration of user constraints and preferences into algorithmic solutions

🚧 Future Enhancements

AI-based predictive routing (weather, congestion, emergencies)

Enhanced cargo optimization and fuel-efficiency algorithms

Interactive zoom/pan map features

Networked multiplayer simulation for ship tracking

👨‍💻 Contributors

Sohaib Akhlaq – Core Algorithm & UI

Hasaam – Data Structures & Visualization
