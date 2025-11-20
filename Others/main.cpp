#include <iostream>
#include <fstream>
#include <iomanip>
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

class Vertex
{
public:
    string name;
    Edge *head;
    Vertex *next;

    Vertex(string n)
    {
        name = n;
        head = nullptr;
        next = nullptr;
    }
};

class Graph
{
private:
    Vertex *vertices;

    Vertex *findVertex(string name)
    {
        Vertex *temp = vertices;
        while (temp != nullptr)
        {
            if (temp->name == name)
                return temp;
            temp = temp->next;
        }
        return nullptr;
    }

    Vertex *addVertex(string name)
    {
        Vertex *v = findVertex(name);
        if (v != nullptr)
            return v;
        Vertex *nv = new Vertex(name);
        nv->next = vertices;
        vertices = nv;
        return nv;
    }

    void addEdge(string src, string dest, string date, string dep, string arr, int cost, string company)
    {
        Vertex *v = addVertex(src);
        addVertex(dest);

        Edge *e = new Edge(dest, date, dep, arr, cost, company);
        e->next = v->head;
        v->head = e;
    }

    void sortEdges(Vertex *v)
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
            Vertex *curr = vertices;
            Vertex *prev = nullptr;

            while (curr->next != nullptr)
            {
                if (curr->name > curr->next->name)
                {
                    Vertex *temp = curr->next;
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

public:
    Graph(string filename)
    {
        vertices = nullptr;
        loadRoutes(filename);
        sortVertices();

        Vertex *v = vertices;
        while (v != nullptr)
        {
            sortEdges(v);
            v = v->next;
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

    void printGraph(bool vertices_only = false)
    {
        if (vertices_only)
        {
            Vertex *v = vertices;

            int i = 1;
            while (v != nullptr)
            {
                cout << i++ << " " << colorText(v->name, BLUE) << "\n";

                v = v->next;
            }
        }
        else
        {

            Vertex *v = vertices;

            while (v != nullptr)
            {
                cout << "[" << colorText(v->name, BLUE) << "] ->\n";

                Edge *e = v->head;

                while (e != nullptr)
                {
                    cout
                        << "    "
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
};

int main()
{
    Graph g("Routes.txt");
    g.printGraph(true);
    return 0;
}
