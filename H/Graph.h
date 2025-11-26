#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <unordered_map>
#include <unordered_set>
#include <set>

#include <queue>
#include <vector>
#include <functional>
#include <limits>
#include "Helper.h"
#include "Time_Cal.h"
using namespace std;

class Edge
{
public:
    string dest;
    string date;
    string dep;
    string arr;
    int cost;
    string company;
    Edge *next;

    Edge(string d, string da, string de, string a, int c, string co)
    {
        dest = d;
        date = da;
        dep = de;
        arr = a;
        cost = c;
        company = co;
        next = nullptr;
    }
};

class Port
{
public:
    float lat, lon;
    bool left = false;
    float charges = 0;

    Port() {}
    Port(float la, float lo, bool l = false, float ch = 0)
    {
        lat = la;
        lon = lo;
        left = l;
        charges = ch;
    }
};

class PortNode
{
public:
    string name;
    Edge *head;
    PortNode *next;
    Port portData;

    PortNode(string n)
    {
        name = n;
        head = nullptr;
        next = nullptr;
    }
};

struct PathStep
{
    PortNode *from; // where this step starts
    PortNode *to;   // null for waiting step

    Edge *edge; // travel edge, null for waiting step

    bool isWaiting;    // true if port fee added
    float waitingCost; // waiting fee (0 for normal travel)

    long long departTime; // in minutes
    long long arriveTime; // in minutes

    int travelCost; // cost of this edge (0 for waiting event)

    // Optional little helper:
    string label() const
    {
        if (isWaiting)
            return "Wait >12h (Port Charges)";
        if (edge)
            return "Travel";
        return "Unknown";
    }
};

class Graph
{
private:
    PortNode *vertices;

    PortNode *findVertex(string name)
    {
        PortNode *temp = vertices;
        while (temp != nullptr)
        {
            if (temp->name == name)
                return temp;
            temp = temp->next;
        }
        return nullptr;
    }
    PortNode *findVertex_Lower(string name)
    {

        PortNode *temp = vertices;
        while (temp != nullptr)
        {
            if (to_lower(temp->name) == to_lower(name))
                return temp;
            temp = temp->next;
        }
        return nullptr;
    }

    PortNode *addVertex(string name)
    {
        PortNode *v = findVertex(name);
        if (v != nullptr)
            return v;
        PortNode *nv = new PortNode(name);
        nv->next = vertices;
        vertices = nv;
        return nv;
    }

    void addEdge(string src, string dest, string date, string dep, string arr, int cost, string company)
    {
        PortNode *v = addVertex(src);
        addVertex(dest);

        Edge *e = new Edge(dest, date, dep, arr, cost, company);
        e->next = v->head;
        v->head = e;
    }

    void sortEdges(PortNode *v)
    {
        if (!v || !v->head)
            return;

        bool swapped = true;
        while (swapped)
        {
            swapped = false;
            Edge *curr = v->head;
            Edge *prev = nullptr;

            while (curr->next != nullptr)
            {
                if (curr->dest > curr->next->dest)
                {
                    Edge *temp = curr->next;
                    curr->next = temp->next;
                    temp->next = curr;

                    if (prev == nullptr)
                        v->head = temp;
                    else
                        prev->next = temp;

                    swapped = true;
                    prev = temp;
                }
                else
                {
                    prev = curr;
                    curr = curr->next;
                }
            }
        }
    }

    void sortVertices()
    {
        if (!vertices)
            return;

        bool swapped = true;
        while (swapped)
        {
            swapped = false;
            PortNode *curr = vertices;
            PortNode *prev = nullptr;

            while (curr->next != nullptr)
            {
                if (curr->name > curr->next->name)
                {
                    PortNode *temp = curr->next;
                    curr->next = temp->next;
                    temp->next = curr;

                    if (prev == nullptr)
                        vertices = temp;
                    else
                        prev->next = temp;

                    swapped = true;
                    prev = temp;
                }
                else
                {
                    prev = curr;
                    curr = curr->next;
                }
            }
        }
    }

    void loadLocations(string file)
    {
        ifstream in(file);
        if (!in)
            return;

        string name;
        float lat, lon;
        string flag;

        while (in >> name >> lat >> lon)
        {
            in >> ws;
            getline(in, flag);
            bool leftFlag = (flag.find("L") != string::npos);

            PortNode *v = addVertex(name);
            v->portData.lat = lat;
            v->portData.lon = lon;
            v->portData.left = leftFlag;
        }
        in.close();
    }

    void loadCharges(string file)
    {
        ifstream in(file);
        if (!in)
            return;

        string name;
        float charge;
        while (in >> name >> charge)
        {
            PortNode *v = findVertex(name);
            if (v)
                v->portData.charges = charge;
        }
        in.close();
    }

public:
    Graph(string routesFile, string locationsFile, string chargesFile)
    {
        vertices = nullptr;
        loadRoutes(routesFile);
        loadLocations(locationsFile);
        loadCharges(chargesFile);

        sortVertices();
        PortNode *v = vertices;
        while (v != nullptr)
        {
            sortEdges(v);
            v = v->next;
        }
    }

    void printGraph(bool vertices_only = false)
    {
        if (vertices_only)
        {
            PortNode *v = vertices;
            int i = 1;
            while (v != nullptr)
            {
                cout << i++ << " " << colorText(v->name, BLUE)
                     << " (Lat=" << v->portData.lat
                     << ", Lon=" << v->portData.lon
                     << ", Left=" << v->portData.left
                     << ", Charges=" << v->portData.charges << ")\n";
                v = v->next;
            }
        }
        else
        {
            PortNode *v = vertices;
            while (v != nullptr)
            {
                cout << "[" << colorText(v->name, BLUE)
                     << " (Lat=" << v->portData.lat
                     << ", Lon=" << v->portData.lon
                     << ", Left=" << v->portData.left
                     << ", Charges=" << v->portData.charges << ")] ->\n";

                Edge *e = v->head;
                while (e != nullptr)
                {
                    cout << "    "
                         << setw(25) << left << colorText(e->dest, GREEN)
                         << setw(15) << left << e->date
                         << setw(15) << left << (e->dep + "-" + e->arr)
                         << "$" << setw(10) << left << e->cost
                         << colorText(e->company, RED)
                         << "\n";
                    e = e->next;
                }
                cout << "\n";
                v = v->next;
            }
        }
    }

    void loadRoutes(string file)
    {
        ifstream in(file.c_str());
        if (!in)
            return;

        string src, dest, date, dep, arr, company;
        int cost;

        while (in >> src >> dest >> date >> dep >> arr >> cost >> company)
        {
            addEdge(src, dest, date, dep, arr, cost, company);
        }

        in.close();
    }

    PortNode *getVertices() { return vertices; }
    PortNode *findVertexByName(string name) { return findVertex(name); }
    PortNode *findVertexByName_Lower(string name) { return findVertex_Lower(name); }

public:
    // ---------------- ITERATOR SUPPORT ----------------
    class Iterator
    {
        PortNode *current;

    public:
        Iterator(PortNode *start) : current(start) {}
        PortNode *operator*() const { return current; }
        Iterator &operator++()
        {
            current = current->next;
            return *this;
        }
        bool operator!=(const Iterator &other) const { return current != other.current; }
    };

    Iterator begin() { return Iterator(vertices); }
    Iterator end() { return Iterator(nullptr); }

    // Return vertex by index (0-based)
    PortNode &operator[](size_t index)
    {
        PortNode *temp = vertices;
        size_t i = 0;

        while (temp != nullptr && i < index)
        {
            temp = temp->next;
            i++;
        }

        if (!temp)
            throw out_of_range("Graph[] index out of range");

        return *temp;
    }

    // Const version
    const PortNode &operator[](size_t index) const
    {
        PortNode *temp = vertices;
        size_t i = 0;

        while (temp != nullptr && i < index)
        {
            temp = temp->next;
            i++;
        }

        if (!temp)
            throw out_of_range("Graph[] index out of range");

        return *temp;
    }

    // =========================================================================================================================
    //                                        ALGORITHM
    // =========================================================================================================================
    // private:
    // Convert "22/12/2024" and "09:00" → minutes since epoch

public: // inside Graph class
    vector<PathStep> dijkstraPath(const string &srcName,
                                  const string &destName,
                                  const string &startDateStr)
    {
        PortNode *src = findVertex_Lower(srcName);
        PortNode *dest = findVertex_Lower(destName);
        if (!src || !dest)
            return {};

        long long startMinutes = toMinutes(startDateStr, "00:00");

        struct NodeState
        {
            PortNode *node;
            long long arrivalTime; // absolute minutes
            int cost;
            vector<PathStep> steps;
        };

        auto cmp = [](const NodeState &a, const NodeState &b)
        {
            return a.cost > b.cost; // prioritize lower cost
        };

        priority_queue<NodeState, vector<NodeState>, decltype(cmp)> pq(cmp);
        pq.push({src, startMinutes, 0, {}});

        // Track visited states: node + arrival time to avoid revisiting worse paths
        unordered_map<PortNode *, set<long long>> visitedTimes;

        while (!pq.empty())
        {
            NodeState cur = pq.top();
            pq.pop();

            // Skip if this node + arrival time is already visited
            if (visitedTimes[cur.node].count(cur.arrivalTime))
                continue;
            visitedTimes[cur.node].insert(cur.arrivalTime);

            // Reached destination
            if (cur.node == dest)
                return cur.steps;

            // Explore all outgoing edges
            for (Edge *e = cur.node->head; e; e = e->next)
            {
                PortNode *v = findVertex_Lower(e->dest);
                if (!v)
                    continue;

                // Absolute departure and arrival times
                long long departTime = toMinutes(e->date, e->dep);
                long long arriveTime = toMinutes(e->date, e->arr);

                // Overnight adjustment
                if (arriveTime < departTime)
                    arriveTime += 1440; // next day

                // Ensure we cannot depart before we arrive at current node
                if (departTime < cur.arrivalTime)
                    continue;

                // Skip if departure is before journey start
                if (departTime < startMinutes)
                    continue;

                // Calculate waiting step (>12h)
                long long waitMinutes = departTime - cur.arrivalTime;
                bool addWaitStep = (waitMinutes > 12 * 60);
                float waitCost = addWaitStep ? cur.node->portData.charges : 0;

                NodeState next;
                next.node = v;
                next.arrivalTime = arriveTime;
                next.cost = cur.cost + e->cost + (addWaitStep ? waitCost : 0);
                next.steps = cur.steps;

                // Add waiting step if needed
                if (addWaitStep)
                {
                    PathStep ws{};
                    ws.from = cur.node;
                    ws.to = nullptr;
                    ws.edge = nullptr;
                    ws.isWaiting = true;
                    ws.waitingCost = waitCost;
                    ws.travelCost = 0;
                    ws.departTime = cur.arrivalTime;
                    ws.arriveTime = departTime;
                    next.steps.push_back(ws);
                }

                // Add travel step
                PathStep ts{};
                ts.from = cur.node;
                ts.to = v;
                ts.edge = e;
                ts.isWaiting = false;
                ts.waitingCost = 0;
                ts.travelCost = e->cost;
                ts.departTime = departTime;
                ts.arriveTime = arriveTime;
                next.steps.push_back(ts);

                // Push next state into the priority queue
                pq.push(next);
            }
        }

        // No feasible path found
        return {};
    }

    // Function to find all feasible paths from srcName to destName starting at a given date/time
    vector<vector<PathStep>> allValidPaths(const string &srcName,
                                           const string &destName,
                                           const string &startDateStr)
    {
        // Find source and destination nodes in the graph
        PortNode *src = findVertex_Lower(srcName);
        PortNode *dest = findVertex_Lower(destName);

        // If either source or destination does not exist, return empty result
        if (!src || !dest)
            return {};

        // Convert journey start date to minutes since epoch (or some reference)
        long long startMinutes = toMinutes(startDateStr, "00:00");

        // This will store all valid paths we discover
        vector<vector<PathStep>> result;

        // Recursive DFS function to explore all valid paths
        // u: current node
        // arrivalAtU: absolute time (minutes) when we arrive at node u
        // path: the path accumulated so far
        function<void(PortNode *, long long, vector<PathStep>)> dfs;
        dfs = [&](PortNode *u, long long arrivalAtU, vector<PathStep> path)
        {
            // If we reached the destination, store the current path
            if (u == dest)
            {
                result.push_back(std::move(path)); // move avoids copying the vector
                return;
            }

            // Safety limits: avoid infinite recursion or too large paths
            if (path.size() > 100 || result.size() > 5000)
                return;

            // Iterate over all outgoing edges (flights, ships, or connections) from current node
            for (Edge *e = u->head; e; e = e->next)
            {
                PortNode *v = findVertex_Lower(e->dest); // find the destination node of this edge
                if (!v)
                    continue; // skip if destination node does not exist

                // Compute absolute departure and arrival times for this edge
                // Convert edge departure and arrival times to absolute minutes
                long long departTime = toMinutes(e->date, e->dep);
                long long arriveTime = toMinutes(e->date, e->arr);

                // Handle overnight journeys (arrival is on next day)
                if (arriveTime < departTime)
                    arriveTime += 1440; // 1440 minutes = 24 hours

                // IMPORTANT RULE: cannot depart before we arrive at current node
                if (departTime < arrivalAtU)
                    continue; // skip infeasible time travel

                // Optional: skip edges before journey start time
                if (departTime < startMinutes)
                    continue;

                // Compute waiting time at current node
                long long waitMinutes = departTime - arrivalAtU;

                // Determine if a "waiting step" should be added (>12 hours)
                bool addWaitStep = (waitMinutes > 12 * 60);
                float waitCost = addWaitStep ? u->portData.charges : 0.0f;

                // Prepare a new path vector for the recursive step
                vector<PathStep> newPath = path;
                newPath.reserve(path.size() + 2); // reserve space to avoid repeated allocations

                // If we need to wait >12 hours, add a waiting step
                if (addWaitStep)
                {
                    PathStep wait{};
                    wait.from = u;         // waiting starts at current node
                    wait.to = nullptr;     // no destination node
                    wait.edge = nullptr;   // no edge
                    wait.isWaiting = true; // mark as waiting
                    wait.waitingCost = waitCost;
                    wait.travelCost = 0;
                    wait.departTime = arrivalAtU; // waiting starts when we arrive
                    wait.arriveTime = departTime; // waiting ends at departure
                    newPath.push_back(wait);
                }

                // Add the actual travel step along this edge
                PathStep travel{};
                travel.from = u; // starting node
                travel.to = v;   // destination node
                travel.edge = e; // the edge being used
                travel.isWaiting = false;
                travel.waitingCost = 0;
                travel.travelCost = e->cost;
                travel.departTime = departTime;
                travel.arriveTime = arriveTime;
                newPath.push_back(travel);

                // Recurse: explore paths from the destination node of this edge
                dfs(v, arriveTime, std::move(newPath));
            }

            // No need for explicit backtracking since we pass a copy of path to each recursive call
        };

        // Start DFS from the source node
        dfs(src, startMinutes, {});

        // Return all discovered valid paths
        return result;
    }

    vector<PathStep> dijkstraFastestPath(const string &srcName,
                                         const string &destName,
                                         const string &startDateStr)
    {
        PortNode *src = findVertex_Lower(srcName);
        PortNode *dest = findVertex_Lower(destName);
        if (!src || !dest)
            return {};

        long long startMinutes = toMinutes(startDateStr, "00:00");

        struct NodeState
        {
            PortNode *node;
            long long timeFromStart; // total elapsed minutes since start
            long long arrivalTime;   // absolute minutes since epoch
            int cost;                // total cost including waiting fees
            vector<PathStep> steps;
        };

        auto cmp = [](const NodeState &a, const NodeState &b)
        {
            return a.timeFromStart > b.timeFromStart; // prioritize fastest elapsed time
        };

        priority_queue<NodeState, vector<NodeState>, decltype(cmp)> pq(cmp);
        pq.push({src, 0, startMinutes, 0, {}});

        // track fastest time to reach a node (avoid dominated paths)
        unordered_map<PortNode *, long long> bestTime;

        while (!pq.empty())
        {
            NodeState cur = pq.top();
            pq.pop();

            // Skip if a better path already reached this node
            if (bestTime.count(cur.node) && cur.timeFromStart >= bestTime[cur.node])
                continue;

            bestTime[cur.node] = cur.timeFromStart;

            // Destination reached
            if (cur.node == dest)
                return cur.steps;

            for (Edge *e = cur.node->head; e; e = e->next)
            {
                PortNode *v = findVertex_Lower(e->dest);
                if (!v)
                    continue;

                long long departTime = toMinutes(e->date, e->dep);
                long long arriveTime = toMinutes(e->date, e->arr);
                if (arriveTime < departTime)
                    arriveTime += 1440; // overnight travel

                // Reject departures that are **before current node arrival**
                if (departTime < cur.arrivalTime)
                    continue;

                long long waitMinutes = departTime - cur.arrivalTime;
                bool addWait = false;
                float waitCost = 0;

                if (waitMinutes > 12 * 60)
                { // >12h waiting triggers port charges
                    addWait = true;
                    waitCost = cur.node->portData.charges;
                }

                NodeState next;
                next.node = v;
                next.arrivalTime = arriveTime;
                next.timeFromStart = cur.timeFromStart + waitMinutes + (arriveTime - departTime);
                next.cost = cur.cost + e->cost + (addWait ? waitCost : 0);
                next.steps = cur.steps;

                // Add waiting step if needed
                if (addWait)
                {
                    PathStep ws;
                    ws.from = cur.node;
                    ws.to = nullptr;
                    ws.edge = nullptr;
                    ws.isWaiting = true;
                    ws.waitingCost = waitCost;
                    ws.travelCost = 0;
                    ws.departTime = cur.arrivalTime;
                    ws.arriveTime = departTime;
                    next.steps.push_back(ws);
                }

                // Add travel step
                PathStep ts;
                ts.from = cur.node;
                ts.to = v;
                ts.edge = e;
                ts.isWaiting = false;
                ts.waitingCost = 0;
                ts.travelCost = e->cost;
                ts.departTime = departTime;
                ts.arriveTime = arriveTime;
                next.steps.push_back(ts);

                pq.push(next);
            }
        }

        return {}; // no feasible path
    }

    // Wrapper function to get filtered paths based on user preferences
    vector<vector<PathStep>> filteredPaths(
        const string &srcName,
        const string &destName,
        const string &startDateStr,
        const vector<string> &preferredShippingCompanies, // e.g., {"Maersk Line", "YangMing"}
        const vector<string> &avoidPorts,                 // e.g., {"Hamburg", "Rotterdam"}
        long long maxVoyageMinutes                        // maximum voyage time in minutes
    )
    {
        // 1️⃣ Get all possible paths
        vector<vector<PathStep>> allPaths = allValidPaths(srcName, destName, startDateStr);

        vector<vector<PathStep>> filtered;

        for (auto &path : allPaths)
        {
            bool valid = true;
            long long totalVoyage = 0;

            for (auto &step : path)
            {
                // Skip waiting steps for company/port checks
                if (step.isWaiting)
                    continue;

                // 2️⃣ Check preferred shipping companies
                if (!preferredShippingCompanies.empty())
                {
                    string company = step.edge->company; // assuming Edge has shipCompany field
                    if (find(preferredShippingCompanies.begin(), preferredShippingCompanies.end(), company) == preferredShippingCompanies.end())
                    {
                        valid = false;
                        break;
                    }
                }

                // 3️⃣ Check ports to avoid
                if (!avoidPorts.empty())
                {
                    string fromPort = step.from->name;
                    string toPort = step.to->name;
                    if (find(avoidPorts.begin(), avoidPorts.end(), fromPort) != avoidPorts.end() ||
                        find(avoidPorts.begin(), avoidPorts.end(), toPort) != avoidPorts.end())
                    {
                        valid = false;
                        break;
                    }
                }

                // 4️⃣ Calculate total voyage time
                totalVoyage += step.arriveTime - step.departTime;
            }

            // 5️⃣ Check maximum voyage time
            if (valid && (maxVoyageMinutes <= 0 || totalVoyage <= maxVoyageMinutes))
            {
                filtered.push_back(path);
            }
        }

        return filtered;
    }
};
