#include "compare_node.h"
#include "story_project.h"
#include "connection.h"
#include "sys_lib.h"
#include "compiler.h"


CompareNode::CompareNode(const std::string &type)
    : BaseNode(type, "Branch Node")
{

}


void CompareNode::Initialize()
{

}

std::string CompareNode::GenerateConstants(IStoryPage &page, IStoryProject &project, int nb_out_conns)
{
    std::string s;

   

    return s;
}

std::string CompareNode::Build(IStoryPage &page, const StoryOptions &options, int nb_out_conns)
{
    std::stringstream ss;

    std::list<std::shared_ptr<Connection>> conns;
    page.GetNodeConnections(conns, GetId());
    int i = 0;
    std::list<std::shared_ptr<Connection>>::iterator c = conns.begin();

/*

; Déclaration des variables en RAM
$var1    DV32    1    ; Première variable à comparer
$var2    DV32    1    ; Deuxième variable à comparer

; Code principal
.compare_ge:
    ; Charger les valeurs des variables dans les registres
    lcons r1, $var1
    load r1, @r1, 4   ; Charger 4 bytes (32 bits) de var1 dans r1
    lcons r2, $var2
    load r2, @r2, 4   ; Charger 4 bytes (32 bits) de var2 dans r2

    ; Comparer r1 >= r2
    gt r3, r1, r2     ; r3 = 1 si r1 > r2, sinon 0
    eq r4, r1, r2     ; r4 = 1 si r1 == r2, sinon 0
    or r0, r3, r4     ; r0 = 1 si r1 > r2 OU r1 == r2, sinon 0

    ret



 */


    if (conns.size() == 2)
    {
        ss << R"(; ---------------------------- )"
        << GetTitle()
        << " Type: Branch"
        << "\n";
        
        ss << "eq  r0, r0, r1\n"
           << "skipz r0\n"
           << "jump " << BaseNode::GetEntryLabel((*c)->inNodeId);
        ++c;
        ss << "jump " << BaseNode::GetEntryLabel((*c)->inNodeId);
    }
    return ss.str();
}

