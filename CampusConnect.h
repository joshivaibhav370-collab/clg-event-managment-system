#ifndef CAMPUSCONNECT_H
#define CAMPUSCONNECT_H

#include <iostream>
#include <string>
#include <vector>

// ---------- Small helpers to build JSON text for the website ----------
inline std::string jsonEscape(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (c == '\n' || c == '\r') continue;
        if (c == '"' || c == '\\') out += '\\';
        out += c;
    }
    return out;
}

// Every action returns {"ok":true/false,"msg":"..."}
inline std::string jsonResult(bool ok, const std::string& msg) {
    return std::string("{\"ok\":") + (ok ? "true" : "false") +
           ",\"msg\":\"" + jsonEscape(msg) + "\"}";
}

// ---------- OOP: Person -> Student (inheritance) ----------
class Person {
protected:
    std::string id;
    std::string name;
    std::string department;

public:
    Person(const std::string& id, const std::string& name, const std::string& department);
    virtual ~Person();

    std::string getId() const;
    std::string getName() const;
    std::string getDepartment() const;
    virtual std::string toJson() const;
};

class Student : public Person {
private:
    int semester;

public:
    Student(const std::string& id, const std::string& name, const std::string& department, int semester);
    int getSemester() const;
    std::string toJson() const override;
};

// ---------- Event ----------
class Event {
private:
    std::string eventId;
    std::string eventName;
    std::string date;
    std::string venue;
    int maxCapacity;
    int currentRegistrations;

public:
    Event(const std::string& id, const std::string& name, const std::string& date, const std::string& venue, int capacity);

    std::string getEventId() const;
    std::string getEventName() const;
    bool isFull() const;
    bool incrementRegistration();
    bool decrementRegistration();
    std::string toJson() const;
};

// ---------- Singly Linked List: confirmed registrations ----------
struct RegNode {
    std::string studentId;
    std::string eventId;
    RegNode* next;
};

class RegistrationList {
private:
    RegNode* head;

public:
    RegistrationList();
    ~RegistrationList();
    RegistrationList(const RegistrationList&) = delete;            // no copying (raw pointers)
    RegistrationList& operator=(const RegistrationList&) = delete;

    void insert(const std::string& studentId, const std::string& eventId);
    bool remove(const std::string& studentId, const std::string& eventId);
    bool isRegistered(const std::string& studentId, const std::string& eventId) const;
    int size() const;
    RegNode* getHead() const;
};

// ---------- Queue (linked list): waiting list, FIFO ----------
struct QueueNode {
    std::string studentId;
    std::string eventId;
    QueueNode* next;
};

class WaitingQueue {
private:
    QueueNode* frontNode;
    QueueNode* rearNode;
    int count;

public:
    WaitingQueue();
    ~WaitingQueue();
    WaitingQueue(const WaitingQueue&) = delete;
    WaitingQueue& operator=(const WaitingQueue&) = delete;

    void enqueue(const std::string& studentId, const std::string& eventId);
    bool dequeue(std::string& studentId, std::string& eventId);
    bool remove(const std::string& studentId, const std::string& eventId);
    bool contains(const std::string& studentId, const std::string& eventId) const;
    bool firstForEvent(const std::string& eventId, std::string& studentId) const;
    bool isEmpty() const;
    int size() const;
    QueueNode* getFront() const;
};

// ---------- Stack (linked list): undo history, LIFO ----------
struct Action {
    std::string actionType;   // "REGISTER" or "WAIT"
    std::string studentId;
    std::string eventId;
};

struct StackNode {
    Action data;
    StackNode* next;
};

class ActionStack {
private:
    StackNode* topNode;

public:
    ActionStack();
    ~ActionStack();
    ActionStack(const ActionStack&) = delete;
    ActionStack& operator=(const ActionStack&) = delete;

    void push(const Action& act);
    bool pop(Action& act);
    bool isEmpty() const;
};

// ---------- Main system: everything the website calls ----------
class CampusConnectSystem {
private:
    std::vector<Event> events;
    std::vector<Student> students;
    RegistrationList registrations;
    WaitingQueue waitingList;
    ActionStack undoStack;

    void seedSampleData();
    std::string studentName(const std::string& id) const;
    std::string eventName(const std::string& id) const;
    std::string pairJson(const std::string& studentId, const std::string& eventId) const;
    std::string promoteFromWaiting(const std::string& eventId);
    bool removeStudentFromEvent(const std::string& studentId, const std::string& eventId, std::string& msg);

public:
    CampusConnectSystem();

    bool addEvent(const std::string& id, const std::string& name, const std::string& date, const std::string& venue, int capacity);
    bool addStudent(const std::string& id, const std::string& name, const std::string& department, int semester);
    Event* findEvent(const std::string& id);
    Student* findStudent(const std::string& id);

    // Actions (return JSON result)
    std::string registerStudent(const std::string& studentId, const std::string& eventId);
    std::string cancelRegistration(const std::string& studentId, const std::string& eventId);
    std::string undoLastAction();

    // Data for the pages (return JSON)
    std::string eventsJson() const;
    std::string studentsJson() const;
    std::string registrationsJson() const;
    std::string waitingJson() const;
    std::string statsJson() const;
};

#endif
