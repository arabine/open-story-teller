


#include <iostream>
#include <queue>
#include "graph.h"

using namespace std;



Graph::Graph(int newMaxSize)
    : mInvalid()
{
    maxSize = newMaxSize;
//    vertexList = new Vertex [maxSize];
}


Graph::Graph(const Graph& other)
{
    *this = other;
}


Graph& Graph::operator=(const Graph& other)
{
    if (this == &other)
    {
        return *this;
    }

    if (!empty())
    {
        vertexList.clear();
    }

    maxSize = other.maxSize;
    vertexList.assign(other.vertexList.begin(), other.vertexList.end());

    return *this;
}


Graph::~Graph()
{
//    delete[] vertexList;
    vertexList.clear();
}


bool Graph::addVertex(const string& id)
{
    if (full())
    {
        return false;
    }

    Vertex newVertex;
    newVertex.setId(id);
    vertexList.push_back(newVertex);

    return true;
}


void Graph::insertEdge(const string& v1, const string& v2, int wt)
{
    std::list<Vertex>::iterator it_v1 = vertexList.end();
    std::list<Vertex>::iterator it_v2 = vertexList.end();
    for (auto it = vertexList.begin(); it != vertexList.end(); ++it)
    {
        if (it->getId() == v1)
        {
            it_v1 = it;
        }
        if (it->getId() == v2)
        {
            it_v2 = it;
        }
    }

    if ((it_v1 != vertexList.end()) && (it_v2 != vertexList.end()))
    {
        if (wt == -1)
        {
            wt = it_v1->first_edge_gap();
        }
        it_v1->edges.push_back(make_pair(wt, *it_v2));
    }
    else
    {
        std::cout << "Cannot create edge, one vertex is not found" << std::endl;
    }
}

Graph::Vertex Graph::first() const
{
    if (vertexList.size() > 0) {
        return vertexList.front();
    } else {
        return Vertex();
    }
}

Graph::Vertex Graph::getVertex(const string &v) const
{
    for (auto it = vertexList.begin(); it != vertexList.end(); ++it)
    {
        if (it->getId() == v)
        {
            return *it;
        }
    }
    return mInvalid;
}

Graph::Vertex Graph::getPreviousVertex(const string &id)
{
    for (auto it = vertexList.begin(); it != vertexList.end(); ++it)
    {
        if (it->edges.size() > 0)
        {
            Vertex v = it->edges.front().second;
            if (v.getId() == id)
            {
                return *it;
            }
        }
    }
    return mInvalid;
}

Graph::Vertex Graph::getNextVertex(const std::string &id) const
{
    for (auto it = vertexList.begin(); it != vertexList.end(); ++it)
    {
        if ((it->getId() == id))
        {
            if (it->edges.size() > 0)
            {
                return it->edges.front().second;
            }
        }
    }
    return mInvalid;
}


void Graph::Vertex::removeEdge(const std::string &id)
{
    edges.remove_if([id](const vPair &n){ return (n.second.getId() == id); });
}

bool Graph::removeVertex(const string& v)
{
    std::list<Vertex>::iterator it_v = vertexList.end();

    // On cherche le vertex
    for (auto it = vertexList.begin(); it != vertexList.end(); ++it)
    {
        if (it->getId() == v)
        {
            it_v = it;
        }
    }

    // vertex non trouvé
    if (it_v == vertexList.end())
    {
        return false;
    }

    std::cout << "RM: " << it_v->getId()<< std::endl;
    vertexList.erase(it_v);

    // On supprime toutes les références à ce vertex dans les autres edges
    // == On supprime les relations éventuelles
    for (auto it = vertexList.begin(); it != vertexList.end(); ++it)
    {
        it->removeEdge(v);
    }

    return true;
}


bool Graph::removeEdge(const string& v1, const string& v2)
{
    bool success = false;
    for (auto it = vertexList.begin(); it != vertexList.end(); ++it)
    {
        if (it->getId() == v1)
        {
            success = true;
            it->removeEdge(v2);
            break;
        }
    }

    return success;
}

bool Graph::clearExtraEdges(const std::string &v1)
{
    bool success = false;
    for (auto it = vertexList.begin(); it != vertexList.end(); ++it)
    {
        if (it->getId() == v1)
        {
            success = true;
            it->edges.remove_if([](const Vertex::vPair &n){ return (n.first != 0); });
            break;
        }
    }

    return success;
}


void Graph::clear()
{
    vertexList.clear();
}


bool Graph::empty() const
{
    return (vertexList.size() == 0);
}


bool Graph::full() const
{
    return (vertexList.size() == maxSize);
}


void Graph::showGraph() const
{
    if (empty())
    {
        cout << "Empty Graph" << endl;
        return;
    }

    cout << "Vertex and their adjacency list:" << endl;
    for (auto it = vertexList.begin(); it != vertexList.end(); ++it)
    {
        cout << '\t' << it->getId();

        for (const auto &j : it->edges)
        {
            cout << " --> [" << j.second.getId()
                 << ", " << j.first << ']';
        }
        cout << endl;
    }
}
/*

void Graph::bfs(const string& src, const string& dest) const
{
    int src_idx = getIndex(src),
        dest_idx = getIndex(dest);

    if (src_idx == -1 ||
        dest_idx == -1)
        return;

    queue<int> q;
    vector<bool> visited(size, false);

    q.push(src_idx);
    visited[src_idx] = true;

    while (!q.empty())
    {
        int u = q.front();
        q.pop();

        cout << ' ' << vertexList[u].getId() << endl;

        if (u == dest_idx)
            return;

        for (auto i : vertexList[u].edges)
        {
            int v = getIndex(i.second.getId());

            if (!visited[v])
            {
                visited[v] = true;
                q.push(v);
            }
        }
    }
}


void Graph::shortestPaths(const string& src) const
{
    typedef pair<int,int> vtx;
    priority_queue<vtx,
                   vector<vtx>,
                   greater<vtx> > pq;

    vector<int> dist(size, INF);

    int src_idx = getIndex(src);
    if (src_idx == -1) return;

    dist[src_idx] = 0;
    pq.push(make_pair(0, src_idx));

    while (!pq.empty())
    {
        int u = pq.top().second;
        pq.pop();

        for (auto i : vertexList[u].edges)
        {
            int v = getIndex(i.second.getId());
            int wt = i.first;

            if (dist[v] > dist[u] + wt)
            {
                dist[v] = dist[u] + wt;
                pq.push(make_pair(dist[v], v));
            }
        }
    }

    for (int i = 0; i < size; i++)
    {
        cout << '\t' << vertexList[i].getId();
        if (dist[i] == INF)
            cout << '\t' << '-' << endl;
        else
            cout << '\t' << dist[i] << endl;
    }
    cout << endl;
}


int Graph::getIndex(const string& v) const
{
    for (int i = 0; i < size; i++)
        if (v == vertexList[i].getId())
            return i;

    return -1;
}
*/

