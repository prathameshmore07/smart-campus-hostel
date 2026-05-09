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
using namespace std;

int totalAllocations = 0;
int failedAllocations = 0;
int successfulSwaps = 0;
int compatibilityChecks = 0;
double totalCompatibilityScore = 0.0;

void printHeader(string title) { 
    cout << "\n======================================================\n";
    cout << "  " << title << "\n";
    cout << "======================================================\n"; 
}

// Forward declaration for duplicate detection in validation functions
struct Student;
extern map<string, Student*> allStudents;

// ----- Terminal Input Validation System -----
// Reusable validation functions
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

// ----- Gender-Based Student ID Validation -----
// Valid formats: SM01, SM25, SM103 (Male) | SF01, SF25, SF210 (Female)
// Rules: Must start with SM/SF, followed by 1+ digits only. Case-sensitive.
bool isValidStudentID(const string& id, char gender) {
    if (id.length() < 3) return false; // Minimum: prefix(2) + 1 digit
    string prefix = id.substr(0, 2);
    if (gender == 'M' && prefix != "SM") return false;
    if (gender == 'F' && prefix != "SF") return false;
    string numeric = id.substr(2);
    if (numeric.empty()) return false;
    for (char c : numeric) { if (!isdigit(c)) return false; }
    return true;
}

// General format check (no gender context) - accepts both SM and SF prefixes
bool isValidStudentIDFormat(const string& id) {
    if (id.length() < 3) return false;
    string prefix = id.substr(0, 2);
    if (prefix != "SM" && prefix != "SF") return false;
    string numeric = id.substr(2);
    if (numeric.empty()) return false;
    for (char c : numeric) { if (!isdigit(c)) return false; }
    return true;
}

// Extracts expected gender from a valid student ID
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

string getValidatedString(string p) {
    string v; 
    while (true) { 
        cout << p; 
        if (cin >> v) { cin.clear(); cin.ignore(10000, '\n'); return v; } 
        cin.clear(); cin.ignore(10000, '\n'); cout << "[Error] Invalid input.\n"; 
    }
}

string getValidatedAlpha(string p) {
    string v; 
    while (true) { 
        cout << p; 
        getline(cin >> ws, v);
        if (isAlphaOnly(v)) return v; 
        cout << "[Error] Invalid input. Must contain only alphabets and spaces.\n"; 
    }
}

string getValidatedAlphanum(string p) {
    string v; 
    while (true) { 
        cout << p; 
        if (cin >> v) { 
            cin.clear(); cin.ignore(10000, '\n'); 
            if (isAlphanumericOnly(v)) return v;
        } 
        cin.clear(); cin.ignore(10000, '\n'); cout << "[Error] Invalid input. Must contain only alphanumeric characters.\n"; 
    }
}

// Gender-aware student ID input with duplicate detection
string getValidatedStudentID(string p, char gender) {
    string v;
    while (true) {
        cout << p;
        if (cin >> v) {
            cin.clear(); cin.ignore(10000, '\n');
            if (!isValidStudentID(v, gender)) {
                cout << "[Error] Invalid Student ID format. Use formats like " << (gender == 'M' ? "SM01" : "SF01") << ".\n";
                continue;
            }
            if (allStudents.count(v)) {
                cout << "[Error] Student ID " << v << " already exists. Duplicate IDs are not allowed.\n";
                continue;
            }
            return v;
        }
        cin.clear(); cin.ignore(10000, '\n'); cout << "[Error] Invalid input.\n";
    }
}

// General student ID input (accepts SM/SF, no gender check, no duplicate check)
string getValidatedStudentIDLookup(string p) {
    string v;
    while (true) {
        cout << p;
        if (cin >> v) {
            cin.clear(); cin.ignore(10000, '\n');
            if (!isValidStudentIDFormat(v)) {
                cout << "[Error] Invalid Student ID format. Use formats like SM01 or SF01.\n";
                continue;
            }
            return v;
        }
        cin.clear(); cin.ignore(10000, '\n'); cout << "[Error] Invalid input.\n";
    }
}

int getValidatedInt(string p) {
    string input;
    while (true) { 
        cout << p; 
        if (cin >> input) { 
            cin.clear(); cin.ignore(10000, '\n'); 
            if (isNumericOnly(input, false, true)) {
                try {
                    return stoi(input);
                } catch(...) {}
            }
        } 
        cin.clear(); cin.ignore(10000, '\n'); cout << "[Error] Invalid input. Must be a valid integer.\n"; 
    }
}

int getValidatedChoice(string p, int mn, int mx) {
    while (true) { 
        int v = getValidatedInt(p); 
        if (v >= mn && v <= mx) return v; 
        cout << "[Error] Invalid input. Must be between " << mn << " and " << mx << ".\n"; 
    }
}

double getValidatedDouble(string p, double mn, double mx) {
    string input;
    while (true) { 
        cout << p; 
        if (cin >> input) { 
            cin.clear(); cin.ignore(10000, '\n'); 
            if (isNumericOnly(input, true, true)) {
                try {
                    double v = stod(input);
                    double r = round(v * 100.0) / 100.0; 
                    if (v >= mn && v <= mx && abs(v - r) < 1e-6) return v; 
                    cout << "[Error] Range " << mn << "-" << mx << ", max 2 decimals.\n"; 
                    continue;
                } catch(...) {}
            }
        } 
        cin.clear(); cin.ignore(10000, '\n'); cout << "[Error] Invalid number.\n"; 
    }
}

char getValidatedGender(string p) {
    char v; string input;
    while (true) { 
        cout << p; 
        if (cin >> input) { 
            cin.clear(); cin.ignore(10000, '\n'); 
            if (input.length() == 1) {
                v = toupper(input[0]); 
                if (v == 'M' || v == 'F') return v; 
            }
        } 
        cin.clear(); cin.ignore(10000, '\n'); cout << "[Error] Enter 'M' or 'F'.\n"; 
    }
}

// ----- Structures & Models -----
struct Student {
    string id, name, currentRoom = "", waitReason = "Pending Allocation";
    char gender; 
    double cgpa;
    int seniority, roomPref, distance, prevPattern, habits[3], foodPreference;
    bool wantsMess, wantsAC, specialNeeds;
    bool isAllocated = false, inWaitlist = false, isGraduated = false;
    int waitTime = 0, rejections = 0, fairnessScore = 0, riskLevel = 0;
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
        // [Hostel Rule Validation]: Normal allocation, waitlist promotion, and vacancy recovery
        // Immediate rejection of mixed-gender room assignments.
        if (s->gender != gender) {
            cout << "\n[Error] This violates hostel rules and regulations.\n";
            return false;
        }
        if (s->isAllocated || occupancy >= capacity) return false;
        s->next = head; head = s; occupancy++; s->isAllocated = true; s->currentRoom = id;
        s->waitReason = "Allocated Successfully"; totalAllocations++; return true;
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

vector<Room> globalRooms; 
map<string, Student*> allStudents; 
map<string, Room*> typeHeads;

// ----- DSA 1: AVL Tree for Student Ranking -----
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
    
    void inord(BSTNode* n, vector<Student*>& l) { 
        if (!n) return; 
        inord(n->left, l); l.push_back(n->student); inord(n->right, l); 
    }
    
    // Memory Management Improvement: Recursive cleanup function
    void freeMemory(BSTNode* node) {
        if (!node) return;
        freeMemory(node->left);
        freeMemory(node->right);
        delete node; // Free BSTNode memory safely (Student* pointers are cleaned up globally)
    }
public:
    ~StudentAVLTree() { freeMemory(root); root = nullptr; }
    void addStudent(Student* s) { root = ins(root, s); }
    vector<Student*> getSorted() { vector<Student*> l; inord(root, l); return l; }
} studentTree;

void addStudentRaw(Student sData) { 
    Student* s = new Student(sData); studentTree.addStudent(s); allStudents[s->id] = s; 
}

// ----- DSA 2: Dynamic Programming (Memoization) -----
// Memoization is used as top-down dynamic programming.
// Overlapping subproblems are cached.
// Risk calculations and compatibility calculations use memoization optimization.
map<string, double> dpRiskMemo; // Caches risk calculations
map<string, int> dpCompatMemo;  // Caches roommate compatibility calculations

void calcRisk(Student* s) {
    string key = to_string(s->distance) + "_" + to_string(s->prevPattern);
    if (!dpRiskMemo.count(key)) dpRiskMemo[key] = min(1.0, 0.05 + (s->distance > 100 ? 0.20 : 0) + (s->prevPattern == 1 ? 0.35 : 0));
    double p = dpRiskMemo[key]; s->riskLevel = p < 0.3 ? 0 : (p < 0.6 ? 1 : 2);
}

int getCompatibility(Student* s1, Student* s2) {
    if (!s1 || !s2) return 0;
    string key = s1->id < s2->id ? s1->id + "_" + s2->id : s2->id + "_" + s1->id;
    if (dpCompatMemo.count(key)) return dpCompatMemo[key]; // Retrieve cached state
    
    int sc = 100; 
    // Improved Compatibility Scoring: Realistic Weighted Penalties
    sc -= abs(s1->habits[0] - s2->habits[0]) * 10;  // Sleep habits impact (Medium penalty)
    sc -= abs(s1->habits[1] - s2->habits[1]) * 15; // Cleanliness mismatch (Higher penalty)
    sc -= abs(s1->habits[2] - s2->habits[2]) * 10; // Study environment mismatch (Medium penalty)
    
    // Food Preference conflict mapping
    if (s1->wantsMess && s2->wantsMess && s1->foodPreference != s2->foodPreference) sc -= 20; // Additional penalty
    
    sc = max(0, min(100, sc)); // Ensure safe 0-100 bounds
    
    compatibilityChecks++; 
    totalCompatibilityScore += sc; 
    return dpCompatMemo[key] = sc; // Cache the new optimization result
}

// ----- DSA 3: Priority Queue for Waitlist -----
struct WaitlistComp {
    bool operator()(Student* a, Student* b) { 
        return ((a->waitTime * 2.0) + (a->fairnessScore * 1.5) + (a->rejections * 1.0) + (a->seniority * 0.5) + (a->specialNeeds ? 50.0 : 0) - (a->riskLevel * 5.0)) <
               ((b->waitTime * 2.0) + (b->fairnessScore * 1.5) + (b->rejections * 1.0) + (b->seniority * 0.5) + (b->specialNeeds ? 50.0 : 0) - (b->riskLevel * 5.0));
    }
};

priority_queue<Student*, vector<Student*>, WaitlistComp> waitQueue; 
queue<Student*> fairnessQueue; 

// ----- DSA 4: Stack for Undo Swaps -----
struct SwapAction { string id1, id2, room1, room2; };
stack<SwapAction> undoStack; 

// ----- Allocation Logic -----
bool isCompatible(Room* r, Student* newS) {
    if (r->gender != newS->gender) return false;
    for (Student* s = r->head; s; s = s->next) if (getCompatibility(s, newS) < 40) return false; 
    return true;
}

bool tryAllocate(Student* s, bool strict) {
    if (strict) {
        for (Room* r = typeHeads[string(1, s->gender) + "_" + to_string(s->roomPref)]; r; r = r->nextTypeRoom)
            if (r->isAC == s->wantsAC && r->occupancy < r->capacity && isCompatible(r, s) && r->allocate(s)) { s->inWaitlist = false; return true; }
        s->waitReason = (s->waitReason == "Pending Allocation") ? "Preferred Room Unavailable" : "Compatibility Conflict";
        return false;
    }
    vector<Room*> avail;
    for (Room& r : globalRooms) if (r.gender == s->gender && r.occupancy < r.capacity && r.isAC == s->wantsAC) avail.push_back(&r);
    sort(avail.begin(), avail.end(), [](Room* a, Room* b) { return (a->capacity - a->occupancy) > (b->capacity - b->occupancy); });
    for (Room* r : avail) if (isCompatible(r, s) && r->allocate(s)) { s->inWaitlist = false; return true; }
    s->waitReason = avail.empty() ? "No Rooms Available" : "High-Risk Compatibility Conflict"; return false;
}

// ----- DSA 5: Graph & DFS for Swaps -----
class SwapGraph {
    map<string, vector<string>> adj;
    void dfs(string u, string p, string start, map<string, bool>& vis, vector<string>& path, vector<vector<string>>& cycles) {
        vis[u] = true; path.push_back(u);
        for (string v : adj[u]) {
            if (v == p) continue;
            if (v == start && path.size() > 1) cycles.push_back(path); else if (!vis[v]) dfs(v, u, start, vis, path, cycles);
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

void exportReports(); void saveToTxt();

void runAllocation() {
    printHeader("Smart Allocation Process");
    vector<Student*> sorted = studentTree.getSorted();
    for (Student* s : sorted) { if (s->isGraduated) continue; calcRisk(s); }
    
    int qSz = fairnessQueue.size();
    for (int i = 0; i < qSz; i++) {
        Student* s = fairnessQueue.front(); fairnessQueue.pop(); if (s->isGraduated) continue;
        if (!s->isAllocated && !tryAllocate(s, true) && !tryAllocate(s, false)) { s->fairnessScore++; fairnessQueue.push(s); }
    }
    
    sort(sorted.begin(), sorted.end(), [](Student* a, Student* b) {
        return ((a->cgpa * 10.0) + (a->seniority * 5.0) + (a->specialNeeds ? 50.0 : 0) - (a->riskLevel * 5.0)) >
               ((b->cgpa * 10.0) + (b->seniority * 5.0) + (b->specialNeeds ? 50.0 : 0) - (b->riskLevel * 5.0));
    });
    
    for (Student* s : sorted) {
        if (s->isGraduated) continue;
        if (!s->isAllocated && !tryAllocate(s, true) && !tryAllocate(s, false) && !s->inWaitlist) {
            s->fairnessScore += ++s->rejections; s->waitTime++; fairnessQueue.push(s); waitQueue.push(s); s->inWaitlist = true; failedAllocations++;
        }
    }
    saveToTxt(); cout << "\n[Success] Allocation process completed successfully.\n";
}

void recoverVacancies() {
    printHeader("Vacancy Recovery Process");
    cout << "[Info] Scanning for high-risk allocations...\n";
    bool found = false;
    for (Room& r : globalRooms) {
        Student* curr = r.head;
        while (curr) {
            Student* nxt = curr->next; calcRisk(curr);
            if (curr->riskLevel == 2) { 
                r.removeStudent(curr->id); curr->waitReason = "Removed due to High Risk (No-Show)"; 
                curr->inWaitlist = true; waitQueue.push(curr); fairnessQueue.push(curr); found = true; 
            }
            curr = nxt;
        }
    }
    if (!found) { cout << "\n[Info] No high-risk vacancies found.\n"; return; }
    
    vector<Student*> tq; while (!waitQueue.empty()) { tq.push_back(waitQueue.top()); waitQueue.pop(); }
    for (Student* s : tq) {
        if (s->isAllocated) continue;
        if (tryAllocate(s, true) || tryAllocate(s, false)) cout << "[Recovered] Promoted " << s->id << " to Room " << s->currentRoom << "\n";
        else waitQueue.push(s);
    }
    saveToTxt(); cout << "\n[Success] Vacancy recovery completed.\n";
}

void processRoomSwaps(string start) {
    printHeader("Graph-Based Swap Execution");
    if (!allStudents.count(start) || !allStudents[start]->isAllocated) { cout << "\n[Error] Invalid student.\n"; return; }
    auto cycles = swapGraph.findCycles(start); if (cycles.empty()) { cout << "\n[Error] No valid swap chains found.\n"; return; } 
    
    vector<string> chain = cycles[0]; map<string, string> orig;
    for (size_t i=0; i<chain.size(); i++) {
        string u = chain[i], nextU = chain[(i+1)%chain.size()];
        if (!allStudents.count(u) || !allStudents[u]->isAllocated) { cout << "\n[Error] Invalid chain.\n"; return; }
        
        // [Hostel Rule Validation]: Swap chains, direct swaps, and DFS cycles
        if (allStudents[u]->gender != allStudents[nextU]->gender) { 
            cout << "\n[Error] This violates hostel rules and regulations.\n"; 
            return; 
        }
        if (orig.count(u) == 0) orig[u] = allStudents[u]->currentRoom;
    }
    
    for (string u : chain) for (Room& r : globalRooms) if (r.id == orig[u]) r.removeStudent(u);
    bool success = true;
    for (size_t i=0; i<chain.size(); i++) {
        string tRoom = orig[chain[(i+1)%chain.size()]]; Student* s = allStudents[chain[i]]; bool alloc = false;
        for (Room& r : globalRooms) if (r.id == tRoom && isCompatible(&r, s)) { r.allocate(s); alloc = true; break; }
        if (!alloc) success = false;
    }
    
    if (!success) {
        cout << "\n[Error] Swap failed. Reverting...\n";
        for (string u : chain) for (Room& r : globalRooms) { if (r.id == allStudents[u]->currentRoom) r.removeStudent(u); if (r.id == orig[u]) r.allocate(allStudents[u]); }
        return;
    }
    if (chain.size() == 2) undoStack.push({chain[0], chain[1], orig[chain[0]], orig[chain[1]]});
    successfulSwaps++; saveToTxt(); cout << "\n[Success] Room swap chain executed perfectly!\n";
}

void undoLastSwap() {
    printHeader("Undo Last Swap");
    if (undoStack.empty()) { cout << "\n[Error] No swaps to undo.\n"; return; }
    auto a = undoStack.top(); undoStack.pop(); Student *s1 = allStudents[a.id1], *s2 = allStudents[a.id2];
    for (Room& r : globalRooms) { r.removeStudent(s1->id); r.removeStudent(s2->id); }
    for (Room& r : globalRooms) { if (r.id == a.room1) r.allocate(s1); if (r.id == a.room2) r.allocate(s2); }
    saveToTxt(); cout << "\n[Success] Undid swap between " << a.id1 << " and " << a.id2 << ".\n";
}

void showAnalyticsReport() {
    printHeader("Analytics & System Diagnostics");
    int cap=0, occ=0; for (Room& r : globalRooms) { cap+=r.capacity; occ+=r.occupancy; }
    cout << left << setw(30) << "Total Capacity:" << cap << "\n" << setw(30) << "Current Occupancy:" << occ << " (" << (cap ? (occ*100.0/cap) : 0) << "%)\n" << setw(30) << "Total Allocations Made:" << totalAllocations << "\n" << setw(30) << "Failed Allocations:" << failedAllocations << "\n" << setw(30) << "Waitlist Size:" << waitQueue.size() << "\n" << setw(30) << "Successful Swaps:" << successfulSwaps << "\n" << setw(30) << "Avg Compatibility Score:" << (compatibilityChecks ? (totalCompatibilityScore/compatibilityChecks) : 0) << "%\n------------------------------------------------------\nReports successfully exported to TXT files.\n";
    exportReports();
}

// ----- File Persistence -----
void exportReports() {
    ofstream f1("exported_reports/room_allocations.txt"), f2("exported_reports/waitlist_report.txt"), f3("exported_reports/student_records.txt"), f4("exported_reports/analytics_report.txt");
    f1 << "=== ROOM ALLOCATIONS ===\n";
    for (Room& r : globalRooms) { 
        f1 << "Room " << r.id << " (" << r.gender << ") [" << r.occupancy << "/" << r.capacity << "]:\n"; 
        for(Student* s=r.head; s; s=s->next) f1 << "  -> " << s->id << " | " << s->name << " (Risk: " << s->riskLevel << ")\n"; 
    }
    f2 << "=== WAITLIST REPORT ===\n";
    auto tq = waitQueue; int rk=1; 
    while(!tq.empty()) { if (!tq.top()->isGraduated) f2 << rk++ << ". [" << tq.top()->id << "] " << tq.top()->name << " | Score: " << tq.top()->fairnessScore << " | Reason: " << tq.top()->waitReason << "\n"; tq.pop(); }
    f3 << "=== STUDENT RECORDS ===\n";
    for (auto const& p : allStudents) if (!p.second->isGraduated) f3 << left << setw(6) << p.second->id << "| " << setw(12) << p.second->name << "| Gender: " << p.second->gender << " | CGPA: " << setw(4) << p.second->cgpa << " | Room: " << (p.second->isAllocated ? p.second->currentRoom : "Waitlisted") << "\n"; 
    f4 << "=== ANALYTICS REPORT ===\nTotal Allocations: " << totalAllocations << "\nFailed Allocations: " << failedAllocations << "\nSuccessful Swaps: " << successfulSwaps << "\nAverage Roommate Compatibility: " << (compatibilityChecks ? (totalCompatibilityScore/compatibilityChecks) : 0) << "%\n";
}

void saveToTxt() {
    ofstream rf("sample_data/rooms.txt"), sf("sample_data/students.txt");
    for (Room& r : globalRooms) rf << r.id << " " << r.gender << " " << r.type << " " << r.capacity << " " << (r.isAC ? "AC" : "NONAC") << "\n";
    for (auto const& p : allStudents) {
        Student* s = p.second; if (s->isGraduated) continue;
        sf << s->id << " " << s->name << " " << s->gender << " " << s->cgpa << " " << s->seniority << " " << s->specialNeeds << " " << s->roomPref << " " << s->wantsMess << " " << s->foodPreference << " " << s->wantsAC << " " << s->distance << " " << s->prevPattern << " " << s->habits[0] << " " << s->habits[1] << " " << s->habits[2] << "\n";
    }
    exportReports();
}

void loadData() {
    ifstream rf("sample_data/rooms.txt"), sf("sample_data/students.txt");
    string id, acStr, n; char g; int t, c, sr, sp, pr, d, pp, h1, h2, h3, fp; double cgpa; bool wm, wac;
    if (rf) while (rf >> id >> g >> t >> c >> acStr) { Room r; r.id=id; r.gender=g; r.type=t; r.capacity=c; r.isAC=(acStr=="AC"); globalRooms.push_back(r); }
    for (Room& r : globalRooms) { string key = string(1, r.gender) + "_" + to_string(r.type); if (!typeHeads.count(key)) typeHeads[key] = &r; else { Room* curr = typeHeads[key]; while (curr->nextTypeRoom) curr = curr->nextTypeRoom; curr->nextTypeRoom = &r; } }
    if (sf) while (sf >> id >> n >> g >> cgpa >> sr >> sp >> pr >> wm >> fp >> wac >> d >> pp >> h1 >> h2 >> h3) {
        // Validate student ID format during TXT persistence loading
        if (!isValidStudentID(id, g)) {
            cout << "[Warning] Skipping invalid Student ID '" << id << "' during load. Expected format: " << (g == 'M' ? "SM##" : "SF##") << "\n";
            continue;
        }
        // Duplicate detection during loading
        if (allStudents.count(id)) {
            cout << "[Warning] Skipping duplicate Student ID '" << id << "' during load.\n";
            continue;
        }
        Student s; s.id=id; s.name=n; s.gender=g; s.cgpa=cgpa; s.seniority=sr; s.specialNeeds=sp; s.roomPref=pr; s.wantsMess=wm; s.foodPreference=fp; s.wantsAC=wac; s.distance=d; s.prevPattern=pp; s.habits[0]=h1; s.habits[1]=h2; s.habits[2]=h3;
        addStudentRaw(s);
    }
}

// ----- Semester Hostel Renewal & Checkout System -----
void semesterRenewal() {
    printHeader("Semester Hostel Renewal & Checkout System");
    cout << "[Semester Renewal Started]\n\n[Checkout]\n"; bool anyCheckout = false;
    for (auto& p : allStudents) {
        Student* s = p.second;
        if (s->seniority == 4 && !s->isGraduated) {
            if (s->isAllocated) for (Room& r : globalRooms) if (r.id == s->currentRoom) { r.removeStudent(s->id); break; }
            s->isGraduated = true; s->isAllocated = false; s->currentRoom = ""; s->waitReason = "Graduated"; s->inWaitlist = false;
            cout << s->id << " - " << s->name << " removed from hostel (Graduated)\n"; anyCheckout = true;
        }
    }
    if (!anyCheckout) cout << "No graduating students found.\n";
    cout << "\n[Vacancy Recovery]\n";
    vector<Student*> tq; while (!waitQueue.empty()) { tq.push_back(waitQueue.top()); waitQueue.pop(); }
    bool anyPromoted = false;
    for (Student* s : tq) {
        if (s->isGraduated || s->isAllocated) continue;
        if (tryAllocate(s, true) || tryAllocate(s, false)) { cout << "Promoted " << s->id << " to Room " << s->currentRoom << "\n"; anyPromoted = true; } else waitQueue.push(s);
    }
    if (!anyPromoted) cout << "No waitlisted students promoted.\n";
    saveToTxt(); cout << "\n[Success]\nSemester renewal completed successfully.\n";
}

int main() {
    loadData();
    while (true) {
        printHeader("Smart Campus Hostel Management ");
        cout << "1. Run Smart Allocation\n"
             << "2. View Room Allocations\n"
             << "3. Recover Vacancies\n"
             << "4. Check Roommate Compatibility\n"
             << "5. Add New Student\n"
             << "6. Add Swap Connection\n"
             << "7. Process Graph-Based Swaps\n"
             << "8. Undo Last Swap\n"
             << "9. Semester Hostel Renewal & Checkout\n"
             << "10. Analytics & Export Reports\n"
             << "11. Exit\n"
             << "------------------------------------------------------\n";
             
        int c = getValidatedChoice("Select an option: ", 1, 11);
        
        if (c == 1) runAllocation();
        else if (c == 2) { 
            printHeader("Current Room Allocations"); 
            cout << "[Room Types: 1: Single Room, 2: Double Sharing, 3: Triple Sharing, 4: Quad Sharing]\n\n";
            for (Room& r : globalRooms) { 
                cout << left << setw(6) << r.id << " [" << r.gender << "] (" << r.occupancy << "/" << r.capacity << "): "; 
                for (Student* s = r.head; s; s = s->next) cout << s->id << " "; cout << "\n"; 
            } 
        } 
        else if (c == 3) recoverVacancies();
        else if (c == 4) {
            printHeader("Roommate Compatibility Check");
            string i1 = getValidatedStudentIDLookup("Enter First Student ID: "), i2 = getValidatedStudentIDLookup("Enter Second Student ID: ");
            if (allStudents.count(i1) && allStudents.count(i2)) { 
                int sc = getCompatibility(allStudents[i1], allStudents[i2]); 
                cout << "\n[Info] Compatibility Score: " << sc << "% " << (sc < 40 ? "(High Risk)" : "(Safe)") << "\n"; 
            } else cout << "\n[Error] Student ID not found in the system.\n";
        } 
        else if (c == 5) {
            printHeader("Enter New Student Details");
            Student s; 
            s.gender = getValidatedGender("Enter Gender (M/F): "); 
            s.id = getValidatedStudentID("Enter Student ID (" + string(s.gender == 'M' ? "SM##" : "SF##") + "): ", s.gender); 
            s.name = getValidatedAlpha("Enter Student Name (Alphabets only): "); 
            s.cgpa = getValidatedDouble("Enter CGPA (0.00 - 10.00): ", 0.0, 10.0);
            s.seniority = getValidatedChoice("Enter Seniority Year (1 - 4): ", 1, 4); 
            s.specialNeeds = getValidatedChoice("Special Needs? (1/0): ", 0, 1); 
            s.roomPref = getValidatedChoice("Room Preference (1: Single Room (1 Bed), 2: Double Sharing (2 Beds), 3: Triple Sharing (3 Beds), 4: Quad Sharing (4 Beds)): ", 1, 4);
            s.wantsAC = getValidatedChoice("Prefer AC? (1/0): ", 0, 1); 
            s.wantsMess = getValidatedChoice("Hostel Mess? (1/0): ", 0, 1); 
            s.foodPreference = s.wantsMess ? getValidatedChoice("Food (0: Veg, 1: Non-Veg): ", 0, 1) : 0;
            s.distance = getValidatedChoice("Distance (km): ", 0, 10000); 
            s.prevPattern = getValidatedChoice("Prev No-Show? (1/0): ", 0, 1);
            s.habits[0] = getValidatedChoice("Sleep Habit (1-10): ", 1, 10); 
            s.habits[1] = getValidatedChoice("Cleanliness (1-10): ", 1, 10); 
            s.habits[2] = getValidatedChoice("Study Env (1-10): ", 1, 10);
            addStudentRaw(s); saveToTxt(); cout << "\n[Success] Student " << s.id << " added.\n";
        } 
        else if (c == 6) {
            printHeader("Add Swap Connection");
            cout << "[Info] Create a swap connection between two students.\n"
                 << "The system uses DFS graph traversal to detect valid room swap chains.\n\n";
            string sw1 = getValidatedStudentIDLookup("Enter First Student ID: ");
            if (!allStudents.count(sw1)) { cout << "\n[Error] Student ID " << sw1 << " not found.\n"; continue; }
            string sw2 = getValidatedStudentIDLookup("Enter Second Student ID: ");
            if (!allStudents.count(sw2)) { cout << "\n[Error] Student ID " << sw2 << " not found.\n"; continue; }
            swapGraph.addEdge(sw1, sw2); 
            cout << "\n[Success] Swap connection added.\n"; 
            saveToTxt(); 
        } 
        else if (c == 7) {
            processRoomSwaps(getValidatedStudentIDLookup("Enter Starting Student ID for Swap: "));
        }
        else if (c == 8) undoLastSwap();
        else if (c == 9) semesterRenewal();
        else if (c == 10) showAnalyticsReport();
        else if (c == 11) { 
            cout << "\nExiting system... Thank You\n"; 
            for (auto &p : allStudents) delete p.second; 
            allStudents.clear(); 
            break; 
        }
    }
    return 0;
}
