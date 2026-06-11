#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <set>

// 1. DATA PARSING & STRUCTURE DEFINITIONS

struct Athlete {
    std::string athleteID;
    std::string name;
    std::vector<std::string> registeredEventIDs;
};

struct Event {
    std::string eventID;
    std::string venueID;
    int startTime; // In minutes from midnight (e.g., 540 = 9:00 AM)
    int endTime;   // In minutes from midnight (e.g., 600 = 10:00 AM)
    int assignedTimeSlotBin = -1; // Determined by Graph Coloring
};

struct TransactionRequest {
    std::string type; // "UPDATE_ID" or "REGISTRATION"
    std::string athleteID;
    std::string detail;
};

// 2. LOGISTICS ENGINE CLASS

class TrackMasterEngine {
private:
    std::unordered_map<std::string, Athlete> athleteRegistry; // O(1) Lookups
    std::vector<Event> masterEvents;
    std::queue<TransactionRequest> transactionQueue;         // Safe processing pipeline

    // Helper to format minutes back to readable HH:MM
    std::string formatTime(int minutes) {
        int hrs = minutes / 60;
        int mins = minutes % 60;
        std::string hrsStr = (hrs < 10) ? "0" + std::to_string(hrs) : std::to_string(hrs);
        std::string minsStr = (mins < 10) ? "0" + std::to_string(mins) : std::to_string(mins);
        return hrsStr + ":" + minsStr;
    }

public:
    // Insert a valid athlete profile into memory
    void registerAthleteInMemory(const std::string& id, const std::string& name, const std::vector<std::string>& events) {
        athleteRegistry[id] = {id, name, events};
    }

    // Insert a tournament event fixture
    void addEventFixture(const std::string& eID, const std::string& vID, int start, int end) {
        masterEvents.push_back({eID, vID, start, end, -1});
    }

    // Push requests into the verification queue
    void queueTransaction(const TransactionRequest& req) {
        transactionQueue.push(req);
    }

    // ==========================================
    // 3. TRANSACTION SAFE PROCESSING PIPELINE
    // ==========================================
    void processTransactionQueue() {
        std::cout << "\n[PIPELINE] Processing Transaction Verification Queue...\n";
        while (!transactionQueue.empty()) {
            TransactionRequest req = transactionQueue.front();
            transactionQueue.pop();

            if (req.type == "UPDATE_ID") {
                // Input Sanitization: Validate alphanumeric criteria and length
                bool isValid = true;
                if (req.detail.length() < 4) isValid = false;
                for (char c : req.detail) {
                    if (!std::isalnum(c)) isValid = false;
                }

                if (!isValid) {
                    std::cout << "  Sanitization Rejection: Invalid profile pattern \"" << req.detail << "\"\n";
                } else {
                    std::cout << "  Data Sanitized: Updated Athlete " << req.athleteID << " profile sequence to " << req.detail << "\n";
                    if (athleteRegistry.find(req.athleteID) != athleteRegistry.end()) {
                        athleteRegistry[req.detail] = athleteRegistry[req.athleteID];
                        athleteRegistry[req.detail].athleteID = req.detail;
                        athleteRegistry.erase(req.athleteID);
                    }
                }
            }
        }
    }

    // ==========================================
    // 4. MODULE A: GREEDY INTERVAL SCHEDULING
    // ==========================================
    void executeIntervalSchedulingForVenue(const std::string& venueID) {
        std::cout << "\n[MODULE A] Running Greedy Interval Selection for Venue: " << venueID << "\n";
        
        // Filter out events belonging to this specific target venue
        std::vector<Event> venueEvents;
        for (const auto& ev : masterEvents) {
            if (ev.venueID == venueID) venueEvents.push_back(ev);
        }

        // Structural Sorting Criteria: Sort by Earliest End Time, O(N log N)
        std::sort(venueEvents.begin(), venueEvents.end(), [](const Event& a, const Event& b) {
            return a.endTime < b.endTime;
        });

        std::vector<Event> scheduledTimeline;
        int lastEndTime = 0;

        // Greedy Choice Execution
        for (const auto& ev : venueEvents) {
            if (ev.startTime >= lastEndTime) {
                scheduledTimeline.push_back(ev);
                lastEndTime = ev.endTime; // Shift temporal boundary marker
            }
        }

        // Print calculated throughput metrics
        std::cout << "  Optimized Maximized Lane Timeline Layout:\n";
        for (const auto& ev : scheduledTimeline) {
            std::cout << "   - Event: " << ev.eventID << " [" << formatTime(ev.startTime) << " -> " << formatTime(ev.endTime) << "]\n";
        }
    }

    // ==========================================
    // 5. MODULE B: GRAPH THEORY CONFLICT RESOLUTION
    // ==========================================
    void executeConflictFreeGraphColoring() {
        std::cout << "\n[MODULE B] Building Conflict Dependency Matrix & Graph Coloring Routine...\n";
        
        int V = masterEvents.size();
        // Map event IDs to linear indices for standard vector graph arrays
        std::unordered_map<std::string, int> eventIndexMap;
        for (int i = 0; i < V; ++i) {
            eventIndexMap[masterEvents[i].eventID] = i;
        }

        // Build Adjacency List dynamically based on overlapping athlete rosters
        std::vector<std::vector<int>> adjList(V, std::vector<int>());
        
        for (auto it = athleteRegistry.begin(); it != athleteRegistry.end(); ++it) {
            const Athlete& athlete = it->second; // Access the Athlete structure directly
            
            // Find collisions where an athlete is registered to multiple events simultaneously
            for (size_t i = 0; i < athlete.registeredEventIDs.size(); ++i) {
                for (size_t j = i + 1; j < athlete.registeredEventIDs.size(); ++j) {
                    std::string evA = athlete.registeredEventIDs[i];
                    std::string evB = athlete.registeredEventIDs[j];
                    
                    if (eventIndexMap.find(evA) != eventIndexMap.end() && eventIndexMap.find(evB) != eventIndexMap.end()) {
                        int u = eventIndexMap[evA];
                        int v = eventIndexMap[evB];
                        // Append undirected constraint edges
                        if (std::find(adjList[u].begin(), adjList[u].end(), v) == adjList[u].end()) {
                            adjList[u].push_back(v);
                            adjList[v].push_back(u);
                        }
                    }
                }
            }
        }
            
        // Run the Greedy Graph Coloring Heuristic Framework
        std::vector<int> resultColors(V, -1);
        resultColors[0] = 0; // Color the first event node

        std::vector<bool> colorAvailabilityTracker(V, true);

        for (int u = 1; u < V; ++u) {
            // Track neighboring constraints and mask their mapped colors out
            for (int neighbor : adjList[u]) {
                if (resultColors[neighbor] != -1) {
                    colorAvailabilityTracker[resultColors[neighbor]] = false;
                }
            }

            // Identify the first safe time-slot index configuration color
            int chosenColor;
            for (chosenColor = 0; chosenColor < V; ++chosenColor) {
                if (colorAvailabilityTracker[chosenColor]) break;
            }

            resultColors[u] = chosenColor;
            masterEvents[u].assignedTimeSlotBin = chosenColor;

            // Clear configuration flags back to safe state values
            std::fill(colorAvailabilityTracker.begin(), colorAvailabilityTracker.end(), true);
        }
        masterEvents[0].assignedTimeSlotBin = 0; // Fix assignment loop index boundary

        // Output Result Bin Assignments
        std::cout << "  Conflict-Free Event Timetable Slots Mapped Successfully:\n";
        for (const auto& ev : masterEvents) {
            std::cout << "   - " << ev.eventID << " mapped safely into Time-Slot Block [" << ev.assignedTimeSlotBin << "]\n";
        }
    }

    // ==========================================
    // 6. STATE DATA SERIALIZATION (FILE I/O)
    // ==========================================
    void serializeSystemState(const std::string& filename) {
        std::ofstream outFile(filename);
        if (!outFile) {
            std::cerr << "Error writing state serialization manifest.\n";
            return;
        }

        outFile << "{\n  \"TrackMasterGeneratedSchedule\": [\n";
        for (size_t i = 0; i < masterEvents.size(); ++i) {
            outFile << "    {\n";
            outFile << "      \"eventID\": \"" << masterEvents[i].eventID << "\",\n";
            outFile << "      \"venueID\": \"" << masterEvents[i].venueID << "\",\n";
            outFile << "      \"timeSlotBin\": " << masterEvents[i].assignedTimeSlotBin << "\n";
            outFile << "    }" << (i == masterEvents.size() - 1 ? "" : ",") << "\n";
        }
        outFile << "  ]\n}\n";
        outFile.close();
        std::cout << "\n[FILE I/O] System manifest layout serialized out safely to \"" << filename << "\"\n";
    }
};

// ==========================================
// 7. DRIVER MAIN RUNTIME ENGINE
// ==========================================
int main() {
    std::cout << "=== TRACKMASTER LOGISTICS ENGINE MAIN CORE RUNTIME ===\n";
    TrackMasterEngine engine;

    // Instrumentation Performance Clock Start
    auto startTimeClock = std::chrono::high_resolution_clock::now();

    // Mocking Base Tournament Fixtures Data
    engine.addEventFixture("110M_Hurdles_Heat1", "Main_Track_Lane1", 540, 570); // 9:00 AM -> 9:30 AM
    engine.addEventFixture("110M_Hurdles_Heat2", "Main_Track_Lane1", 560, 600); // 9:20 AM -> 10:00 AM (Overlaps Lane)
    engine.addEventFixture("110M_Hurdles_Final", "Main_Track_Lane1", 610, 640); // 10:10 AM -> 10:40 AM
    engine.addEventFixture("Long_Jump_Qualifiers", "Field_Pit_A", 550, 620);    // 9:10 AM -> 10:20 AM

    // Mocking Athlete Registration Sets with cross-over structural conflicts
    // Athlete Anjali competes in both Hurdles Heat 1 and Long Jump (creating an operational bottleneck)
    engine.registerAthleteInMemory("A2409", "Anjali Kumari", {"110M_Hurdles_Heat1", "Long_Jump_Qualifiers"});
    engine.registerAthleteInMemory("A1102", "Rohit Sharma", {"110M_Hurdles_Heat2"});

    // Simulating batch user input error corrections in tracking ledger profiles
    engine.queueTransaction({"UPDATE_ID", "A2409", "24JE0009"}); // Valid pattern mapping correction
    engine.queueTransaction({"UPDATE_ID", "A1102", "ERR!!#"});   // Toxic/Malformed data input verification string

    // Pipeline Execution Sequence
    engine.processTransactionQueue();
    engine.executeIntervalSchedulingForVenue("Main_Track_Lane1");
    engine.executeConflictFreeGraphColoring();
    
    // Save State Manifest
    engine.serializeSystemState("final_schedule_manifest.json");

    // Performance End Time Instrumentation Tracking
    auto endTimeClock = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> dynamicLatency = endTimeClock - startTimeClock;
    
    std::cout << "\n[PERFORMANCE] Overall Core Engine Clock Execution Latency: " << dynamicLatency.count() << " ms\n";
    std::cout << "=========================================================\n";

    return 0;
}
