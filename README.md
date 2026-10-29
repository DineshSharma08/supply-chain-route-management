# Supply Chain Route Management System

A web-based project that demonstrates how Data Structures and Algorithms (DSA) can be used to manage locations, routes, and deliveries in a supply chain.

## Project Overview

The Supply Chain Route Management System is designed to represent a basic supply chain network and demonstrate route planning and delivery management. It combines a web interface with fundamental DSA concepts.

The project also includes a separate DSA Lab page for demonstrating different data structures and algorithms.

## Objectives

- Manage locations and routes in a supply chain network.
- Calculate the shortest route between locations.
- Manage deliveries using a queue.
- Demonstrate common Data Structures and Algorithms.
- Provide a simple, interactive interface for learning and demonstration.

## Features

### Supply Chain Dashboard
- Add and manage locations.
- Add and manage routes.
- Calculate shortest routes using Dijkstra's algorithm.
- Add deliveries to a delivery queue.
- Process deliveries in FIFO order.
- View completed delivery history.
- Display supply chain information through a dashboard.

### DSA Lab
The separate DSA Lab demonstrates the following concepts:

1. **Graph:** Represents locations and connections between them.
2. **Dijkstra's Algorithm:** Finds a shortest path based on route distances.
3. **Queue:** Demonstrates First In, First Out (FIFO) operations.
4. **Stack:** Demonstrates Last In, First Out (LIFO) operations.
5. **Sorting:** Demonstrates sorting algorithms.
6. **Searching:** Demonstrates linear and binary search.
7. **Linked List:** Demonstrates basic linked-list operations.

The exact operations available depend on the implementation of each module.

## Technologies Used

- **C:** Backend HTTP server and file-handling functionality.
- **HTML:** Structure of the web pages.
- **CSS:** Styling and layout.
- **JavaScript:** Interactive features and DSA demonstrations.
- **GitHub:** Source code hosting.
- **GitHub Pages:** Hosting the static website frontend.

## Data Structures and Algorithms

| Concept | Purpose |
|---|---|
| Graph | Represents locations and routes |
| Dijkstra's Algorithm | Calculates shortest paths |
| Queue | Processes deliveries in FIFO order |
| Stack | Demonstrates LIFO operations |
| Sorting Algorithms | Arranges data in a selected order |
| Searching Algorithms | Finds specific elements |
| Linked List | Demonstrates dynamically linked nodes |

## Project Structure

```text
supply-chain-route-management/
│
├── index.html
├── style.css
├── app.js
├── dsa-lab.html
├── server.c
├── locations.txt
├── routes.txt
├── deliveries.txt
└── completed_deliveries.txt
```

Some text files may be created or updated when the local C backend uses them. Files that are not present in your repository can be omitted from this list.

## How to Run the Website

### Online

1. Open the GitHub Pages website link provided in the repository settings.
2. Use the dashboard to explore the available features.
3. Open the DSA Lab page to explore the DSA demonstrations.

The DSA Lab page is available online only if `dsa-lab.html` has been uploaded to the published location.

### Run the C Server Locally on Windows

If GCC is installed, open PowerShell in the project folder and compile the C server:

```powershell
gcc server.c -o website_server.exe -lws2
