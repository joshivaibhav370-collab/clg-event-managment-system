#include "CampusConnect.h"

// ==================== Person / Student ====================
Person::Person(const std::string& id, const std::string& name, const std::string& department)
    : id(id), name(name), department(department) {}

Person::~Person() {}

std::string Person::getId() const { return id; }
std::string Person::getName() const { return name; }
std::string Person::getDepartment() const { return department; }

std::string Person::toJson() const {
    return "{\"id\":\"" + jsonEscape(id) + "\",\"name\":\"" + jsonEscape(name) +
           "\",\"department\":\"" + jsonEscape(department) + "\"}";
}

Student::Student(const std::string& id, const std::string& name, const std::string& department, int semester)
    : Person(id, name, department), semester(semester) {}

int Student::getSemester() const { return semester; }

std::string Student::toJson() const {
    return "{\"id\":\"" + jsonEscape(id) + "\",\"name\":\"" + jsonEscape(name) +
           "\",\"department\":\"" + jsonEscape(department) +
           "\",\"semester\":" + std::to_string(semester) + "}";
}

// ==================== Event ====================
Event::Event(const std::string& id, const std::string& name, const std::string& date, const std::string& venue, int capacity)
    : eventId(id), eventName(name), date(date), venue(venue), maxCapacity(capacity), currentRegistrations(0) {}

std::string Event::getEventId() const { return eventId; }
std::string Event::getEventName() const { return eventName; }

bool Event::isFull() const {
    return currentRegistrations >= maxCapacity;
}

bool Event::incrementRegistration() {
    if (isFull()) return false;
    currentRegistrations++;
    return true;
}

bool Event::decrementRegistration() {
    if (currentRegistrations == 0) return false;
    currentRegistrations--;
    return true;
}

std::string Event::toJson() const {
    return "{\"id\":\"" + jsonEscape(eventId) + "\",\"name\":\"" + jsonEscape(eventName) +
           "\",\"date\":\"" + jsonEscape(date) + "\",\"venue\":\"" + jsonEscape(venue) +
           "\",\"capacity\":" + std::to_string(maxCapacity) +
           ",\"registered\":" + std::to_string(currentRegistrations) + "}";
}

// ==================== RegistrationList (linked list) ====================
RegistrationList::RegistrationList() : head(nullptr) {}

RegistrationList::~RegistrationList() {
    while (head != nullptr) {
        RegNode* temp = head;
        head = head->next;
        delete temp;
    }
}

void RegistrationList::insert(const std::string& studentId, const std::string& eventId) {
    RegNode* newNode = new RegNode{studentId, eventId, nullptr};
    if (head == nullptr) {
        head = newNode;
        return;
    }
    RegNode* temp = head;
    while (temp->next != nullptr) temp = temp->next;
    temp->next = newNode;
}

bool RegistrationList::remove(const std::string& studentId, const std::string& eventId) {
    RegNode* prev = nullptr;
    RegNode* cur = head;
    while (cur != nullptr) {
        if (cur->studentId == studentId && cur->eventId == eventId) {
            if (prev == nullptr) head = cur->next;
            else prev->next = cur->next;
            delete cur;
            return true;
        }
        prev = cur;
        cur = cur->next;
    }
    return false;
}

bool RegistrationList::isRegistered(const std::string& studentId, const std::string& eventId) const {
    for (RegNode* cur = head; cur != nullptr; cur = cur->next) {
        if (cur->studentId == studentId && cur->eventId == eventId) return true;
    }
    return false;
}

int RegistrationList::size() const {
    int n = 0;
    for (RegNode* cur = head; cur != nullptr; cur = cur->next) n++;
    return n;
}

RegNode* RegistrationList::getHead() const { return head; }

// ==================== WaitingQueue (FIFO) ====================
WaitingQueue::WaitingQueue() : frontNode(nullptr), rearNode(nullptr), count(0) {}

WaitingQueue::~WaitingQueue() {
    std::string s, e;
    while (dequeue(s, e)) {}
}

void WaitingQueue::enqueue(const std::string& studentId, const std::string& eventId) {
    QueueNode* newNode = new QueueNode{studentId, eventId, nullptr};
    if (rearNode == nullptr) {
        frontNode = rearNode = newNode;
    } else {
        rearNode->next = newNode;
        rearNode = newNode;
    }
    count++;
}

bool WaitingQueue::dequeue(std::string& studentId, std::string& eventId) {
    if (isEmpty()) return false;
    QueueNode* temp = frontNode;
    studentId = temp->studentId;
    eventId = temp->eventId;
    frontNode = frontNode->next;
    if (frontNode == nullptr) rearNode = nullptr;
    delete temp;
    count--;
    return true;
}

// Removes one student from anywhere in the queue (used for cancel / promote)
bool WaitingQueue::remove(const std::string& studentId, const std::string& eventId) {
    QueueNode* prev = nullptr;
    QueueNode* cur = frontNode;
    while (cur != nullptr) {
        if (cur->studentId == studentId && cur->eventId == eventId) {
            if (prev == nullptr) frontNode = cur->next;
            else prev->next = cur->next;
            if (cur == rearNode) rearNode = prev;
            delete cur;
            count--;
            return true;
        }
        prev = cur;
        cur = cur->next;
    }
    return false;
}

bool WaitingQueue::contains(const std::string& studentId, const std::string& eventId) const {
    for (QueueNode* cur = frontNode; cur != nullptr; cur = cur->next) {
        if (cur->studentId == studentId && cur->eventId == eventId) return true;
    }
    return false;
}

// Finds the earliest waiting student for one event (FIFO per event)
bool WaitingQueue::firstForEvent(const std::string& eventId, std::string& studentId) const {
    for (QueueNode* cur = frontNode; cur != nullptr; cur = cur->next) {
        if (cur->eventId == eventId) {
            studentId = cur->studentId;
            return true;
        }
    }
    return false;
}

bool WaitingQueue::isEmpty() const { return frontNode == nullptr; }
int WaitingQueue::size() const { return count; }
QueueNode* WaitingQueue::getFront() const { return frontNode; }

// ==================== ActionStack (LIFO) ====================
ActionStack::ActionStack() : topNode(nullptr) {}

ActionStack::~ActionStack() {
    Action act;
    while (pop(act)) {}
}

void ActionStack::push(const Action& act) {
    topNode = new StackNode{act, topNode};
}

bool ActionStack::pop(Action& act) {
    if (isEmpty()) return false;
    StackNode* temp = topNode;
    act = temp->data;
    topNode = topNode->next;
    delete temp;
    return true;
}

bool ActionStack::isEmpty() const { return topNode == nullptr; }

// ==================== CampusConnectSystem ====================
CampusConnectSystem::CampusConnectSystem() {
    seedSampleData();
}

void CampusConnectSystem::seedSampleData() {
    addEvent("E101", "Hackathon", "2026-12-10", "Lab-1", 3);
    addEvent("E102", "Dance Battle", "2026-12-12", "Auditorium", 4);
    addEvent("E103", "Cricket Tournament", "2026-12-15", "College Ground", 4);
    addEvent("E104", "Coding Contest", "2026-12-18", "Lab-2", 2);
    addEvent("E105", "Photography Contest", "2026-12-20", "Hall-A", 3);

    addStudent("S101", "Rahul Sharma", "CSE", 5);
    addStudent("S102", "Priya Patel", "ECE", 3);
    addStudent("S103", "Aman Verma", "ME", 7);
    addStudent("S104", "Riya Singh", "CSE", 1);
    addStudent("S105", "Kunal Mehta", "IT", 3);
    addStudent("S106", "Sneha Gupta", "CE", 5);

    registerStudent("S101", "E101");
    registerStudent("S104", "E101");
    registerStudent("S102", "E102");
    registerStudent("S101", "E104");
    registerStudent("S103", "E104");
    registerStudent("S106", "E104");   // E104 is full -> goes to waiting list

    Action a;
    while (undoStack.pop(a)) {}        // sample data should not be undoable
}

bool CampusConnectSystem::addEvent(const std::string& id, const std::string& name, const std::string& date, const std::string& venue, int capacity) {
    if (findEvent(id) != nullptr) return false;
    events.emplace_back(id, name, date, venue, capacity);
    return true;
}

bool CampusConnectSystem::addStudent(const std::string& id, const std::string& name, const std::string& department, int semester) {
    if (findStudent(id) != nullptr) return false;
    students.emplace_back(id, name, department, semester);
    return true;
}

Event* CampusConnectSystem::findEvent(const std::string& id) {
    for (auto& ev : events) {
        if (ev.getEventId() == id) return &ev;
    }
    return nullptr;
}

Student* CampusConnectSystem::findStudent(const std::string& id) {
    for (auto& st : students) {
        if (st.getId() == id) return &st;
    }
    return nullptr;
}

std::string CampusConnectSystem::studentName(const std::string& id) const {
    for (const auto& st : students) {
        if (st.getId() == id) return st.getName();
    }
    return id;
}

std::string CampusConnectSystem::eventName(const std::string& id) const {
    for (const auto& ev : events) {
        if (ev.getEventId() == id) return ev.getEventName();
    }
    return id;
}

// If a seat is free, move the earliest waiting student of this event into it.
// Returns the promoted student's ID, or "" if nobody was promoted.
std::string CampusConnectSystem::promoteFromWaiting(const std::string& eventId) {
    Event* ev = findEvent(eventId);
    std::string next;
    if (ev == nullptr || ev->isFull()) return "";
    if (!waitingList.firstForEvent(eventId, next)) return "";

    waitingList.remove(next, eventId);
    ev->incrementRegistration();
    registrations.insert(next, eventId);
    return next;
}

// Removes a student from an event (confirmed list OR waiting list).
bool CampusConnectSystem::removeStudentFromEvent(const std::string& studentId, const std::string& eventId, std::string& msg) {
    if (registrations.remove(studentId, eventId)) {
        Event* ev = findEvent(eventId);
        if (ev != nullptr) ev->decrementRegistration();
        msg = "Registration of " + studentId + " for " + eventId + " removed.";

        std::string promoted = promoteFromWaiting(eventId);
        if (!promoted.empty()) {
            msg += " " + promoted + " moved from waiting list to confirmed.";
        }
        return true;
    }
    if (waitingList.remove(studentId, eventId)) {
        msg = studentId + " removed from the waiting list of " + eventId + ".";
        return true;
    }
    msg = "No registration found for " + studentId + " in " + eventId + ".";
    return false;
}

std::string CampusConnectSystem::registerStudent(const std::string& studentId, const std::string& eventId) {
    if (findStudent(studentId) == nullptr) {
        return jsonResult(false, "Student " + studentId + " not found.");
    }
    Event* ev = findEvent(eventId);
    if (ev == nullptr) {
        return jsonResult(false, "Event " + eventId + " not found.");
    }
    if (registrations.isRegistered(studentId, eventId)) {
        return jsonResult(false, "Student is already registered for this event.");
    }
    if (waitingList.contains(studentId, eventId)) {
        return jsonResult(false, "Student is already in the waiting list for this event.");
    }

    if (ev->isFull()) {
        waitingList.enqueue(studentId, eventId);
        undoStack.push(Action{"WAIT", studentId, eventId});
        return jsonResult(true, "Event is full. " + studentId + " added to the waiting list.");
    }

    ev->incrementRegistration();
    registrations.insert(studentId, eventId);
    undoStack.push(Action{"REGISTER", studentId, eventId});
    return jsonResult(true, studentId + " registered for " + ev->getEventName() + ".");
}

std::string CampusConnectSystem::cancelRegistration(const std::string& studentId, const std::string& eventId) {
    std::string msg;
    bool ok = removeStudentFromEvent(studentId, eventId, msg);
    return jsonResult(ok, msg);
}

// Undo = take the last action from the stack and reverse it.
std::string CampusConnectSystem::undoLastAction() {
    Action last;
    if (!undoStack.pop(last)) {
        return jsonResult(false, "Nothing to undo.");
    }
    std::string msg;
    bool ok = removeStudentFromEvent(last.studentId, last.eventId, msg);
    return jsonResult(ok, "Undo: " + msg);
}

// ---------- JSON output ----------
std::string CampusConnectSystem::pairJson(const std::string& studentId, const std::string& eventId) const {
    return "{\"studentId\":\"" + jsonEscape(studentId) +
           "\",\"studentName\":\"" + jsonEscape(studentName(studentId)) +
           "\",\"eventId\":\"" + jsonEscape(eventId) +
           "\",\"eventName\":\"" + jsonEscape(eventName(eventId)) + "\"}";
}

std::string CampusConnectSystem::eventsJson() const {
    std::string out = "[";
    for (size_t i = 0; i < events.size(); i++) {
        if (i > 0) out += ",";
        out += events[i].toJson();
    }
    return out + "]";
}

std::string CampusConnectSystem::studentsJson() const {
    std::string out = "[";
    for (size_t i = 0; i < students.size(); i++) {
        if (i > 0) out += ",";
        out += students[i].toJson();
    }
    return out + "]";
}

std::string CampusConnectSystem::registrationsJson() const {
    std::string out = "[";
    for (RegNode* n = registrations.getHead(); n != nullptr; n = n->next) {
        if (out.size() > 1) out += ",";
        out += pairJson(n->studentId, n->eventId);
    }
    return out + "]";
}

std::string CampusConnectSystem::waitingJson() const {
    std::string out = "[";
    for (QueueNode* n = waitingList.getFront(); n != nullptr; n = n->next) {
        if (out.size() > 1) out += ",";
        out += pairJson(n->studentId, n->eventId);
    }
    return out + "]";
}

std::string CampusConnectSystem::statsJson() const {
    return "{\"events\":" + std::to_string(events.size()) +
           ",\"students\":" + std::to_string(students.size()) +
           ",\"registered\":" + std::to_string(registrations.size()) +
           ",\"waiting\":" + std::to_string(waitingList.size()) + "}";
}
