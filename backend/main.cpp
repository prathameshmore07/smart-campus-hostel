#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <stack>
#include <map>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <thread>
#include <mutex>

// Socket Networking Headers for macOS / Unix
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

inline int socket_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    return ::bind(sockfd, addr, addrlen);
}

using namespace std;

// ============================================================================
// SECTION 1: GLOBAL CONFIGURATIONS & ANALYTICS STATE
// ============================================================================

mutex dataMutex; // Thread safety protection for runtime dataset modifications

int totalAllocations = 0;
int failedAllocations = 0;
int successfulSwaps = 0;
int compatibilityChecks = 0;
double totalCompatibilityScore = 0.0;
int nextAllocationOrder = 1;
bool allowUpgrade = false;

// ============================================================================
// SECTION 2: STRUCTURES, MODELS & DECLARATIONS
// ============================================================================

struct Student {
    string id, name, currentRoom = "", waitReason = "Pending Allocation";
    char gender; 
    double cgpa = 0.0;
    int seniority = 1, roomPref = 0, distance = 0, prevPattern = 0, habits[3] = {5, 5, 5}, foodPreference = 0;
    bool wantsMess = false, wantsAC = false, specialNeeds = false;
    bool isAllocated = false, inWaitlist = false, isGraduated = false;
    int waitTime = 0, rejections = 0, fairnessScore = 0, riskLevel = 0;
    int allocationOrder = 0;
    
    // Academic Lifecycle Traits
    int currentYear = 1;
    bool examPassed = false;
    bool hasBacklog = false;
    bool placementExtension = false;
    double finalSemesterCGPA = 0.0;
    int semester = 1; // Cumulative academic term (1-8)
    int familyBackgroundRisk = 0; // 0: low, 1: high
    
    Student* next = nullptr; 
};

struct Room {
    string id; 
    char gender; 
    int type, capacity, occupancy = 0;
    Student* head = nullptr; 
    Room* nextTypeRoom = nullptr; 
    bool isAC;
    
    bool allocate(Student* s) {
        if (s->gender != gender) {
            return false;
        }
        if (s->isAllocated || occupancy >= capacity) return false;
        s->next = head; head = s; occupancy++; s->isAllocated = true; s->currentRoom = id;
        s->waitReason = "Allocated Successfully"; totalAllocations++; 
        s->allocationOrder = nextAllocationOrder++;
        return true;
    }
    
    void removeStudent(string sId) {
        Student *c = head, *p = nullptr;
        while (c && c->id != sId) { p = c; c = c->next; }
        if (c) { 
            if (p) p->next = c->next; else head = c->next; 
            occupancy--; c->next = nullptr; c->isAllocated = false; c->currentRoom = ""; totalAllocations--; 
        }
    }
};

// Primary Global Storage Tables
vector<Room> globalRooms; 
map<string, Student*> allStudents; 
map<string, Room*> typeHeads;

// Forward Declarations for Cross-Referenced Functions
void logCheckout(const Student* s, const string& reason);
void exportReports(); 
void saveToTxt();

// ============================================================================
// SECTION 3: DATABASE SEARCH & REMOVAL LOOKUP HELPERS
// ============================================================================

Student* findStudentById(const string& sid) {
    auto it = allStudents.find(sid);
    return (it != allStudents.end()) ? it->second : nullptr;
}

Room* findRoomById(const string& rid) {
    for (Room& r : globalRooms) {
        if (r.id == rid) return &r;
    }
    return nullptr;
}

void vacateStudentFromRoom(Student* s) {
    if (!s || !s->isAllocated || s->currentRoom.empty()) return;
    Room* r = findRoomById(s->currentRoom);
    if (r) {
        r->removeStudent(s->id);
    }
}

// ============================================================================
// SECTION 4: FORMAT & ID VALIDATION SYSTEM
// ============================================================================

bool isAlphaOnly(const string& str) {
    if (str.empty()) return false;
    for (char c : str) {
        if (!isalpha(c) && c != ' ') return false;
    }
    return true;
}

bool isAlphanumericOnly(const string& str) {
    if (str.empty()) return false;
    for (char c : str) {
        if (!isalnum(c)) return false;
    }
    return true;
}

bool isValidStudentID(const string& id, char gender) {
    if (id.length() < 3) return false; 
    string prefix = id.substr(0, 2);
    if (gender == 'M' && prefix != "SM") return false;
    if (gender == 'F' && prefix != "SF") return false;
    string numeric = id.substr(2);
    if (numeric.empty()) return false;
    for (char c : numeric) { if (!isdigit(c)) return false; }
    return true;
}

bool isValidStudentIDFormat(const string& id) {
    if (id.length() < 3) return false;
    string prefix = id.substr(0, 2);
    if (prefix != "SM" && prefix != "SF") return false;
    string numeric = id.substr(2);
    if (numeric.empty()) return false;
    for (char c : numeric) { if (!isdigit(c)) return false; }
    return true;
}

char genderFromID(const string& id) {
    return (id.length() >= 2 && id[1] == 'M') ? 'M' : 'F';
}

bool isNumericOnly(const string& str, bool allowDecimal = false, bool allowNegative = false) {
    if (str.empty()) return false;
    int dotCount = 0;
    for (size_t i = 0; i < str.length(); i++) {
        char c = str[i];
        if (c == '-' && i == 0 && allowNegative) continue;
        if (c == '.' && allowDecimal) {
            dotCount++;
            if (dotCount > 1) return false;
            continue;
        }
        if (!isdigit(c)) return false;
    }
    return true;
}

// ============================================================================
// SECTION 5: DSA 1 — AVL TREE FOR MERIT-BASED RANKING
// ============================================================================

struct BSTNode { 
    Student* student; 
    BSTNode *left = nullptr, *right = nullptr; 
    int height = 1; 
    BSTNode(Student* s) : student(s) {} 
};

class StudentAVLTree {
    BSTNode* root = nullptr;
    int h(BSTNode* n) { return n ? n->height : 0; }
    int b(BSTNode* n) { return n ? h(n->left) - h(n->right) : 0; }
    
    BSTNode* rRot(BSTNode* y) { 
        BSTNode* x = y->left; y->left = x->right; x->right = y; 
        y->height = max(h(y->left), h(y->right)) + 1; x->height = max(h(x->left), h(x->right)) + 1; return x; 
    }
    
    BSTNode* lRot(BSTNode* x) { 
        BSTNode* y = x->right; x->right = y->left; y->left = x; 
        x->height = max(h(x->left), h(x->right)) + 1; y->height = max(h(y->left), h(y->right)) + 1; return y; 
    }
    
    BSTNode* ins(BSTNode* n, Student* s) {
        if (!n) return new BSTNode(s);
        if (s->cgpa > n->student->cgpa) n->left = ins(n->left, s); else n->right = ins(n->right, s);
        n->height = 1 + max(h(n->left), h(n->right)); int bal = b(n);
        if (bal > 1 && s->cgpa > n->left->student->cgpa) return rRot(n);
        if (bal < -1 && s->cgpa <= n->right->student->cgpa) return lRot(n);
        if (bal > 1 && s->cgpa <= n->left->student->cgpa) { n->left = lRot(n->left); return rRot(n); }
        if (bal < -1 && s->cgpa > n->right->student->cgpa) { n->right = rRot(n->right); return lRot(n); }
        return n;
    }
    
    BSTNode* minNode(BSTNode* node) {
        BSTNode* current = node;
        while (current && current->left) current = current->left;
        return current;
    }
    
    BSTNode* del(BSTNode* n, Student* s) {
        if (!n) return nullptr;
        
        if (s->cgpa > n->student->cgpa) {
            n->left = del(n->left, s);
        } else if (s->cgpa < n->student->cgpa) {
            n->right = del(n->right, s);
        } else {
            if (s->id == n->student->id) {
                if (!n->left || !n->right) {
                    BSTNode* temp = n->left ? n->left : n->right;
                    if (!temp) {
                        temp = n;
                        n = nullptr;
                    } else {
                        *n = *temp;
                    }
                    delete temp;
                } else {
                    BSTNode* temp = minNode(n->right);
                    n->student = temp->student;
                    n->right = del(n->right, temp->student);
                }
            } else {
                n->right = del(n->right, s);
            }
        }
        
        if (!n) return nullptr;
        
        n->height = 1 + max(h(n->left), h(n->right));
        int bal = b(n);
        
        if (bal > 1 && b(n->left) >= 0) return rRot(n);
        if (bal > 1 && b(n->left) < 0) { n->left = lRot(n->left); return rRot(n); }
        if (bal < -1 && b(n->right) <= 0) return lRot(n);
        if (bal < -1 && b(n->right) > 0) { n->right = rRot(n->right); return lRot(n); }
        
        return n;
    }
    
    void inord(BSTNode* n, vector<Student*>& l) { 
        if (!n) return; 
        inord(n->left, l); l.push_back(n->student); inord(n->right, l); 
    }
    
    void freeMemory(BSTNode* node) {
        if (!node) return;
        freeMemory(node->left);
        freeMemory(node->right);
        delete node;
    }
public:
    ~StudentAVLTree() { freeMemory(root); root = nullptr; }
    void addStudent(Student* s) { root = ins(root, s); }
    void removeStudent(Student* s) { root = del(root, s); }
    vector<Student*> getSorted() { vector<Student*> l; inord(root, l); return l; }
} studentTree;

void addStudentRaw(Student sData) { 
    Student* s = new Student(sData); studentTree.addStudent(s); allStudents[s->id] = s; 
}

// ============================================================================
// SECTION 6: DSA 2 — DP MEMOIZATION (COMPATIBILITY & RISK RATINGS)
// ============================================================================

map<string, double> dpRiskMemo; 
map<string, int> dpCompatMemo;  

void calcRisk(Student* s) {
    string key = to_string(s->distance) + "_" + to_string(s->prevPattern) + "_" + to_string(s->familyBackgroundRisk);
    if (!dpRiskMemo.count(key)) {
        double baseP = min(1.0, 0.05 + (s->distance > 100 ? 0.20 : 0) + (s->prevPattern == 1 ? 0.35 : 0));
        double finalP = min(1.0, baseP + (s->familyBackgroundRisk == 1 ? 0.25 : 0.0));
        dpRiskMemo[key] = finalP;
    }
    double p = dpRiskMemo[key]; 
    s->riskLevel = p < 0.3 ? 0 : (p < 0.6 ? 1 : 2);
}

int getCompatibility(Student* s1, Student* s2) {
    if (!s1 || !s2) return 0;
    string key = s1->id < s2->id ? s1->id + "_" + s2->id : s2->id + "_" + s1->id;
    if (dpCompatMemo.count(key)) return dpCompatMemo[key]; 
    
    int sc = 100; 
    sc -= abs(s1->habits[0] - s2->habits[0]) * 10;  
    sc -= abs(s1->habits[1] - s2->habits[1]) * 15; 
    sc -= abs(s1->habits[2] - s2->habits[2]) * 10; 
    
    if (s1->wantsMess && s2->wantsMess && s1->foodPreference != s2->foodPreference) sc -= 20; 
    
    sc = max(0, min(100, sc)); 
    
    compatibilityChecks++; 
    totalCompatibilityScore += sc; 
    return dpCompatMemo[key] = sc; 
}

// ============================================================================
// SECTION 7: DSA 3 — PRIORITY QUEUE FOR FAIRNESS-BASED WAITLIST
// ============================================================================

struct WaitlistComp {
    bool operator()(Student* a, Student* b) { 
        return ((a->waitTime * 2.0) + (a->fairnessScore * 1.5) + (a->rejections * 1.0) + (a->seniority * 0.5) + (a->specialNeeds ? 50.0 : 0) - (a->riskLevel * 5.0)) <
               ((b->waitTime * 2.0) + (b->fairnessScore * 1.5) + (b->rejections * 1.0) + (b->seniority * 0.5) + (b->specialNeeds ? 50.0 : 0) - (b->riskLevel * 5.0));
    }
};

priority_queue<Student*, vector<Student*>, WaitlistComp> waitQueue; 
queue<Student*> fairnessQueue; 

void rebuildWaitlist() {
    priority_queue<Student*, vector<Student*>, WaitlistComp> emptyQueue;
    std::swap(waitQueue, emptyQueue);
    
    queue<Student*> emptyFair;
    std::swap(fairnessQueue, emptyFair);
    
    for (auto const& p : allStudents) {
        Student* s = p.second;
        if (s->inWaitlist && !s->isAllocated && !s->isGraduated) {
            waitQueue.push(s);
            fairnessQueue.push(s);
        }
    }
}

// ============================================================================
// SECTION 8: DSA 4 — STACK FOR UNDO SWAPS
// ============================================================================

struct SwapAction { string id1, id2, room1, room2; };
stack<SwapAction> undoStack; 

string getCurrentTimestamp() {
    time_t now = time(0);
    char* dt = ctime(&now);
    string timestamp = dt ? string(dt) : "N/A";
    if (!timestamp.empty() && timestamp.back() == '\n') timestamp.pop_back();
    return timestamp;
}

// ============================================================================
// SECTION 9: CORE ALLOCATION ALGORITHMS (STRICT & FALLBACK AUTO-UPGRADE)
// ============================================================================

bool checkRoomCompatibility(Room* r, Student* newS, int threshold) {
    if (r->gender != newS->gender) return false;
    for (Student* s = r->head; s; s = s->next) {
        if (getCompatibility(s, newS) < threshold) return false; 
    }
    return true;
}

bool isCompatible(Room* r, Student* newS) {
    return checkRoomCompatibility(r, newS, 40);
}

bool tryAllocate(Student* s, bool strict) {
    if (strict) {
        bool allocated = false;
        string key = string(1, s->gender) + "_" + to_string(s->roomPref);
        if (typeHeads.count(key)) {
            for (Room* r = typeHeads[key]; r; r = r->nextTypeRoom) {
                if (r->isAC == s->wantsAC && r->occupancy < r->capacity && isCompatible(r, s)) {
                    if (r->allocate(s)) {
                        s->inWaitlist = false;
                        s->waitReason = "Allocated Successfully";
                        allocated = true;
                        return true;
                    }
                }
            }
        }
        if (!allocated) {
            s->waitReason = "Preferred Room Unavailable";
        }
        return false;
    }
    
    // Fallback automatic room upgrades
    if (!allowUpgrade) return false;
    
    vector<Room*> avail;
    for (Room& r : globalRooms) {
        if (r.gender == s->gender && r.type > s->roomPref && r.occupancy < r.capacity && r.isAC == s->wantsAC) {
            avail.push_back(&r);
        }
    }
    
    sort(avail.begin(), avail.end(), [](Room* a, Room* b) { 
        return (a->capacity - a->occupancy) > (b->capacity - b->occupancy); 
    });
    
    for (Room* r : avail) {
        if (isCompatible(r, s) && r->allocate(s)) {
            s->inWaitlist = false;
            s->waitReason = "Auto Upgraded";
            return true;
        }
    }
    
    return false;
}

// ============================================================================
// SECTION 10: DSA 5 — SWAPGRAPH DFS & CYCLE DETECTION ENGINE
// ============================================================================

class SwapGraph {
    map<string, vector<string>> adj;
    void dfs(string u, string p, string start, map<string, bool>& vis, vector<string>& path, vector<vector<string>>& cycles) {
        vis[u] = true; path.push_back(u);
        for (string v : adj[u]) {
            if (v == p) {
                if (v == start && path.size() == 2) {
                    cycles.push_back(path);
                }
                continue;
            }
            if (v == start && path.size() > 1) {
                cycles.push_back(path);
            } else if (!vis[v]) {
                dfs(v, u, start, vis, path, cycles);
            }
        }
        path.pop_back(); vis[u] = false;
    }
public:
    void addEdge(string u, string v) { adj[u].push_back(v); adj[v].push_back(u); }
    vector<vector<string>> findCycles(string start) { 
        map<string, bool> vis; vector<string> path; vector<vector<string>> cycles; 
        dfs(start, "", start, vis, path, cycles); return cycles; 
    }
} swapGraph;

// Swap Chain Verification Helper
bool isChainValidForSwap(const vector<string>& chain, map<string, string>& orig, stringstream& log) {
    for (size_t i = 0; i < chain.size(); i++) {
        string u = chain[i];
        string nextU = chain[(i + 1) % chain.size()];
        Student* s = findStudentById(u);
        Student* ns = findStudentById(nextU);
        
        if (!s || !s->isAllocated) {
            log << "[Error] Student " << u << " is not allocated to any room.\n";
            return false;
        }
        if (!ns) {
            log << "[Error] Student " << nextU << " not found in the system.\n";
            return false;
        }
        if (s->gender != ns->gender) {
            log << "[Error] Swap chain violates strict gender segregation rules. Operation Aborted.\n";
            return false;
        }
        if (orig.count(u) == 0) {
            orig[u] = s->currentRoom;
        }
    }
    return true;
}

// ============================================================================
// SECTION 11: SYSTEM WORKFLOWS (ALLOCATE, RECOVER, SWAPS & SEMESTER PROCESS)
// ============================================================================

string runAllocation() {
    stringstream log;
    log << "[Smart Allocation Process Started]\n";
    vector<Student*> sorted = studentTree.getSorted();
    for (Student* s : sorted) { if (s->isGraduated) continue; calcRisk(s); }
    
    int qSz = fairnessQueue.size();
    int fairnessPromotions = 0;
    for (int i = 0; i < qSz; i++) {
        Student* s = fairnessQueue.front(); fairnessQueue.pop(); if (s->isGraduated) continue;
        if (!s->isAllocated && (tryAllocate(s, true) || (allowUpgrade && tryAllocate(s, false)))) {
            fairnessPromotions++;
            log << "[Fairness Promotion] Allocated waitlisted student " << s->id << " (" << s->name << ") to Room " << s->currentRoom << "\n";
        } else if (!s->isAllocated) {
            fairnessQueue.push(s);
        }
    }
    
    sort(sorted.begin(), sorted.end(), [](Student* a, Student* b) {
        double scoreA = (a->cgpa * 10.0) + (a->seniority * 5.0) + (a->specialNeeds ? 50.0 : 0) - (a->riskLevel * 5.0);
        double scoreB = (b->cgpa * 10.0) + (b->seniority * 5.0) + (b->specialNeeds ? 50.0 : 0) - (b->riskLevel * 5.0);
        if (abs(scoreA - scoreB) > 1e-9) {
            return scoreA > scoreB;
        }
        if (a->roomPref != b->roomPref) {
            return a->roomPref < b->roomPref; // Higher priority (Single sharing first)
        }
        return a->seniority > b->seniority; // Higher seniority first
    });
    
    int newlyAllocated = 0;
    int newlyWaitlisted = 0;
    for (Student* s : sorted) {
        if (s->isGraduated) continue;
        if (!s->isAllocated) {
            if (tryAllocate(s, true) || (allowUpgrade && tryAllocate(s, false))) {
                newlyAllocated++;
                log << "[Greedy Allocation] Allocated " << s->id << " (" << s->name << ") to Room " << s->currentRoom << "\n";
            } else if (!s->inWaitlist) {
                s->fairnessScore += ++s->rejections; s->waitTime++; fairnessQueue.push(s); waitQueue.push(s); s->inWaitlist = true; failedAllocations++;
                newlyWaitlisted++;
                log << "[Waitlist Action] Waitlisted student " << s->id << " (" << s->name << ") | Reason: " << s->waitReason << "\n";
            }
        }
    }
    saveToTxt();
    rebuildWaitlist();
    log << "\n[Success] Allocation process completed successfully.\n";
    log << "Total Processed Students: " << sorted.size() << "\n";
    log << "Fairness Promotions: " << fairnessPromotions << "\n";
    log << "Newly Allocated: " << newlyAllocated << "\n";
    log << "Newly Waitlisted: " << newlyWaitlisted << "\n";
    return log.str();
}

string recoverVacancies() {
    stringstream log;
    log << "[Vacancy Recovery Process Started]\n";
    log << "Scanning for high-risk no-shows (Distance and past patterns analysis)...\n";
    bool found = false;
    for (Room& r : globalRooms) {
        Student* curr = r.head;
        while (curr) {
            Student* nxt = curr->next; calcRisk(curr);
            if (curr->riskLevel == 2) { 
                logCheckout(curr, "Removed due to High Risk (No-Show)");
                r.removeStudent(curr->id); curr->waitReason = "Removed due to High Risk (No-Show)"; 
                curr->inWaitlist = true; waitQueue.push(curr); fairnessQueue.push(curr); found = true; 
                log << "[Vacancy Detected] Student " << curr->id << " (" << curr->name << ") classified as High Risk. Vacated from Room " << r.id << ".\n";
            }
            curr = nxt;
        }
    }
    if (!found) { 
        log << "No new high-risk vacancies detected during this scan. All rooms are stable.\n"; 
    }
    
    log << "Processing waitlist allocation for any available vacant spaces...\n";
    vector<Student*> tq; 
    while (!waitQueue.empty()) { 
        tq.push_back(waitQueue.top()); 
        waitQueue.pop(); 
    }
    
    bool anyPromoted = false;
    for (Student* s : tq) {
        if (s->isAllocated || s->isGraduated) continue;
        if (tryAllocate(s, true) || (allowUpgrade && tryAllocate(s, false))) {
            log << "[Promotion] Promoted waitlisted student " << s->id << " (" << s->name << ") to vacant Room " << s->currentRoom << ".\n";
            anyPromoted = true;
        } else {
            s->fairnessScore++;
            waitQueue.push(s);
        }
    }
    if (!anyPromoted) {
        log << "No waitlisted students could be promoted (no vacant rooms of preferred type/gender, or waitlist is empty).\n";
    }
    
    saveToTxt(); 
    rebuildWaitlist();
    log << "\n[Success] Vacancy recovery completed.\n";
    return log.str();
}

string processRoomSwaps(string start) {
    stringstream log;
    log << "[Graph DFS Swap Chain Algorithm Initiated]\n";
    Student* startStudent = findStudentById(start);
    if (!startStudent) {
        log << "[Error] Student " << start << " not found in the system.\n";
        return log.str();
    }
    if (!startStudent->isAllocated) {
        log << "[Error] Student " << start << " must be allocated to a room before initiating a swap.\n";
        return log.str();
    }
    
    auto cycles = swapGraph.findCycles(start); 
    if (cycles.empty()) { 
        log << "[Error] No cyclic swap chains detected with starting student " << start << ". Add swap connections first.\n"; 
        return log.str(); 
    } 
    
    vector<string> chain = cycles[0]; 
    map<string, string> orig;
    log << "Cycle Detected in Swap Graph: ";
    for (size_t i = 0; i < chain.size(); i++) {
        log << chain[i] << " -> ";
    }
    log << chain[0] << "\n";
    
    // Safety verification: Gender check
    if (!isChainValidForSwap(chain, orig, log)) {
        return log.str();
    }
    
    // Temporarily vacate rooms
    for (string u : chain) {
        Room* r = findRoomById(orig[u]);
        if (r) {
            r->removeStudent(u);
        }
    }
    
    bool success = true;
    for (size_t i = 0; i < chain.size(); i++) {
        string tRoomId = orig[chain[(i + 1) % chain.size()]]; 
        Student* s = findStudentById(chain[i]); 
        Room* r = findRoomById(tRoomId);
        
        bool alloc = false;
        if (r && isCompatible(r, s)) { 
            r->allocate(s); 
            alloc = true; 
        }
        if (!alloc) {
            success = false;
            log << "[Validation Alert] Mismatch in compatibility checks for Student " << s->id << " in target Room " << tRoomId << ".\n";
        }
    }
    
    if (!success) {
        log << "[Error] Compatibility threshold not met during swap. Reverting all changes to original states...\n";
        for (string u : chain) {
            Student* s = findStudentById(u);
            vacateStudentFromRoom(s);
            Room* rOrig = findRoomById(orig[u]);
            if (rOrig) {
                rOrig->allocate(s);
            }
        }
        return log.str();
    }
    
    if (chain.size() == 2) {
        undoStack.push({chain[0], chain[1], orig[chain[0]], orig[chain[1]]});
    }
    successfulSwaps++; 
    saveToTxt(); 
    log << "[Success] Room swap chain executed perfectly! Positions updated on database.\n";
    for (string u : chain) {
        log << " - Student " << u << " successfully reassigned to Room " << allStudents[u]->currentRoom << "\n";
    }
    return log.str();
}

string undoLastSwap() {
    stringstream log;
    log << "[Undo Last Swap Operation Initiated]\n";
    if (undoStack.empty()) { 
        log << "[Error] No previous direct swap records in Stack storage to undo.\n"; 
        return log.str(); 
    }
    auto a = undoStack.top(); undoStack.pop(); 
    Student *s1 = findStudentById(a.id1), *s2 = findStudentById(a.id2);
    
    if (s1) vacateStudentFromRoom(s1);
    if (s2) vacateStudentFromRoom(s2);
    
    Room* r1 = findRoomById(a.room1);
    Room* r2 = findRoomById(a.room2);
    
    if (r1 && s1) r1->allocate(s1);
    if (r2 && s2) r2->allocate(s2);
    
    saveToTxt(); 
    log << "[Success] Reverted previous swap successfully.\n";
    log << " - Student " << a.id1 << " moved back to Room " << a.room1 << "\n";
    log << " - Student " << a.id2 << " moved back to Room " << a.room2 << "\n";
    return log.str();
}

string semesterRenewal() {
    stringstream log;
    log << "[Semester Hostel Renewal & Checkout Process Started]\n";
    log << "[Academic Eligibility Check Phase]\n"; 
    bool anyCheckout = false;
    int skippedBacklog = 0, skippedFailed = 0, skippedExtension = 0, skippedSemester = 0;
    
    for (auto& p : allStudents) {
        Student* s = p.second;
        if (s->seniority == 4 && !s->isGraduated) {
            // Academic eligibility workflow
            if (s->semester < 8) {
                log << "Retained: " << s->id << " - " << s->name << " (Final semester not completed, staying in hostel).\n";
                skippedSemester++;
                continue;
            }
            if (!s->examPassed) {
                log << "Retained: " << s->id << " - " << s->name << " (Exam not passed, staying in hostel).\n";
                skippedFailed++;
                continue;
            }
            if (s->hasBacklog) {
                log << "Retained: " << s->id << " - " << s->name << " (Has active backlogs, staying in hostel).\n";
                skippedBacklog++;
                continue;
            }
            if (s->placementExtension) {
                log << "Retained: " << s->id << " - " << s->name << " (Placement extension approved, staying in hostel).\n";
                skippedExtension++;
                continue;
            }
            
            // Qualified checkout
            vacateStudentFromRoom(s);
            logCheckout(s, "Semester Renewal Checkout");
            s->isGraduated = true; s->isAllocated = false; s->currentRoom = ""; s->waitReason = "Graduated"; s->inWaitlist = false;
            log << "Checkout: " << s->id << " - " << s->name << " (Year 4, exams passed, no backlogs) checked out.\n"; 
            anyCheckout = true;
        }
    }
    if (!anyCheckout) {
        if (skippedFailed + skippedBacklog + skippedExtension + skippedSemester > 0) {
            log << "No eligible graduates found. Retained: " << skippedFailed << " failed, " << skippedBacklog << " backlog, " << skippedExtension << " extension, " << skippedSemester << " semester incomplete.\n";
        } else {
            log << "No Year-4 students found in the system.\n";
        }
    }
    
    log << "\n[Waitlist Rotation Phase]\n";
    vector<Student*> tq; while (!waitQueue.empty()) { tq.push_back(waitQueue.top()); waitQueue.pop(); }
    bool anyPromoted = false;
    for (Student* s : tq) {
        if (s->isGraduated || s->isAllocated) continue;
        if (tryAllocate(s, true) || (allowUpgrade && tryAllocate(s, false))) { 
            log << "Promotion: Waitlisted student " << s->id << " (" << s->name << ") promoted to Room " << s->currentRoom << ".\n"; 
            anyPromoted = true; 
        } else {
            waitQueue.push(s);
        }
    }
    if (!anyPromoted) log << "No waitlisted students could be allocated in this rotation (Capacity limitations).\n";
    saveToTxt(); 
    log << "\n[Success] Semester renewal run finalized.\n";
    return log.str();
}

// ============================================================================
// SECTION 12: FILE PERSISTENCE & REPORT EXPORT
// ============================================================================

void exportReports() {
    ofstream f1("exported_reports/room_allocations.txt"), 
             f2("exported_reports/waitlist_report.txt"), 
             f3("exported_reports/student_records.txt"), 
             f4("exported_reports/analytics_report.txt");
             
    f1 << "=== ROOM ALLOCATIONS ===\n";
    for (Room& r : globalRooms) { 
        f1 << "Room " << r.id << " (" << r.gender << ") [" << r.occupancy << "/" << r.capacity << "]:\n"; 
        for(Student* s=r.head; s; s=s->next) f1 << "  -> " << s->id << " | " << s->name << " (Risk: " << s->riskLevel << ")\n"; 
    }
    
    f2 << "=== WAITLIST REPORT ===\n";
    auto tq = waitQueue; int rk=1; 
    while(!tq.empty()) { 
        Student* s = tq.top();
        if (!s->isGraduated && !s->isAllocated) {
            f2 << rk++ << ". [" << s->id << "] " << s->name 
               << " | Reason: " << s->waitReason 
               << " | Fairness Score: " << s->fairnessScore 
               << " | Waiting Time: " << s->waitTime 
               << " | Risk Level: " << s->riskLevel << "\n"; 
        }
        tq.pop(); 
    }
    
    f3 << "=== STUDENT RECORDS ===\n";
    for (auto const& p : allStudents) {
        if (!p.second->isGraduated) {
            f3 << left << setw(6) << p.second->id << "| " << setw(12) << p.second->name << "| Gender: " << p.second->gender << " | CGPA: " << setw(4) << p.second->cgpa << " | Room: " << (p.second->isAllocated ? p.second->currentRoom : "Waitlisted") << "\n"; 
        }
    }
    
    int cap=0, occ=0; 
    for (Room& r : globalRooms) { cap+=r.capacity; occ+=r.occupancy; }
    f4 << "=== ANALYTICS REPORT ===\nTotal Rooms: " << globalRooms.size() << "\nTotal Capacity: " << cap << "\nTotal Occupancy: " << occ << "\nTotal Allocations: " << totalAllocations << "\nFailed Allocations: " << failedAllocations << "\nSuccessful Swaps: " << successfulSwaps << "\nAverage Roommate Compatibility: " << (compatibilityChecks ? (totalCompatibilityScore/compatibilityChecks) : 0) << "%\n";
}

void logCheckout(const Student* s, const string& reason) {
    ofstream out("checkout.txt", ios::app);
    ofstream out2("exported_reports/checkout.txt", ios::app);
    
    time_t now = time(0);
    char* dt = ctime(&now);
    string timestamp = dt ? string(dt) : "N/A";
    if (!timestamp.empty() && timestamp.back() == '\n') timestamp.pop_back();
    
    string roomStr = s->isAllocated ? s->currentRoom : (s->currentRoom.empty() ? "-" : s->currentRoom);
    
    if (out.is_open()) {
        out << "[" << timestamp << "] Resident ID: " << s->id 
            << " | Name: " << s->name 
            << " | Gender: " << s->gender 
            << " | Room: " << roomStr
            << " | Action/Reason: " << reason << "\n";
    }
    if (out2.is_open()) {
        out2 << "[" << timestamp << "] Resident ID: " << s->id 
             << " | Name: " << s->name 
             << " | Gender: " << s->gender 
             << " | Room: " << roomStr
             << " | Action/Reason: " << reason << "\n";
    }
}

void saveToTxt() {
    ofstream rf("sample_data/rooms.txt"), sf("sample_data/students.txt");
    for (Room& r : globalRooms) rf << r.id << " " << r.gender << " " << r.type << " " << r.capacity << " " << (r.isAC ? "AC" : "NONAC") << "\n";
    for (auto const& p : allStudents) {
        Student* s = p.second; if (s->isGraduated) continue;
        sf << s->id << " " << s->name << " " << s->gender << " " << s->cgpa << " " << s->seniority << " " << s->specialNeeds << " " << s->roomPref << " " << s->wantsMess << " " << s->foodPreference << " " << s->wantsAC << " " << s->distance << " " << s->prevPattern << " " << s->habits[0] << " " << s->habits[1] << " " << s->habits[2] << " " << s->examPassed << " " << s->hasBacklog << " " << s->placementExtension << " " << s->finalSemesterCGPA << " " << s->semester << " " << s->familyBackgroundRisk << "\n";
    }
    exportReports();
}

void loadSettings() {
    ifstream f("sample_data/settings.txt");
    if (f.is_open()) {
        string line;
        while (getline(f, line)) {
            if (line.find("Allow automatic room upgrade: 1") != string::npos || line.find("Allow automatic room upgrade: true") != string::npos) {
                allowUpgrade = true;
            }
        }
    } else {
        ofstream out("sample_data/settings.txt");
        if (out.is_open()) {
            out << "Allow automatic room upgrade: 0\n";
        }
    }
}

void loadData() {
    loadSettings();
    ifstream rf("sample_data/rooms.txt"), sf("sample_data/students.txt");
    string id, acStr, n; char g; int t, c, sr, sp, pr, d, pp, h1, h2, h3, fp; double cgpa; bool wm, wac;
    globalRooms.clear();
    allStudents.clear();
    typeHeads.clear();
    
    if (rf) while (rf >> id >> g >> t >> c >> acStr) { Room r; r.id=id; r.gender=g; r.type=t; r.capacity=c; r.isAC=(acStr=="AC"); globalRooms.push_back(r); }
    for (Room& r : globalRooms) { string key = string(1, r.gender) + "_" + to_string(r.type); if (!typeHeads.count(key)) typeHeads[key] = &r; else { Room* curr = typeHeads[key]; while (curr->nextTypeRoom) curr = curr->nextTypeRoom; curr->nextTypeRoom = &r; } }
    if (sf) {
        string line;
        while (getline(sf, line)) {
            if (line.empty()) continue;
            istringstream iss(line);
            if (!(iss >> id >> n >> g >> cgpa >> sr >> sp >> pr >> wm >> fp >> wac >> d >> pp >> h1 >> h2 >> h3)) continue;
            if (!isValidStudentID(id, g)) continue;
            if (allStudents.count(id)) continue;
            Student s; s.id=id; s.name=n; s.gender=g; s.cgpa=cgpa; s.seniority=sr; s.currentYear=sr; s.specialNeeds=sp; s.roomPref=pr; s.wantsMess=wm; s.foodPreference=fp; s.wantsAC=wac; s.distance=d; s.prevPattern=pp; s.habits[0]=h1; s.habits[1]=h2; s.habits[2]=h3;
            // Read academic fields if present (backward compatible with old format)
            int ep = 0, hb = 0, pe = 0; double fsCgpa = 0.0;
            if (iss >> ep >> hb >> pe >> fsCgpa) {
                s.examPassed = ep; s.hasBacklog = hb; s.placementExtension = pe; s.finalSemesterCGPA = fsCgpa;
            }
            // Read semester if present, otherwise default based on year
            int sem = 0;
            if (iss >> sem && sem >= 1 && sem <= 8) {
                s.semester = sem;
            } else {
                s.semester = (s.seniority - 1) * 2 + 1;
            }
            // Read family background risk if present (backward compatible)
            int famRisk = 0;
            if (iss >> famRisk) {
                s.familyBackgroundRisk = famRisk;
            }
            addStudentRaw(s);
        }
    }
    exportReports();
}

// ============================================================================
// SECTION 13: JSON SERIALIZATION & PARSING HELPERS
// ============================================================================

string studentToJson(const Student* s) {
    if (!s) return "null";
    stringstream ss;
    ss << "{"
       << "\"id\":\"" << s->id << "\","
       << "\"name\":\"" << s->name << "\","
       << "\"gender\":\"" << string(1, s->gender) << "\","
       << "\"cgpa\":" << s->cgpa << ","
       << "\"seniority\":" << s->seniority << ","
       << "\"currentYear\":" << s->currentYear << ","
       << "\"specialNeeds\":" << (s->specialNeeds ? "true" : "false") << ","
       << "\"roomPref\":" << s->roomPref << ","
       << "\"wantsAC\":" << (s->wantsAC ? "true" : "false") << ","
       << "\"wantsMess\":" << (s->wantsMess ? "true" : "false") << ","
       << "\"foodPreference\":" << s->foodPreference << ","
       << "\"distance\":" << s->distance << ","
       << "\"prevPattern\":" << s->prevPattern << ","
       << "\"sleepHabit\":" << s->habits[0] << ","
       << "\"cleanlinessHabit\":" << s->habits[1] << ","
       << "\"studyHabit\":" << s->habits[2] << ","
       << "\"isAllocated\":" << (s->isAllocated ? "true" : "false") << ","
       << "\"currentRoom\":\"" << s->currentRoom << "\","
       << "\"inWaitlist\":" << (s->inWaitlist ? "true" : "false") << ","
       << "\"waitReason\":\"" << s->waitReason << "\","
       << "\"waitTime\":" << s->waitTime << ","
       << "\"rejections\":" << s->rejections << ","
       << "\"fairnessScore\":" << s->fairnessScore << ","
       << "\"riskLevel\":" << s->riskLevel << ","
       << "\"allocationOrder\":" << s->allocationOrder << ","
       << "\"isGraduated\":" << (s->isGraduated ? "true" : "false") << ","
       << "\"examPassed\":" << (s->examPassed ? "true" : "false") << ","
       << "\"hasBacklog\":" << (s->hasBacklog ? "true" : "false") << ","
       << "\"placementExtension\":" << (s->placementExtension ? "true" : "false") << ","
       << "\"finalSemesterCGPA\":" << s->finalSemesterCGPA << ","
       << "\"semester\":" << s->semester << ","
       << "\"familyBackgroundRisk\":" << s->familyBackgroundRisk
       << "}";
    return ss.str();
}

string roomToJson(const Room& r) {
    stringstream ss;
    ss << "{"
       << "\"id\":\"" << r.id << "\","
       << "\"gender\":\"" << string(1, r.gender) << "\","
       << "\"type\":" << r.type << ","
       << "\"capacity\":" << r.capacity << ","
       << "\"occupancy\":" << r.occupancy << ","
       << "\"isAC\":" << (r.isAC ? "true" : "false") << ","
       << "\"students\":[";
    bool first = true;
    for (Student* s = r.head; s; s = s->next) {
        if (!first) ss << ",";
        ss << studentToJson(s);
        first = false;
    }
    ss << "]}";
    return ss.str();
}

string getWaitlistJson() {
    stringstream ss;
    ss << "[";
    auto tq = waitQueue;
    bool first = true;
    while (!tq.empty()) {
        Student* s = tq.top();
        tq.pop();
        if (s->inWaitlist && !s->isAllocated && !s->isGraduated) {
            if (!first) ss << ",";
            ss << studentToJson(s);
            first = false;
        }
    }
    ss << "]";
    return ss.str();
}

// Simple JSON Parsing Primitives
string parseJsonString(const string& json, const string& key) {
    size_t kPos = json.find("\"" + key + "\"");
    if (kPos == string::npos) return "";
    size_t colon = json.find(":", kPos);
    if (colon == string::npos) return "";
    size_t startQuote = json.find("\"", colon);
    if (startQuote == string::npos) return "";
    size_t endQuote = json.find("\"", startQuote + 1);
    if (endQuote == string::npos) return "";
    return json.substr(startQuote + 1, endQuote - (startQuote + 1));
}

double parseJsonDouble(const string& json, const string& key) {
    size_t kPos = json.find("\"" + key + "\"");
    if (kPos == string::npos) return 0.0;
    size_t colon = json.find(":", kPos);
    if (colon == string::npos) return 0.0;
    size_t firstDigit = json.find_first_of("0123456789.-", colon);
    if (firstDigit == string::npos) return 0.0;
    size_t lastDigit = json.find_first_not_of("0123456789.eE+-", firstDigit);
    string numStr = json.substr(firstDigit, lastDigit - firstDigit);
    try {
        return stod(numStr);
    } catch (...) {
        return 0.0;
    }
}

int parseJsonInt(const string& json, const string& key) {
    return (int)parseJsonDouble(json, key);
}

bool parseJsonBool(const string& json, const string& key) {
    size_t kPos = json.find("\"" + key + "\"");
    if (kPos == string::npos) return false;
    size_t colon = json.find(":", kPos);
    if (colon == string::npos) return false;
    size_t tPos = json.find("true", colon);
    size_t fPos = json.find("false", colon);
    size_t comma = json.find(",", colon);
    if (comma == string::npos) comma = json.find("}", colon);
    
    if (tPos != string::npos && tPos < comma) return true;
    if (fPos != string::npos && fPos < comma) return false;
    
    size_t digPos = json.find_first_of("01", colon);
    if (digPos != string::npos && digPos < comma) {
        return json[digPos] == '1';
    }
    return false;
}

string escapeJsonString(const string& input) {
    stringstream ss;
    for (char c : input) {
        if (c == '"') ss << "\\\"";
        else if (c == '\\') ss << "\\\\";
        else if (c == '\n') ss << "\\n";
        else if (c == '\r') ss << "\\r";
        else if (c == '\t') ss << "\\t";
        else ss << c;
    }
    return ss.str();
}

// ============================================================================
// SECTION 14: MULTITHREADED SOCKET WEB SERVER
// ============================================================================

void sendHttpResponse(int clientSocket, int statusCode, const string& contentType, const string& body) {
    stringstream ss;
    string statusText = "OK";
    if (statusCode == 400) statusText = "Bad Request";
    else if (statusCode == 401) statusText = "Unauthorized";
    else if (statusCode == 404) statusText = "Not Found";
    else if (statusCode == 405) statusText = "Method Not Allowed";
    else if (statusCode == 500) statusText = "Internal Server Error";
    
    ss << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";
    ss << "Content-Type: " << contentType << "\r\n";
    ss << "Content-Length: " << body.length() << "\r\n";
    ss << "Access-Control-Allow-Origin: *\r\n"; 
    ss << "Access-Control-Allow-Methods: GET, POST, OPTIONS, DELETE\r\n";
    ss << "Access-Control-Allow-Headers: Content-Type\r\n";
    ss << "Connection: close\r\n\r\n";
    ss << body;
    string resp = ss.str();
    ::send(clientSocket, resp.c_str(), resp.length(), 0);
    ::close(clientSocket);
}

// REST Response Utilities for Centralized Generation
void sendJsonError(int clientSocket, int statusCode, const string& errorMsg, bool includeSuccessField = false) {
    stringstream ss;
    ss << "{";
    if (includeSuccessField) {
        ss << "\"success\":false,";
    }
    ss << "\"error\":\"" << escapeJsonString(errorMsg) << "\"}";
    sendHttpResponse(clientSocket, statusCode, "application/json", ss.str());
}

void sendActionLogResponse(int clientSocket, const string& logStr) {
    stringstream ss;
    ss << "{\"success\": true, \"log\": \"" << escapeJsonString(logStr) << "\"}";
    sendHttpResponse(clientSocket, 200, "application/json", ss.str());
}

bool isAdminAuthenticated(const string& requestStr) {
    return true; // Auto-authenticate sessions for local local developer workflow
}

void serveFile(const string& filePath, const string& contentType, int clientSocket) {
    ifstream file(filePath, ios::binary);
    if (!file) {
        sendHttpResponse(clientSocket, 404, "text/plain", "404 Not Found: File not found " + filePath);
        return;
    }
    stringstream fileContent;
    fileContent << file.rdbuf();
    string content = fileContent.str();
    
    stringstream ss;
    ss << "HTTP/1.1 200 OK\r\n";
    ss << "Content-Type: " << contentType << "\r\n";
    ss << "Content-Length: " << content.length() << "\r\n";
    ss << "Access-Control-Allow-Origin: *\r\n";
    ss << "Connection: close\r\n\r\n";
    string headers = ss.str();
    
    ::send(clientSocket, headers.c_str(), headers.length(), 0);
    ::send(clientSocket, content.c_str(), content.length(), 0);
    ::close(clientSocket);
}

string getContentType(const string& path) {
    if (path.length() >= 5 && path.substr(path.length() - 5) == ".html") return "text/html; charset=utf-8";
    if (path.length() >= 4 && path.substr(path.length() - 4) == ".css") return "text/css; charset=utf-8";
    if (path.length() >= 3 && path.substr(path.length() - 3) == ".js") return "application/javascript; charset=utf-8";
    if (path.length() >= 4 && path.substr(path.length() - 4) == ".png") return "image/png";
    if (path.length() >= 4 && path.substr(path.length() - 4) == ".jpg") return "image/jpeg";
    if (path.length() >= 5 && path.substr(path.length() - 5) == ".jpeg") return "image/jpeg";
    if (path.length() >= 4 && path.substr(path.length() - 4) == ".txt") return "text/plain; charset=utf-8";
    return "application/octet-stream";
}

// Reusable business handler for removing a student from active maps, trees, and rooms
void handleDeleteStudent(int clientSocket, const string& sid) {
    Student* s = findStudentById(sid);
    if (!s) {
        sendJsonError(clientSocket, 404, "Student ID not found.");
        return;
    }
    if (s->isGraduated) {
        sendJsonError(clientSocket, 400, "Cannot delete already graduated students.");
        return;
    }
    
    vacateStudentFromRoom(s);
    logCheckout(s, "removed by admin");
    studentTree.removeStudent(s);
    allStudents.erase(sid);
    delete s;
    rebuildWaitlist();
    saveToTxt();
    
    sendHttpResponse(clientSocket, 200, "application/json", "{\"success\": true}");
}

// ============================================================================
// SECTION 15: HTTP ROUTER & CONNECTION HANDLER
// ============================================================================

void handleConnection(int clientSocket) {
    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    ssize_t bytesRead = ::recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) {
        ::close(clientSocket);
        return;
    }
    
    string requestStr(buffer, bytesRead);
    
    size_t firstSpace = requestStr.find(' ');
    if (firstSpace == string::npos) { ::close(clientSocket); return; }
    string method = requestStr.substr(0, firstSpace);
    
    size_t secondSpace = requestStr.find(' ', firstSpace + 1);
    if (secondSpace == string::npos) { ::close(clientSocket); return; }
    string path = requestStr.substr(firstSpace + 1, secondSpace - (firstSpace + 1));
    
    if (method == "OPTIONS") {
        stringstream ss;
        ss << "HTTP/1.1 200 OK\r\n"
           << "Access-Control-Allow-Origin: *\r\n"
           << "Access-Control-Allow-Methods: GET, POST, OPTIONS, DELETE\r\n"
           << "Access-Control-Allow-Headers: Content-Type\r\n"
           << "Connection: close\r\n\r\n";
        string resp = ss.str();
        ::send(clientSocket, resp.c_str(), resp.length(), 0);
        ::close(clientSocket);
        return;
    }
    
    size_t qMark = path.find('?');
    string queryString = "";
    if (qMark != string::npos) {
        queryString = path.substr(qMark + 1);
        path = path.substr(0, qMark);
    }
    
    size_t contentLength = 0;
    size_t clPos = requestStr.find("Content-Length:");
    if (clPos != string::npos) {
        size_t clValStart = clPos + 15;
        size_t clValEnd = requestStr.find("\r\n", clValStart);
        if (clValEnd != string::npos) {
            string clStr = requestStr.substr(clValStart, clValEnd - clValStart);
            clStr.erase(0, clStr.find_first_not_of(" \t"));
            clStr.erase(clStr.find_last_not_of(" \t") + 1);
            try { contentLength = stoi(clStr); } catch (...) {}
        }
    }
    
    size_t dblRn = requestStr.find("\r\n\r\n");
    string body = "";
    if (dblRn != string::npos) {
        body = requestStr.substr(dblRn + 4);
    }
    
    if (contentLength > body.length()) {
        size_t remaining = contentLength - body.length();
        while (remaining > 0) {
            char chunk[4096];
            memset(chunk, 0, sizeof(chunk));
            ssize_t chunkRead = ::recv(clientSocket, chunk, min(sizeof(chunk) - 1, remaining), 0);
            if (chunkRead <= 0) break;
            body.append(chunk, chunkRead);
            remaining -= chunkRead;
        }
    }
    
    lock_guard<mutex> lock(dataMutex);
    
    if (method == "GET") {
        if (path == "/" || path == "/index.html") {
            serveFile("frontend/index.html", "text/html; charset=utf-8", clientSocket);
        } else if (path == "/style.css") {
            serveFile("frontend/style.css", "text/css; charset=utf-8", clientSocket);
        } else if (path == "/script.js") {
            serveFile("frontend/script.js", "application/javascript; charset=utf-8", clientSocket);
        } else if (path == "/api/status") {
            int cap = 0, occ = 0;
            for (Room& r : globalRooms) { cap += r.capacity; occ += r.occupancy; }
            double avgComp = compatibilityChecks ? (totalCompatibilityScore / compatibilityChecks) : 0.0;
            
            stringstream ss;
            ss << "{"
               << "\"totalRooms\":" << globalRooms.size() << ","
               << "\"totalCapacity\":" << cap << ","
               << "\"occupancy\":" << occ << ","
               << "\"occupancyRate\":" << (cap ? (occ * 100.0 / cap) : 0.0) << ","
               << "\"totalAllocations\":" << totalAllocations << ","
               << "\"failedAllocations\":" << failedAllocations << ","
               << "\"waitlistSize\":" << waitQueue.size() << ","
               << "\"successfulSwaps\":" << successfulSwaps << ","
               << "\"avgCompatibility\":" << avgComp
               << "}";
            sendHttpResponse(clientSocket, 200, "application/json", ss.str());
        } else if (path == "/api/students") {
            stringstream ss;
            ss << "[";
            bool first = true;
            for (auto const& p : allStudents) {
                if (!first) ss << ",";
                ss << studentToJson(p.second);
                first = false;
            }
            ss << "]";
            sendHttpResponse(clientSocket, 200, "application/json", ss.str());
        } else if (path == "/api/rooms") {
            stringstream ss;
            ss << "[";
            bool first = true;
            for (size_t i = 0; i < globalRooms.size(); i++) {
                if (!first) ss << ",";
                ss << roomToJson(globalRooms[i]);
                first = false;
            }
            ss << "]";
            sendHttpResponse(clientSocket, 200, "application/json", ss.str());
        } else if (path == "/api/settings") {
            stringstream ss;
            ss << "{\"allowUpgrade\":" << (allowUpgrade ? "true" : "false") << "}";
            sendHttpResponse(clientSocket, 200, "application/json", ss.str());
        } else if (path == "/api/waitlist") {
            sendHttpResponse(clientSocket, 200, "application/json", getWaitlistJson());
        } else if (path == "/api/preview-swap") {
            string startId = "";
            size_t pos = queryString.find("startId=");
            if (pos != string::npos) {
                startId = queryString.substr(pos + 8);
            }
            if (startId.empty()) {
                sendJsonError(clientSocket, 400, "Missing startId", true);
                return;
            }
            if (!findStudentById(startId)) {
                sendJsonError(clientSocket, 404, "Student not found", true);
                return;
            }
            auto cycles = swapGraph.findCycles(startId);
            if (cycles.empty()) {
                sendHttpResponse(
                    clientSocket,
                    200,
                    "application/json",
                    "{\"success\":true,\"chain\":[]}"
                );
                return;
            }
            vector<string> chain = cycles[0];
            stringstream ss;
            ss << "{";
            ss << "\"success\":true,";
            ss << "\"chain\":[";
            for (size_t i = 0; i < chain.size(); i++) {
                if (i > 0)
                    ss << ",";
                ss << "\"" << chain[i] << "\"";
            }
            ss << ",\"" << chain[0] << "\"";
            ss << "]";
            ss << "}";
            sendHttpResponse(
                clientSocket,
                200,
                "application/json",
                ss.str()
            );
        } else if (path == "/api/reports") {
            string reportFile = "room_allocations";
            size_t fileParam = queryString.find("file=");
            if (fileParam != string::npos) {
                size_t amp = queryString.find("&", fileParam);
                reportFile = queryString.substr(fileParam + 5, amp == string::npos ? string::npos : amp - (fileParam + 5));
            }
            string reportPath = "exported_reports/" + reportFile + ".txt";
            serveFile(reportPath, "text/plain; charset=utf-8", clientSocket);
        } else if (path.rfind("/assets/", 0) == 0) {
            string assetPath = path.substr(1);
            serveFile(assetPath, getContentType(assetPath), clientSocket);
        } else {
            sendHttpResponse(clientSocket, 404, "text/plain", "404 Not Found");
        }
    } else if (method == "POST") {
        if (path == "/api/login") {
            string username = parseJsonString(body, "username");
            string password = parseJsonString(body, "password");
            if (username == "admin" && password == "staywise2026") {
                sendHttpResponse(clientSocket, 200, "application/json", "{\"success\": true, \"token\": \"StaywiseSessionToken2026\"}");
            } else {
                sendHttpResponse(clientSocket, 401, "application/json", "{\"success\": false, \"error\": \"Invalid username or password.\"}");
            }
            return;
        }
        
        // Block other POST operations if session isn't admin
        if (!isAdminAuthenticated(requestStr)) {
            sendHttpResponse(clientSocket, 401, "application/json", "{\"error\": \"Unauthorized. Admin session required.\"}");
            return;
        }
        
        if (path == "/api/students") {
            string id = parseJsonString(body, "id");
            string name = parseJsonString(body, "name");
            string gStr = parseJsonString(body, "gender");
            char gender = (gStr.length() == 1) ? toupper(gStr[0]) : 'M';
            double cgpa = parseJsonDouble(body, "cgpa");
            int seniority = parseJsonInt(body, "seniority");
            bool specialNeeds = parseJsonBool(body, "specialNeeds");
            int roomPref = parseJsonInt(body, "roomPref");
            bool wantsAC = parseJsonBool(body, "wantsAC");
            bool wantsMess = parseJsonBool(body, "wantsMess");
            int foodPreference = parseJsonInt(body, "foodPreference");
            int distance = parseJsonInt(body, "distance");
            bool prevPattern = parseJsonBool(body, "prevPattern");
            int sleepHabit = parseJsonInt(body, "sleepHabit");
            int cleanlinessHabit = parseJsonInt(body, "cleanlinessHabit");
            int studyHabit = parseJsonInt(body, "studyHabit");
            int familyBackgroundRisk = parseJsonInt(body, "familyBackgroundRisk");
            
            if (!isValidStudentID(id, gender)) {
                string format = (gender == 'M') ? "SM##" : "SF##";
                sendJsonError(clientSocket, 400, "Invalid Student ID format. Use format like " + format + ".");
                return;
            }
            
            if (findStudentById(id)) {
                sendJsonError(clientSocket, 400, "Student ID " + id + " already exists. Duplicate IDs are not allowed.");
                return;
            }
            
            Student s;
            s.id = id;
            s.name = name;
            s.gender = gender;
            s.cgpa = cgpa;
            s.seniority = seniority;
            s.currentYear = seniority;
            s.specialNeeds = specialNeeds;
            s.roomPref = roomPref;
            s.wantsAC = wantsAC;
            s.wantsMess = wantsMess;
            s.foodPreference = foodPreference;
            s.distance = distance;
            s.prevPattern = prevPattern;
            s.familyBackgroundRisk = familyBackgroundRisk;
            s.habits[0] = sleepHabit;
            s.habits[1] = cleanlinessHabit;
            s.habits[2] = studyHabit;
            s.inWaitlist = true;
            
            addStudentRaw(s);
            rebuildWaitlist();
            saveToTxt();
            
            stringstream ss;
            ss << "{\"success\": true, \"student\": " << studentToJson(allStudents[id]) << "}";
            sendHttpResponse(clientSocket, 201, "application/json", ss.str());
        } else if (path == "/api/run-allocation") {
            sendActionLogResponse(clientSocket, runAllocation());
        } else if (path == "/api/recover-vacancies") {
            sendActionLogResponse(clientSocket, recoverVacancies());
        } else if (path == "/api/compatibility") {
            string id1 = parseJsonString(body, "id1");
            string id2 = parseJsonString(body, "id2");
            Student* s1 = findStudentById(id1);
            Student* s2 = findStudentById(id2);
            if (!s1 || !s2) {
                sendJsonError(clientSocket, 404, "One or both student IDs not found in the system.");
                return;
            }
            int score = getCompatibility(s1, s2);
            stringstream ss;
            ss << "{"
               << "\"id1\":\"" << id1 << "\","
               << "\"name1\":\"" << s1->name << "\","
               << "\"id2\":\"" << id2 << "\","
               << "\"name2\":\"" << s2->name << "\","
               << "\"score\":" << score << ","
               << "\"risk\":\"" << (score < 40 ? "High Risk" : "Safe") << "\""
               << "}";
            sendHttpResponse(clientSocket, 200, "application/json", ss.str());
        } else if (path == "/api/add-swap") {
            string id1 = parseJsonString(body, "id1");
            string id2 = parseJsonString(body, "id2");
            Student* s1 = findStudentById(id1);
            Student* s2 = findStudentById(id2);
            if (!s1 || !s2) {
                sendJsonError(clientSocket, 404, "One or both student IDs not found in the system.");
                return;
            }
            if (s1->gender != s2->gender) {
                sendJsonError(clientSocket, 400, "Cross-gender swaps are not permitted.");
                return;
            }
            swapGraph.addEdge(id1, id2);
            saveToTxt();
            sendHttpResponse(clientSocket, 200, "application/json", "{\"success\": true, \"message\": \"Swap connection added successfully.\"}");
        } else if (path == "/api/process-swap") {
            string startId = parseJsonString(body, "startId");
            sendActionLogResponse(clientSocket, processRoomSwaps(startId));
        } else if (path == "/api/undo-swap") {
            sendActionLogResponse(clientSocket, undoLastSwap());
        } else if (path == "/api/update-student") {
            string sid = parseJsonString(body, "id");
            Student* s = findStudentById(sid);
            if (!s) {
                sendJsonError(clientSocket, 404, "Student ID not found.");
                return;
            }
            int prevYear = s->seniority;
            int prevSemester = s->semester;
            
            string name = parseJsonString(body, "name");
            if (!name.empty()) s->name = name;

            if (body.find("\"roomPref\"") != string::npos) {
                s->roomPref = parseJsonInt(body, "roomPref");
            }
            if (body.find("\"wantsAC\"") != string::npos) {
                s->wantsAC = parseJsonBool(body, "wantsAC");
            }
            if (body.find("\"wantsMess\"") != string::npos) {
                s->wantsMess = parseJsonBool(body, "wantsMess");
            }
            if (body.find("\"foodPreference\"") != string::npos) {
                s->foodPreference = parseJsonInt(body, "foodPreference");
            }
            if (body.find("\"sleepHabit\"") != string::npos) {
                s->habits[0] = parseJsonInt(body, "sleepHabit");
            }
            if (body.find("\"cleanlinessHabit\"") != string::npos) {
                s->habits[1] = parseJsonInt(body, "cleanlinessHabit");
            }
            if (body.find("\"studyHabit\"") != string::npos) {
                s->habits[2] = parseJsonInt(body, "studyHabit");
            }
            if (body.find("\"examPassed\"") != string::npos) {
                s->examPassed = parseJsonBool(body, "examPassed");
            }
            if (body.find("\"hasBacklog\"") != string::npos) {
                s->hasBacklog = parseJsonBool(body, "hasBacklog");
            }
            if (body.find("\"placementExtension\"") != string::npos) {
                s->placementExtension = parseJsonBool(body, "placementExtension");
            }
            if (body.find("\"finalSemCgpa\"") != string::npos) {
                double fc = parseJsonDouble(body, "finalSemCgpa");
                if (fc >= 0.0 && fc <= 10.0) s->finalSemesterCGPA = fc;
            }

            // Update CGPA with AVL Rebalancing
            if (body.find("\"cgpa\"") != string::npos) {
                double nc = parseJsonDouble(body, "cgpa");
                if (nc > 0.0 && nc <= 10.0 && nc != s->cgpa) {
                    studentTree.removeStudent(s);
                    s->cgpa = nc;
                    studentTree.addStudent(s);
                }
            }

            if (body.find("\"seniority\"") != string::npos) {
                int sen = parseJsonInt(body, "seniority");
                if (sen >= 1 && sen <= 4) { s->seniority = sen; s->currentYear = sen; }
            }
            if (body.find("\"semester\"") != string::npos) {
                int sem = parseJsonInt(body, "semester");
                if (sem >= 1 && sem <= 8) s->semester = sem;
            }
            if (body.find("\"familyBackgroundRisk\"") != string::npos) {
                s->familyBackgroundRisk = parseJsonInt(body, "familyBackgroundRisk");
            }
            
            // Clear compatibility DP cache entries related to this student
            for (auto it = dpCompatMemo.begin(); it != dpCompatMemo.end(); ) {
                if (it->first.rfind(s->id + "_", 0) == 0 || 
                    (it->first.size() >= s->id.size() + 1 && it->first.substr(it->first.size() - s->id.size() - 1) == "_" + s->id)) {
                    it = dpCompatMemo.erase(it);
                } else {
                    ++it;
                }
            }
            calcRisk(s);
            
            // --- Auto-Promotion Workflow ---
            bool promoted = false;
            bool graduationEligible = false;
            string promoMsg = "";
            
            if (s->examPassed && !s->hasBacklog) {
                if (s->seniority < 4) {
                    s->seniority++;
                    s->semester = (s->seniority - 1) * 2 + 1;
                    promoted = true;
                    promoMsg = "Promoted to Year " + to_string(s->seniority);
                    s->examPassed = false;
                    s->hasBacklog = false;
                    s->finalSemesterCGPA = 0.0;
                } else if (s->seniority == 4 && !s->placementExtension) {
                    graduationEligible = true;
                    promoMsg = "Eligible for graduation checkout";
                } else {
                    promoMsg = "Year 4 complete but retained (placement extension)";
                }
            } else {
                string reasons;
                if (!s->examPassed) reasons += "Exam pending/failed";
                if (s->hasBacklog) {
                    if (!reasons.empty()) reasons += ", ";
                    reasons += "Has backlog";
                }
                promoMsg = "Academic state updated (" + reasons + ")";
            }
            
            rebuildWaitlist();
            saveToTxt();
            
            stringstream ss;
            ss << "{\"success\":true,\"student\":" << studentToJson(s) << ","
               << "\"promotion\":{" 
               << "\"promoted\":" << (promoted ? "true" : "false") << ","
               << "\"graduationEligible\":" << (graduationEligible ? "true" : "false") << ","
               << "\"previousYear\":" << prevYear << ","
               << "\"newYear\":" << s->seniority << ","
               << "\"previousSemester\":" << prevSemester << ","
               << "\"newSemester\":" << s->semester << ","
               << "\"message\":\"" << escapeJsonString(promoMsg) << "\""
               << "}}";
            sendHttpResponse(clientSocket, 200, "application/json", ss.str());
        } else if (path == "/api/delete-student") {
            string sid = parseJsonString(body, "id");
            handleDeleteStudent(clientSocket, sid);
        } else if (path == "/api/semester-renewal") {
            sendActionLogResponse(clientSocket, semesterRenewal());
        } else if (path == "/api/graduation-checkout") {
            stringstream log;
            log << "[Graduation Checkout Process Started]\n";
            int graduated = 0, skippedSem = 0, skippedFailed = 0, skippedBacklog = 0, skippedExt = 0;
            
            for (auto& p : allStudents) {
                Student* s = p.second;
                if (s->isGraduated) continue;
                if (s->semester < 8) { if (s->seniority == 4) skippedSem++; continue; }
                if (!s->examPassed) { skippedFailed++; continue; }
                if (s->hasBacklog) { skippedBacklog++; continue; }
                if (s->placementExtension) { skippedExt++; continue; }
                
                // Qualified checkout
                vacateStudentFromRoom(s);
                logCheckout(s, "Graduation Checkout");
                s->isGraduated = true;
                s->isAllocated = false;
                s->currentRoom = "";
                s->inWaitlist = false;
                s->waitReason = "Graduated";
                graduated++;
                log << "Graduated: " << s->id << " - " << s->name << "\n";
            }
            
            rebuildWaitlist();
            
            int promoted = 0;
            vector<Student*> tq;
            while (!waitQueue.empty()) { tq.push_back(waitQueue.top()); waitQueue.pop(); }
            for (Student* s : tq) {
                if (s->isGraduated || s->isAllocated) continue;
                if (tryAllocate(s, true) || (allowUpgrade && tryAllocate(s, false))) {
                    promoted++;
                    log << "Promoted: " << s->id << " (" << s->name << ") -> Room " << s->currentRoom << "\n";
                } else {
                    waitQueue.push(s);
                }
            }
            
            rebuildWaitlist();
            saveToTxt();
            
            stringstream ss;
            ss << "{\"success\":true,"
               << "\"graduated\":" << graduated << ","
               << "\"promoted\":" << promoted << ","
               << "\"skippedFailed\":" << skippedFailed << ","
               << "\"skippedBacklog\":" << skippedBacklog << ","
               << "\"skippedExtension\":" << skippedExt << ","
               << "\"skippedSemester\":" << skippedSem << ","
               << "\"log\":\"" << escapeJsonString(log.str()) << "\"}";
            sendHttpResponse(clientSocket, 200, "application/json", ss.str());
        } else if (path == "/api/manual-assign") {
            string sid = parseJsonString(body, "studentId");
            string rid = parseJsonString(body, "roomId");
            
            Student* s = findStudentById(sid);
            if (!s) {
                sendJsonError(clientSocket, 404, "Student ID not found.");
                return;
            }
            if (s->isGraduated) {
                sendJsonError(clientSocket, 400, "Student is already graduated.");
                return;
            }
            if (s->isAllocated) {
                sendJsonError(clientSocket, 400, "Student is already allocated.");
                return;
            }
            
            Room* targetRoom = findRoomById(rid);
            if (!targetRoom) {
                sendJsonError(clientSocket, 404, "Room ID not found.");
                return;
            }
            if (targetRoom->occupancy >= targetRoom->capacity) {
                sendJsonError(clientSocket, 400, "Target room is fully occupied.");
                return;
            }
            if (s->gender != targetRoom->gender) {
                sendJsonError(clientSocket, 400, "Gender mismatch between student and room.");
                return;
            }
            
            // Check compatibility with existing roommates
            if (targetRoom->head) {
                Student* roommate = targetRoom->head;
                while (roommate) {
                    int score = getCompatibility(s, roommate);
                    if (score < 50) {
                        sendJsonError(clientSocket, 400, "Compatibility score with existing roommate " + roommate->name + " is too low (" + to_string(score) + "%).");
                        return;
                    }
                    roommate = roommate->next;
                }
            }
            
            s->inWaitlist = false;
            if (!targetRoom->allocate(s)) {
                sendJsonError(clientSocket, 400, "Failed to allocate student to room.");
                return;
            }
            
            rebuildWaitlist();
            saveToTxt();
            
            stringstream ss;
            ss << "{\"success\": true, \"message\": \"" << s->id << " manually assigned to Room " << targetRoom->id << "\"}";
            sendHttpResponse(clientSocket, 200, "application/json", ss.str());
        } else if (path == "/api/settings") {
            bool upgradeVal = false;
            if (body.find("\"allowUpgrade\"") != string::npos) {
                string valStr = parseJsonString(body, "allowUpgrade");
                if (valStr == "true" || valStr == "1") {
                    upgradeVal = true;
                } else {
                    size_t pos = body.find("\"allowUpgrade\"");
                    size_t colon = body.find(":", pos);
                    if (colon != string::npos) {
                        size_t nextVal = body.find_first_not_of(" \t\r\n", colon + 1);
                        if (nextVal != string::npos && body.substr(nextVal, 4) == "true") {
                            upgradeVal = true;
                        }
                    }
                }
            }
            allowUpgrade = upgradeVal;
            
            ofstream out("sample_data/settings.txt");
            if (out.is_open()) {
                out << "Allow automatic room upgrade: " << (allowUpgrade ? "1" : "0") << "\n";
            }
            
            stringstream ss;
            ss << "{\"success\":true,\"allowUpgrade\":" << (allowUpgrade ? "true" : "false") << "}";
            sendHttpResponse(clientSocket, 200, "application/json", ss.str());
        } else {
            sendHttpResponse(clientSocket, 404, "text/plain", "404 Not Found");
        }
    } else if (method == "DELETE") {
        if (path == "/api/students") {
            if (!isAdminAuthenticated(requestStr)) {
                sendHttpResponse(clientSocket, 401, "application/json", "{\"error\": \"Unauthorized. Admin session required.\"}");
                return;
            }
            string sid = "";
            size_t idParam = queryString.find("id=");
            if (idParam != string::npos) {
                size_t amp = queryString.find('&', idParam);
                sid = queryString.substr(idParam + 3, amp == string::npos ? string::npos : amp - (idParam + 3));
            }
            if (sid.empty()) sid = parseJsonString(body, "id");
            
            handleDeleteStudent(clientSocket, sid);
        } else {
            sendHttpResponse(clientSocket, 404, "text/plain", "404 Not Found");
        }
    } else {
        sendHttpResponse(clientSocket, 405, "text/plain", "405 Method Not Allowed");
    }
}

// ============================================================================
// SECTION 16: WEB SERVER SOCKET INITIALIZATION
// ============================================================================

void runHttpServer(int port) {
    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) {
        cerr << "[Error] Failed to create socket.\n";
        return;
    }
    
    int opt = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    if (socket_bind(serverFd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        cerr << "[Error] Failed to bind to port " << port << ".\n";
        ::close(serverFd);
        return;
    }
    
    if (::listen(serverFd, 20) < 0) {
        cerr << "[Error] Failed to listen on socket.\n";
        ::close(serverFd);
        return;
    }
    
    cout << "\n==================================================================\n";
    cout << "  🚀 Smart Campus Hostel Web Server is up and running!\n";
    cout << "  🔗 Access URL: http://localhost:" << port << "\n";
    cout << "==================================================================\n\n";
    
    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = ::accept(serverFd, (struct sockaddr*)&clientAddr, &clientLen);
        if (clientSocket >= 0) {
            thread(handleConnection, clientSocket).detach();
        }
    }
    ::close(serverFd);
}

// ============================================================================
// SECTION 17: SYSTEM ENTRY POINT (MAIN)
// ============================================================================

int main() {
    // 1. Create file directory structures
    system("mkdir -p exported_reports sample_data");
    
    // 2. Load databases
    cout << "[System Initializing] Loading static database records... ";
    loadData();
    cout << "Done.\n";
    
    cout << "[System Initializing] Running auto-allocation on startup... ";
    runAllocation();
    cout << "Done.\n";
    
    // 3. Start C++ Socket Web Server on Port 8080
    runHttpServer(8080);
    
    // Cleanup heap allocations on termination
    for (auto &p : allStudents) delete p.second;
    allStudents.clear();
    
    return 0;
}
