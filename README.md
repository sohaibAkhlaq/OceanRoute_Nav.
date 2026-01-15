# 🌊 OceanRoute Nav — Advanced Maritime Navigation Optimizer


<p align="center">
  <i>Optimizing maritime routes with intelligence, efficiency, and real-time visualization.</i>
</p>

---

## 🚀 Project Overview
**OceanRoute Nav** is an intelligent maritime navigation and logistics visualization system built in **C++** with **SFML**.  
It optimizes cargo ship routes between international ports while balancing:

- ⏱ Travel Time  
- 💰 Fuel & Operational Cost  
- ⚓ Docking Constraints  

This project demonstrates advanced mastery of **data structures**, **graph algorithms**, and **real-time visual rendering**.

---

## ✨ Key Features

### 🗺 Route Data Representation
- Ports & routes represented as **graphs** (vertices = ports, edges = routes)  
- Each edge stores **shipping company, cost, duration, date**  
- Realistic simulation of international maritime networks  

### 🚢 Ship Route Booking
- Select origin, destination, and date  
- Visualize **direct and connecting routes**  
- Feasibility checks for docking & cargo transfers  

### ⚡ Optimal Pathfinding
- Implements **Dijkstra** & **A*** algorithms  
- Find **shortest-time** or **cheapest** routes  
- **Step-by-step visual feedback** during calculation  

### ⚙ Custom Ship Preferences
- Filter routes by:  
  - Preferred shipping companies  
  - Ports to avoid  
  - Maximum voyage duration  

### 🏗 Docking & Layover Management
- Each port maintains a **first-come-first-serve queue**  
- Ships visualized dynamically **arriving, docking, and departing**  

### 🔄 Multi-leg Route Management
- Multi-stop journeys represented using **linked lists**  
- Dynamic modification of routes with real-time updates  

### 🔍 Graphical Query & Subgraph Generation
- Filter routes by **company, cost, duration, or port activity**  
- Inactive or congested ports are visually dimmed  

### 🎨 Advanced Visual Feedback
- Animated algorithm progress  
- Highlighting & fading for ports & routes  
- SFML-based **real-time UI**

---

## 🛠 Technologies Used
| Category | Technology |
|----------|-----------|
| **Language** | C++ |
| **Graphics & UI** | SFML ([https://www.sfml-dev.org/](https://www.sfml-dev.org/)) |
| **Data Structures** | Graphs, Linked Lists, Queues, Priority Queues, Trees |
| **Algorithms** | Dijkstra, A* |
| **IDE/Compiler** | Visual Studio / Code::Blocks / GCC |

---

## 📂 Project File Structure
OceanRoute_Nav/
├── src/
│ └── main.cpp
│ └── RouteManager.cpp
│ └── Graph.cpp
├── SFML/
├── Data/
│ └── Routes.txt
│ └── PortCharges.txt
├── Images/
├── Font/
├── Music/
├── Videos/
├── Others/
├── Pdf/
└── .gitignore

---

## 📌 Badges
<p align="center">
  <a href="https://github.com/your-username/OceanRoute_Nav">
    <img src="https://img.shields.io/badge/Status-Active-brightgreen" alt="Project Status"/>
  </a>
  <a href="https://github.com/your-username/OceanRoute_Nav">
    <img src="https://img.shields.io/badge/C++-100%25-blue" alt="Language"/>
  </a>
  <a href="https://www.sfml-dev.org/">
    <img src="https://img.shields.io/badge/SFML-Graphics-orange" alt="SFML"/>
  </a>
  <a href="https://github.com/your-username/OceanRoute_Nav/stargazers">
    <img src="https://img.shields.io/github/stars/your-username/OceanRoute_Nav?style=social" alt="Stars"/>
  </a>
</p>

---

## 🚀 How to Run

### 1️⃣ Clone the repository

git clone https://github.com/sohaibAkhlaq/OceanRoute_Nav.git
cd OceanRoute_Nav

2️⃣ Compile the project

g++ src/*.cpp -o OceanRouteNav -lsfml-graphics -lsfml-window -lsfml-system

3️⃣ Run the executable

./OceanRouteNav

---

## 📊 Learning Outcomes

Mastery of graph-based data structures in real-world routing

Implementation of pathfinding algorithms (Dijkstra & A*)

Real-time graphical visualization using SFML

Dynamic management of queues & linked lists for multi-leg journeys

Incorporating user constraints into algorithmic solutions

---

## 🚧 Future Enhancements

AI-driven predictive routing (weather, congestion)

Enhanced cargo optimization & fuel efficiency

Interactive map zoom/pan features

Networked multiplayer ship tracking

---

## 👨‍💻 Contributors

Sohaib Akhlaq – Core Algorithms & UI

Hasaam – Data Structures & Visualization

---

## 🎯 Why It Stands Out

OceanRoute Nav combines high-performance algorithms, advanced data structures, and immersive real-time visualization to solve complex maritime routing problems.
Its professional interface, animated pathfinding, and predictive features make it ideal for portfolios, internships, and real-world shipping applications.

---

## 💡 Visual Enhancements Included

Interactive route maps with highlighting & fading effects

Animated algorithm path exploration

Dynamic dock & ship status visualization

Filterable graph subviews by company, cost, and duration

---

<p align="center"> <i>Designed to demonstrate advanced C++ skills, algorithmic problem-solving, and real-time visualization expertise.</i> </p> ```
