#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <unordered_map>
#include <queue>
#include <vector>
#include <functional>
#include <limits>
#include "Helper.h"
using namespace std;

long long toMinutes(const string &date, const string &time)
{
    int d, m, y, hh, mm;
    char c1, c2, c3;

    stringstream ss(date + " " + time);
    ss >> d >> c1 >> m >> c2 >> y >> hh >> c3 >> mm;

    tm t = {};
    t.tm_mday = d;
    t.tm_mon = m - 1;
    t.tm_year = y - 1900;
    t.tm_hour = hh;
    t.tm_min = mm;

    return mktime(&t) / 60;
}

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
        // Inside Graph class
    pair<vector<pair<PortNode *, Edge *>>, vector<PortNode *>>
    dijkstraPath(const string &srcName, const string &destName, const string &startDateStr)
    {
        cout << "Called dijkstraPath()\n";

        PortNode *src = findVertex_Lower(srcName);
        PortNode *dest = findVertex_Lower(destName);

        if (!src || !dest)
            return {};

        long long startDateMinutes = toMinutes(startDateStr, "00:00");
        // Convert user start date → minutes since epoch

        struct NodeState
        {
            PortNode *node;
            int cost;
            long long arrivalTime;
            vector<PortNode *> pathNodes;
            vector<pair<PortNode *, Edge *>> pathEdges;
        };

        auto cmp = [](const NodeState &a, const NodeState &b)
        {
            return a.cost > b.cost;
        };

        priority_queue<NodeState, vector<NodeState>, decltype(cmp)> pq(cmp);

        pq.push({src, 0, startDateMinutes, {src}, {}});

        NodeState bestPath;
        bool found = false;
        unordered_map<PortNode *, int> bestCost;

        while (!pq.empty())
        {
            NodeState cur = pq.top();
            pq.pop();

            if (bestCost.count(cur.node) && cur.cost > bestCost[cur.node])
                continue;

            bestCost[cur.node] = cur.cost;

            if (cur.node == dest)
            {
                bestPath = cur;
                found = true;
                break;
            }

            for (Edge *e = cur.node->head; e != nullptr; e = e->next)
            {
                PortNode *v = findVertex_Lower(e->dest);
                if (!v)
                    continue;

                long long departTime = toMinutes(e->date, e->dep);
                long long arrivalTime = toMinutes(e->date, e->arr);

                // REJECT if edge is before user-specified date
                if (departTime < startDateMinutes)
                    continue;

                // REJECT if departure is before the current arrival
                if (departTime < cur.arrivalTime)
                    continue;

                NodeState nextState;
                nextState.node = v;
                nextState.cost = cur.cost + e->cost;
                nextState.arrivalTime = arrivalTime;
                nextState.pathNodes = cur.pathNodes;
                nextState.pathNodes.push_back(v);
                nextState.pathEdges = cur.pathEdges;
                nextState.pathEdges.push_back({cur.node, e});

                pq.push(nextState);
            }
        }

        if (!found)
            return {};

        return {bestPath.pathEdges, bestPath.pathNodes};
    }

    vector<pair<
        vector<pair<PortNode *, Edge *>>,
        vector<PortNode *>>>
    allValidPaths(const string &srcName, const string &destName, const string &startDateStr)
    {
        cout << "Called allValidPaths()\n";

        PortNode *src = findVertex_Lower(srcName);
        PortNode *dest = findVertex_Lower(destName);

        vector<
            pair<
                vector<pair<PortNode *, Edge *>>,
                vector<PortNode *>>>
            result;

        if (!src || !dest)
            return result;

        long long startDateMinutes = toMinutes(startDateStr, "00:00");

        unordered_map<PortNode *, bool> visited;

        function<void(PortNode *, long long,
                      vector<PortNode *> &,
                      vector<pair<PortNode *, Edge *>> &)>
            dfs;

        dfs = [&](PortNode *u, long long lastArrival,
                  vector<PortNode *> &pathNodes,
                  vector<pair<PortNode *, Edge *>> &pathEdges)
        {
            if (u == dest)
            {
                result.push_back({pathEdges, pathNodes});
                return;
            }

            visited[u] = true;

            for (Edge *e = u->head; e != nullptr; e = e->next)
            {
                PortNode *v = findVertex_Lower(e->dest);
                if (!v || visited[v])
                    continue;

                long long departTime = toMinutes(e->date, e->dep);
                long long arrivalTime = toMinutes(e->date, e->arr);

                // Reject edges earlier than user start date
                if (departTime < startDateMinutes)
                    continue;

                // Reject edges earlier than previous arrival
                if (departTime < lastArrival)
                    continue;

                pathNodes.push_back(v);
                pathEdges.push_back({u, e});

                dfs(v, arrivalTime, pathNodes, pathEdges);

                pathNodes.pop_back();
                pathEdges.pop_back();
            }

            visited[u] = false;
        };

        vector<PortNode *> pathNodes;
        vector<pair<PortNode *, Edge *>> pathEdges;

        pathNodes.push_back(src);

        dfs(src, startDateMinutes, pathNodes, pathEdges);

        return result;
    }
};
