#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_RESOURCES 100
#define MAX_PROCESSES 100

// Forward declaration of Resource structure
typedef struct Resource Resource;

// Structure to represent a process
typedef struct {
    int id;
    Resource* holding;
    Resource* waiting;
} Process;

// Structure to represent a resource
struct Resource {
    int id;
    int site;
    int heldBy; // Process ID of the process holding this resource, -1 if not held
    int localCoordinator; // Local coordinator for the site
    int globalCoordinator; // Global coordinator for all sites
};

// Function to check for cycles in the resource allocation graph
bool detectCycle(Process* processes, Resource* resources, Process* cur, int start) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (cur->waiting != NULL && cur->waiting->id == processes[i].holding->id) {
            if (processes[i].id == start) {
                return true;
            } else {
                if (detectCycle(processes, resources, &processes[i], start)) {
                    return true;
                }
            }
        }
    }
    return false;
}

// Function to check for deadlock in a site
bool checkDeadlockSite(Process* processes, Resource* resources, int site) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].id != -1 && processes[i].holding != NULL && processes[i].waiting != NULL &&
            processes[i].holding->site == site && processes[i].waiting->site == site) {
            if (detectCycle(processes, resources, &processes[i], processes[i].id)) {
                return true;
            }
        }
    }
    return false;
}

// Function to check for deadlock in the coordinator
bool checkDeadlock(Process* processes, Resource* resources) {
    bool globalDeadlock = false; // Initialize to false

    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].waiting != NULL && detectCycle(processes, resources, &processes[i], processes[i].id)) {
            // Check if the waiting resource is from a different site
            bool waitingFromDifferentSite = false;
            for (int j = 0; j < MAX_PROCESSES; j++) {
                if (processes[j].id != -1 && processes[j].holding != NULL && processes[j].waiting != NULL) {
                    if (processes[j].holding->site != processes[j].waiting->site) {
                        waitingFromDifferentSite = true;
                        break;
                    }
                }
            }
            if (waitingFromDifferentSite) {
                printf("Global deadlock detected at Process %d in the central coordinator.\n", processes[i].id);
                globalDeadlock = true; // Set globalDeadlock to true
            } else {
                printf("Deadlock within site %d detected at Process %d.\n", processes[i].waiting->site, processes[i].id);
            }
        }
    }

    return globalDeadlock; // Return the result after checking all processes
}

int main() {
    Resource resources[MAX_RESOURCES];
    Process processes[MAX_PROCESSES];

    // Initialize processes
    for (int i = 0; i < MAX_PROCESSES; i++) {
        processes[i].id = -1; // Indicates empty slot
        processes[i].holding = NULL;
        processes[i].waiting = NULL;
    }

    // Initialize resources for site 1
    int s1No, s2No;
    printf("No. of resources in site 1: ");
    scanf("%d", &s1No);
    for (int i = 0; i < s1No; i++) {
        resources[i].id = i;
        resources[i].site = 1;
        resources[i].heldBy = -1; // Initially not held by any process
        resources[i].localCoordinator = 1; // Local coordinator for site 1
        resources[i].globalCoordinator = 0; // Not set yet
    }

    // Initialize resources for site 2
    printf("No. of resources in site 2: ");
    scanf("%d", &s2No);
    for (int i = s1No; i < s1No + s2No; i++) {
        resources[i].id = i;
        resources[i].site = 2;
        resources[i].heldBy = -1; // Initially not held by any process
        resources[i].localCoordinator = 2; // Local coordinator for site 2
        resources[i].globalCoordinator = 0; // Not set yet
    }

    printf("\nResources in site 1:\n");
    for (int i = 0; i < s1No; i++) {
        printf("%d ", resources[i].id);
    }
    printf("\nResources in site 2:\n");
    for (int i = s1No; i < s1No + s2No; i++) {
        printf("%d ", resources[i].id);
    }
    printf("\n\n");

    // Input processes
    int NoOfProcesses;
    printf("Enter number of processes: ");
    scanf("%d", &NoOfProcesses);
    for (int i = 0; i < NoOfProcesses; i++) {
        int hld, wai;
        printf("What resource is process-%d holding? (Enter -1 for none): ", i);
        scanf("%d", &hld);
        printf("What resource is process-%d waiting for? (Enter -1 for none): ", i);
        scanf("%d", &wai);
        processes[i].id = i;
        if (hld != -1) {
            processes[i].holding = &resources[hld];
            resources[hld].heldBy = i; // Process i is holding resource hld
        } else {
            processes[i].holding = NULL;
        }
        if (wai != -1) {
            processes[i].waiting = &resources[wai];
        } else {
            processes[i].waiting = NULL;
        }
    }

    // Print local coordinators
    printf("\nLocal Coordinators:\n");
    for (int i = 0; i < s1No + s2No; i++) {
        printf("Resource %d - Site %d: Local Coordinator: %d\n", resources[i].id, resources[i].site, resources[i].localCoordinator);
    }

    // Find the site with the highest number of resources as the global coordinator
    int maxResources = s1No;
    int globalCoordinatorSite = 1;
    if (s2No > maxResources) {
        maxResources = s2No;
        globalCoordinatorSite = 2;
    }

    // Assign the global coordinator
    for (int i = 0; i < s1No + s2No; i++) {
                if (resources[i].site == globalCoordinatorSite) {
            resources[i].globalCoordinator = 1; // Mark the resource in the site with maximum resources as the global coordinator
        } else {
            resources[i].globalCoordinator = 0; // Other resources are not global coordinators
        }
    }

    // Print global coordinator
    printf("\nGlobal Coordinator:\n");
    for (int i = 0; i < s1No + s2No; i++) {
        if (resources[i].globalCoordinator == 1) {
            printf("Resource %d - Site %d: Global Coordinator\n", resources[i].id, resources[i].site);
            break; // Assuming there is only one global coordinator
        }
    }

    // Check for deadlock
    bool globalDeadlock = checkDeadlock(processes, resources);
    bool site1Deadlock = checkDeadlockSite(processes, resources, 1);
    bool site2Deadlock = checkDeadlockSite(processes, resources, 2);

    // Print deadlock detection results
    if (globalDeadlock) {
        printf("Deadlock detected in central coordinator\n");
    }
    if (site1Deadlock) {
        printf("Deadlock detected in site 1\n");
    }
    if (site2Deadlock) {
        printf("Deadlock detected in site 2\n");
    }
    if (!globalDeadlock && !site1Deadlock && !site2Deadlock) {
        printf("No deadlock detected\n");
    }

    return 0;
}

       
