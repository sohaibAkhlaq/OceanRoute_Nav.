#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <unordered_map>
#include <queue>
#include <vector>
#include <limits>
#include "Helper.h"
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
public:
public: // inside Graph class
    // Inside Graph class
    pair<vector<pair<PortNode *, Edge *>>, vector<PortNode *>> dijkstraPath(const string &srcName, const string &destName)
    {
        PortNode *src = findVertex_Lower(srcName);
        PortNode *dest = findVertex_Lower(destName);

        if (!src || !dest)
            return {}; // source or dest not found

        unordered_map<PortNode *, int> dist;
        unordered_map<PortNode *, PortNode *> prevNode;
        unordered_map<PortNode *, Edge *> prevEdge;

        for (PortNode *v = vertices; v != nullptr; v = v->next)
            dist[v] = numeric_limits<int>::max();

        dist[src] = 0;

        auto cmp = [](const pair<int, PortNode *> &a, const pair<int, PortNode *> &b)
        { return a.first > b.first; };
        priority_queue<pair<int, PortNode *>, vector<pair<int, PortNode *>>, decltype(cmp)> pq(cmp);

        pq.push({0, src});

        while (!pq.empty())
        {
            auto [d, u] = pq.top();
            pq.pop();

            if (d > dist[u])
                continue;

            Edge *e = u->head;
            while (e)
            {
                PortNode *v = findVertex_Lower(e->dest);
                if (!v)
                {
                    e = e->next;
                    continue;
                }

                int newDist = dist[u] + e->cost;
                if (newDist < dist[v])
                {
                    dist[v] = newDist;
                    prevNode[v] = u;
                    prevEdge[v] = e;
                    pq.push({newDist, v});
                }
                e = e->next;
            }
        }

        // Reconstruct path
        vector<PortNode *> pathNodes;
        vector<pair<PortNode *, Edge *>> pathEdges; // store source + edge

        if (dist[dest] == numeric_limits<int>::max())
            return {pathEdges, pathNodes}; // no path

        // walk backwards
        for (PortNode *at = dest; at != nullptr; at = prevNode[at])
            pathNodes.push_back(at);
        reverse(pathNodes.begin(), pathNodes.end());

        // Build edge list with source node
        for (size_t i = 1; i < pathNodes.size(); i++)
        {
            PortNode *from = pathNodes[i - 1];
            PortNode *to = pathNodes[i];
            Edge *e = prevEdge[to];
            pathEdges.push_back({from, e});
        }

        return {pathEdges, pathNodes};
    }
};
