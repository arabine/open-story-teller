#ifndef GRAPH_H
#define GRAPH_H


#include <utility>
#include <climits>
#include <string>
#include <vector>
#include <list>
#include <algorithm>

class  Graph
{
static const int DEFAULT_MAX_SIZE = 1000;
static const int INF = INT_MAX;

public:
    class Vertex
    {
    public:
        Vertex()
        {
            mValid = false;
        }

        ~Vertex()
        {

        }

        int first_edge_gap()
        {
            std::vector<int> vec;

            for (auto &p : edges)
            {
                vec.push_back(p.first);
            }

            // Handle the special case of an empty vector.  Return 1.
            if( vec.empty() )
            {
                return 1;
            }

            // Sort the vector
            std::sort( vec.begin(), vec.end() );

            // Find the first adjacent pair that differ by more than 1.
            auto i = std::adjacent_find( vec.begin(), vec.end(), [](int l, int r){return l+1<r;} );

            // Handle the special case of no gaps.  Return the last value + 1.
            if ( i == vec.end() )
            {
                --i;
            }

            return 1 + *i;
        }

        bool isValid() const { return mValid; }
        void setId(const std::string& newId)
            { mId = newId; mValid = true; }
        std::string getId() const
            { return mId; }


        void removeEdge(const std::string &id);

        // poids / vertex
        typedef std::pair<int, Vertex> vPair;
        std::list<vPair> edges;
        Vertex& operator=(const Vertex& other)
        {
            mId = other.mId;
            edges = other.edges;
            mValid = other.mValid;
            return *this;
        }

    private:
        std::string mId{"invalid"};
        bool mValid{false};
    };

    Graph(int newMaxSize = DEFAULT_MAX_SIZE);
    Graph(const Graph& other);
    Graph& operator=(const Graph& other);
    ~Graph();

    typedef std::list<Vertex>::iterator Iter;

    std::list<Vertex>::iterator begin() { return vertexList.begin(); }
    std::list<Vertex>::const_iterator begin() const { return vertexList.begin(); }
    std::list<Vertex>::iterator end() { return vertexList.end(); }
    std::list<Vertex>::const_iterator end() const { return vertexList.end(); }

    bool addVertex(const std::string& id);

    // If defaut weight is -1, then auto attribute the weight
    void insertEdge(const std::string& v1, const std::string& v2, int wt = -1);

    Vertex first() const;
    Vertex getVertex(const std::string& id) const;
    Vertex getPreviousVertex(const std::string &id);
    Vertex getNextVertex(const std::string &id) const;

    bool searchVertex(const std::string& v, Vertex& returnVertex) const;
//    bool searchEdge(const std::string& v1, const std::string& v2,
//                    int& wt) const;
    bool removeVertex(const std::string& v1);
    bool removeEdge(const std::string& v1, const std::string& v2);
    bool clearExtraEdges(const std::string& v1);

    void clear();

    bool empty() const;
    bool full() const;

    void showGraph() const;

//    void bfs(const std::string& src,
//             const std::string& dest) const;
//    void shortestPaths(const std::string& src) const;

private:
    int getIndex(const std::string& v) const;
    Vertex mInvalid;
    std::list<Vertex> vertexList;
    int maxSize;
};


#endif // GRAPH_H
