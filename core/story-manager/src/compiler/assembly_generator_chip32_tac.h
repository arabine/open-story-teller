// ===================================================================
// assembly_generator_chip32_tac.h
// IMPLÉMENTATION COMPLÈTE avec tous les types gérés
// ===================================================================

#pragma once

#include "assembly_generator.h"
#include "tac.h"
#include "print_node.h"
#include "operator_node.h"
#include "variable_node.h"
#include "branch_node.h"
#include "function_entry_node.h"
#include "call_function_node.h"
#include <algorithm>
#include <set>

class AssemblyGeneratorChip32TAC : public AssemblyGenerator
{
public:
    AssemblyGeneratorChip32TAC(const GeneratorContext& context)
        : AssemblyGenerator(context)
        , m_currentContext(FunctionContext::MAIN_PROGRAM)
    {
    }
    
    virtual ~AssemblyGeneratorChip32TAC() = default;

    // ===================================================================
    // WORKFLOW COMPLET : Cette méthode orchestre tout
    // ===================================================================
    void GenerateCompleteProgram(const std::vector<std::shared_ptr<BaseNode>>& nodes,
                                 const std::vector<std::shared_ptr<ASTNode>>& astNodes)
    {
        Reset();
        
        // === ÉTAPE 1 : HEADER ===
        GenerateHeader();
        
        // === ÉTAPE 2 : SECTION DATA ===
        StartSection(Section::DATA);

        // Générer les variables globales
        GenerateGlobalVariables();
        
        // Générer les variables des nœuds (format strings, etc.)
        GenerateNodesVariables(nodes);
        
        // === ÉTAPE 3 : GÉNÉRATION TAC ===
        m_tacGenerator = std::make_unique<TACGenerator>();
        m_tacProgram = m_tacGenerator->Generate(astNodes);
        
        // DEBUG : Afficher le TAC généré
        if (m_context.debugOutput) {
            std::cout << "\n" << m_tacProgram.ToString() << std::endl;
        }
        
        // Générer les temporaires TAC qui seront en RAM
        GenerateTACTemporaries();
        
        m_assembly << "\n";
        
        // === ÉTAPE 4 : SECTION TEXT (à partir du TAC) ===
        StartSection(Section::TEXT);
        GenerateFromTAC();
        
        // === ÉTAPE 5 : EXIT ===
        GenerateExit();
    }


        virtual void GenerateMain() override {
        m_assembly << ".main:\n";
    }
    
    virtual void AddComment(const std::string& comment) override {
        m_assembly << std::string(m_depth * 4, ' ') << "; " << comment << "\n";
    }
    
    virtual void GenerateExit() override {
        AddComment("Program exit");
        m_assembly << "    halt\n";
    }
    
    // Note: Cette méthode ne sera PAS utilisée car on génère depuis le TAC
    virtual void GenerateNodeCode(std::shared_ptr<ASTNode> node, bool isDataPath = false) override {
        // Cette méthode n'est plus utilisée avec l'approche TAC
        // Tout est géré via GenerateFromTAC()
        
        // On peut laisser une implémentation vide ou lancer une exception
        if (m_context.debugOutput) {
            AddComment("WARNING: GenerateNodeCode called but not used in TAC mode");
        }
    }

    void GenerateTACToAssembly(const TACProgram& tac) {
        m_tacProgram = tac;
        
        // Générer les temporaires si nécessaire
        GenerateTACTemporaries();
        
        // Convertir le TAC en assembleur
        for (const auto& instr : m_tacProgram.GetInstructions()) {
            GenerateTACInstruction(instr);
        }
    }
    
    // Exposer l'assembly stream pour écrire directement
    std::stringstream& GetAssembly() { return m_assembly; }

private:
    enum class FunctionContext {
        MAIN_PROGRAM,
        SUB_FUNCTION
    };
    
    FunctionContext m_currentContext;
    std::unique_ptr<TACGenerator> m_tacGenerator;
    TACProgram m_tacProgram;
    
    // Map des temporaires TAC vers leur localisation
    struct TempLocation {
        enum Type { REGISTER, MEMORY };
        Type type;
        std::string location;  // "t0" ou "$temp_label"
    };
    std::map<std::string, TempLocation> m_tempLocations;
    int m_nextTempReg = 0;      // Prochain registre t0-t9 disponible
    int m_tempVarCounter = 0;    // Compteur pour variables temporaires en RAM
    
    // ===================================================================
    // GÉNÉRATION DE LA SECTION DATA
    // ===================================================================
    
    void GenerateTACTemporaries() {
        // Analyser le TAC pour déterminer quels temporaires ont besoin de RAM
        std::set<std::string> allTemps;
        
        for (const auto& instr : m_tacProgram.GetInstructions()) {
            if (instr->GetDest() && 
                instr->GetDest()->GetType() == TACOperand::Type::TEMPORARY) {
                allTemps.insert(instr->GetDest()->GetValue());
            }
            if (instr->GetOp1() && 
                instr->GetOp1()->GetType() == TACOperand::Type::TEMPORARY) {
                allTemps.insert(instr->GetOp1()->GetValue());
            }
            if (instr->GetOp2() && 
                instr->GetOp2()->GetType() == TACOperand::Type::TEMPORARY) {
                allTemps.insert(instr->GetOp2()->GetValue());
            }
        }
        
        if (m_context.debugOutput && !allTemps.empty()) {
            AddComment("TAC Temporaries: " + std::to_string(allTemps.size()) + " total");
        }
        
        // Allouer les 10 premiers dans des registres, le reste en RAM
        int tempIndex = 0;
        for (const auto& temp : allTemps) {
            if (tempIndex < 10) {
                // Utiliser un registre t0-t9
                m_tempLocations[temp] = {
                    TempLocation::REGISTER,
                    "t" + std::to_string(tempIndex)
                };
                
                if (m_context.debugOutput) {
                    AddComment("  " + temp + " -> register t" + std::to_string(tempIndex));
                }
            } else {
                // Créer une variable en RAM
                std::string varLabel = "temp_" + std::to_string(m_tempVarCounter++);
                m_assembly << "$" << varLabel << " DV32, 0 ; TAC temporary " 
                          << temp << "\n";
                m_tempLocations[temp] = {
                    TempLocation::MEMORY,
                    varLabel
                };
                
                if (m_context.debugOutput) {
                    AddComment("  " + temp + " -> memory $" + varLabel);
                }
            }
            tempIndex++;
        }
    }
    
    // ===================================================================
    // GÉNÉRATION DE LA SECTION TEXT À PARTIR DU TAC
    // ===================================================================
    
    void GenerateFromTAC() {
        for (const auto& instr : m_tacProgram.GetInstructions()) {
            GenerateTACInstruction(instr);
        }
    }
    
    void GenerateTACInstruction(std::shared_ptr<TACInstruction> instr) {
        if (m_context.debugOutput) {
            AddComment("TAC: " + instr->ToString());
        }
        
        switch (instr->GetOpCode()) {
            case TACInstruction::OpCode::ADD:
            case TACInstruction::OpCode::SUB:
            case TACInstruction::OpCode::MUL:
            case TACInstruction::OpCode::DIV:
                GenerateBinaryOp(instr);
                break;
                
            case TACInstruction::OpCode::EQ:
            case TACInstruction::OpCode::NE:
            case TACInstruction::OpCode::GT:
            case TACInstruction::OpCode::LT:
            case TACInstruction::OpCode::GE:
            case TACInstruction::OpCode::LE:
                GenerateComparison(instr);
                break;
                
            case TACInstruction::OpCode::COPY:
                GenerateCopy(instr);
                break;
                
            case TACInstruction::OpCode::LOAD:
                GenerateLoad(instr);
                break;
                
            case TACInstruction::OpCode::STORE:
                GenerateStore(instr);
                break;
                
            case TACInstruction::OpCode::LABEL:
                m_assembly << instr->GetDest()->ToString() << ":\n";
                break;
                
            case TACInstruction::OpCode::GOTO:
                m_assembly << "    jump " << instr->GetDest()->ToString() << "\n";
                break;
                
            case TACInstruction::OpCode::IF_FALSE:
                GenerateIfFalse(instr);
                break;
                
            case TACInstruction::OpCode::IF_TRUE:
                GenerateIfTrue(instr);
                break;
                
            case TACInstruction::OpCode::PARAM:
                GenerateParam(instr);
                break;
                
            case TACInstruction::OpCode::PRINT:
                GeneratePrint(instr);
                break;
                
            case TACInstruction::OpCode::CALL:
                GenerateCall(instr);
                break;
                
            case TACInstruction::OpCode::RETURN:
                m_assembly << "    ret\n";
                break;
                
            case TACInstruction::OpCode::NOP:
                m_assembly << "    nop\n";
                break;
                
            default:
                AddComment("WARNING: Unsupported TAC instruction");
        }
    }
    
    // ===================================================================
    // HELPERS : Charger une opérande dans un registre
    // ===================================================================
    
    std::string LoadOperand(std::shared_ptr<TACOperand> op, const std::string& targetReg) {
        if (!op) return targetReg;
        
        switch (op->GetType()) {
            case TACOperand::Type::CONSTANT:
                m_assembly << "    lcons " << targetReg << ", " << op->GetValue() << "\n";
                return targetReg;
                
            case TACOperand::Type::VARIABLE:
                m_assembly << "    load " << targetReg << ", $" << op->GetValue() << ", 4\n";
                return targetReg;
                
            case TACOperand::Type::TEMPORARY: {
                auto it = m_tempLocations.find(op->GetValue());
                if (it == m_tempLocations.end()) {
                    throw std::runtime_error("Temporary not found: " + op->GetValue());
                }
                
                if (it->second.type == TempLocation::REGISTER) {
                    // Déjà dans un registre, copier si nécessaire
                    if (it->second.location != targetReg) {
                        m_assembly << "    mov " << targetReg << ", " 
                                  << it->second.location << "\n";
                    }
                    return targetReg;
                } else {
                    // Charger depuis la RAM
                    m_assembly << "    load " << targetReg << ", $" 
                              << it->second.location << ", 4\n";
                    return targetReg;
                }
            }
            
            case TACOperand::Type::REGISTER:
                if (op->GetValue() != targetReg) {
                    m_assembly << "    mov " << targetReg << ", " 
                              << op->GetValue() << "\n";
                }
                return targetReg;
                
            case TACOperand::Type::LABEL:
                // Pour les labels, on utilise lcons avec l'adresse
                m_assembly << "    lcons " << targetReg << ", " 
                          << op->ToString() << "\n";
                return targetReg;
                
            default:
                return targetReg;
        }
    }
    
    void StoreResult(std::shared_ptr<TACOperand> dest, const std::string& sourceReg) {
        if (!dest) return;
        
        if (dest->GetType() == TACOperand::Type::TEMPORARY) {
            auto it = m_tempLocations.find(dest->GetValue());
            if (it == m_tempLocations.end()) {
                throw std::runtime_error("Temporary not found: " + dest->GetValue());
            }
            
            if (it->second.type == TempLocation::REGISTER) {
                // Stocker dans un registre
                if (it->second.location != sourceReg) {
                    m_assembly << "    mov " << it->second.location << ", " 
                              << sourceReg << "\n";
                }
            } else {
                // Stocker en RAM
                m_assembly << "    store $" << it->second.location << ", " 
                          << sourceReg << ", 4\n";
            }
        } else if (dest->GetType() == TACOperand::Type::VARIABLE) {
            m_assembly << "    store $" << dest->GetValue() << ", " 
                      << sourceReg << ", 4\n";
        } else if (dest->GetType() == TACOperand::Type::REGISTER) {
            if (dest->GetValue() != sourceReg) {
                m_assembly << "    mov " << dest->GetValue() << ", " 
                          << sourceReg << "\n";
            }
        }
    }
    
    // ===================================================================
    // GÉNÉRATION DES INSTRUCTIONS
    // ===================================================================
    
    void GenerateBinaryOp(std::shared_ptr<TACInstruction> instr) {
        // Charger les opérandes
        LoadOperand(instr->GetOp1(), "r0");
        LoadOperand(instr->GetOp2(), "r1");
        
        // Effectuer l'opération
        std::string op;
        switch (instr->GetOpCode()) {
            case TACInstruction::OpCode::ADD: op = "add"; break;
            case TACInstruction::OpCode::SUB: op = "sub"; break;
            case TACInstruction::OpCode::MUL: op = "mul"; break;
            case TACInstruction::OpCode::DIV: op = "div"; break;
            default: return;
        }
        
        m_assembly << "    " << op << " r0, r1\n";
        
        // Stocker le résultat
        StoreResult(instr->GetDest(), "r0");
    }
    
    void GenerateComparison(std::shared_ptr<TACInstruction> instr) {
        // Charger les opérandes
        LoadOperand(instr->GetOp1(), "r0");
        LoadOperand(instr->GetOp2(), "r1");
        
        // Effectuer la comparaison
        switch (instr->GetOpCode()) {
            case TACInstruction::OpCode::EQ:
                m_assembly << "    eq r0, r0, r1\n";
                break;
            case TACInstruction::OpCode::NE:
                m_assembly << "    eq r0, r0, r1\n";
                m_assembly << "    lcons r2, 1\n";
                m_assembly << "    xor r0, r2\n";
                break;
            case TACInstruction::OpCode::GT:
                m_assembly << "    gt r0, r0, r1\n";
                break;
            case TACInstruction::OpCode::LT:
                m_assembly << "    lt r0, r0, r1\n";
                break;
            case TACInstruction::OpCode::GE:
                // >= est NOT(<)
                m_assembly << "    lt r0, r0, r1\n";
                m_assembly << "    lcons r2, 1\n";
                m_assembly << "    xor r0, r2\n";
                break;
            case TACInstruction::OpCode::LE:
                // <= est NOT(>)
                m_assembly << "    gt r0, r0, r1\n";
                m_assembly << "    lcons r2, 1\n";
                m_assembly << "    xor r0, r2\n";
                break;
            default: return;
        }
        
        // Stocker le résultat
        StoreResult(instr->GetDest(), "r0");
    }
    
    void GenerateCopy(std::shared_ptr<TACInstruction> instr) {
        LoadOperand(instr->GetOp1(), "r0");
        StoreResult(instr->GetDest(), "r0");
    }
    
    void GenerateLoad(std::shared_ptr<TACInstruction> instr) {
        // dest = *op1
        LoadOperand(instr->GetOp1(), "r0");  // r0 contient l'adresse
        m_assembly << "    load r1, @r0, 4\n";
        StoreResult(instr->GetDest(), "r1");
    }
    
    void GenerateStore(std::shared_ptr<TACInstruction> instr) {
        // *dest = op1
        LoadOperand(instr->GetOp1(), "r0");  // r0 contient la valeur
        LoadOperand(instr->GetDest(), "r1"); // r1 contient l'adresse
        m_assembly << "    store @r1, r0, 4\n";
    }
    
    void GenerateIfFalse(std::shared_ptr<TACInstruction> instr) {
        LoadOperand(instr->GetOp1(), "r0");
        m_assembly << "    skipz r0\n";
        m_assembly << "    jump " << instr->GetDest()->ToString() << "\n";
    }
    
    void GenerateIfTrue(std::shared_ptr<TACInstruction> instr) {
        LoadOperand(instr->GetOp1(), "r0");
        m_assembly << "    skipnz r0\n";
        m_assembly << "    jump " << instr->GetDest()->ToString() << "\n";
    }
    
    std::vector<std::shared_ptr<TACOperand>> m_printParams;
    
    void GenerateParam(std::shared_ptr<TACInstruction> instr) {
        // Accumuler les paramètres pour le prochain PRINT/CALL
        m_printParams.push_back(instr->GetDest());
    }
    
    void GeneratePrint(std::shared_ptr<TACInstruction> instr) {
        // Sauvegarder r0, r1
        m_assembly << "    push r0\n";
        m_assembly << "    push r1\n";
        
        // Sauvegarder les registres d'arguments selon le nombre de paramètres
        int paramCount = m_printParams.size();
        for (int i = 0; i < std::min(4, paramCount); i++) {
            m_assembly << "    push r" << (i + 2) << "\n";
        }
        
        // Charger les paramètres dans r2, r3, r4, r5
        for (size_t i = 0; i < m_printParams.size() && i < 4; i++) {
            LoadOperand(m_printParams[i], "r" + std::to_string(i + 2));
        }
        
        // Charger la chaîne de format dans r0
        m_assembly << "    lcons r0, $" << instr->GetDest()->GetValue() << "\n";
        
        // Charger le nombre d'arguments dans r1
        m_assembly << "    lcons r1, " << paramCount << "\n";
        
        // Syscall
        m_assembly << "    syscall 4\n";
        
        // Restaurer les registres
        for (int i = std::min(4, paramCount) - 1; i >= 0; i--) {
            m_assembly << "    pop r" << (i + 2) << "\n";
        }
        m_assembly << "    pop r1\n";
        m_assembly << "    pop r0\n";
        
        // Vider la liste des paramètres
        m_printParams.clear();
    }
    
    void GenerateCall(std::shared_ptr<TACInstruction> instr) {
        // Générer l'appel de fonction
        std::string functionName = instr->GetOp1() ? 
                                   instr->GetOp1()->ToString() : 
                                   instr->GetDest()->ToString();
        
        m_assembly << "    call " << functionName << "\n";
        
        // Si on a une destination, sauvegarder le résultat
        if (instr->GetDest() && instr->GetOp1()) {
            // Le résultat est dans r0 par convention
            StoreResult(instr->GetDest(), "r0");
        }
    }
    
    // ===================================================================
    // HELPER : Obtenir la taille d'une variable
    // ===================================================================
    int GetVariableSize(std::shared_ptr<Variable> var) {
        if (!var) return 4;
        
        switch (var->GetValueType()) {
            case Variable::ValueType::BOOL:
                return 1;
            case Variable::ValueType::INTEGER:
            case Variable::ValueType::FLOAT:
                return 4;
            case Variable::ValueType::STRING:
                return 1; // Par caractère
            default:
                return 4;
        }
    }
    
    // ===================================================================
    // IMPLÉMENTATIONS DES MÉTHODES VIRTUELLES - SECTION DATA
    // ===================================================================
    
    virtual void Visit(const std::shared_ptr<Variable> v) override {
        if (!v || !v->IsConstant()) return;
        
        switch (v->GetValueType()) {
            case Variable::ValueType::STRING: {
                std::string value = v->GetValue<std::string>();
                
                // Convertir {0} {1} {2} {3} en %d
                for (int i = 0; i < 4; ++i) {
                    std::string placeholder = "{" + std::to_string(i) + "}";
                    size_t pos = 0;
                    while ((pos = value.find(placeholder, pos)) != std::string::npos) {
                        value.replace(pos, placeholder.length(), "%d");
                        pos += 2;
                    }
                }
                
                m_assembly << "$" << v->GetLabel() << " DC8, \"" 
                          << value << "\" ; " << v->GetVariableName() << "\n";
                break;
            }
            
            case Variable::ValueType::INTEGER:
                m_assembly << "$" << v->GetLabel() << " DC32, " 
                          << v->GetValue<int>() << " ; " 
                          << v->GetVariableName() << "\n";
                break;
            
            case Variable::ValueType::FLOAT:
                m_assembly << "$" << v->GetLabel() << " DC32, " 
                          << v->GetValue<float>() << " ; " 
                          << v->GetVariableName() << "\n";
                break;
            
            case Variable::ValueType::BOOL:
                m_assembly << "$" << v->GetLabel() << " DCB, " 
                          << (v->GetValue<bool>() ? "1" : "0") << " ; " 
                          << v->GetVariableName() << "\n";
                break;
            
            default:
                AddComment("WARNING: Unsupported constant variable type for " + 
                          v->GetVariableName());
                break;
        }
    }
    
    virtual void GenerateVariable(const std::shared_ptr<Variable> v) override {
        if (!v) return;
        
        switch (v->GetValueType()) {
            case Variable::ValueType::STRING:
                m_assembly << "$" << v->GetLabel() << " DV8, \"" 
                        << v->GetValue<std::string>() << "\" ; " 
                        << v->GetVariableName() << "\n";
                break;
            
            case Variable::ValueType::INTEGER:
                // CORRECTION : Déclarer 1 élément, pas la valeur !
                m_assembly << "$" << v->GetLabel() << " DV32, 1 ; " 
                        << v->GetVariableName() << "\n";
                break;
            
            case Variable::ValueType::FLOAT:
                m_assembly << "$" << v->GetLabel() << " DV32, 1 ; " 
                        << v->GetVariableName() << "\n";
                break;
            
            case Variable::ValueType::BOOL:
                m_assembly << "$" << v->GetLabel() << " DVB, 1 ; " 
                        << v->GetVariableName() << "\n";
                break;
            
            default:
                AddComment("WARNING: Unsupported variable type for " + 
                        v->GetVariableName());
                m_assembly << "$" << v->GetLabel() << " DV32, 1 ; " 
                        << v->GetVariableName() << " (unknown type)\n";
                break;
        }
    }
    

};
