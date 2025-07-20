#include "voting.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// Enable in Makefile
#ifdef DEBUG_PRINTS_ENABLED
#define DebugPrint(...) printf(__VA_ARGS__);
#else
#define DebugPrint(...)
#endif

#define PRIMES_SZ 1024
#define DISTRICTS_SZ 56
#define PARTIES_SZ 5

typedef struct District District;
typedef struct Station Station;
typedef struct Voter Voter;
typedef struct Party Party;
typedef struct Candidate Candidate;
typedef struct ElectedCandidate ElectedCandidate;

struct District {
    int did;
    int seats;
    int blanks;
    int invalids;
    int partyVotes[PARTIES_SZ];
};

struct Station {
    int sid;
    int did;
    int registered;
    Voter* voters;
    Station* next;
};
struct Voter {
    int vid;
    bool voted;
    Voter* parent;
    Voter* lc;
    Voter* rc;
};

struct Party {
    int pid;
    int electedCount;
    Candidate* candidates;
};
struct Candidate {
    int cid;
    int did;
    int pid; // Add this line
    int votes;
    bool isElected;
    Candidate* lc;
    Candidate* rc;
};

struct ElectedCandidate {
    int cid;
    int did;
    int pid;
    ElectedCandidate* next;
};

District Districts[DISTRICTS_SZ];
Station** StationsHT;
Party Parties[PARTIES_SZ];
ElectedCandidate* Parliament;

const int DefaultDid = -1;
const int BlankDid = -1;
const int InvalidDid = -2;

const int Primes[PRIMES_SZ] = {
    0, 1, 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541, 547, 557, 563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691, 701, 709, 719, 727, 733, 739, 743, 751, 757, 761, 769, 773, 787, 797, 809, 811, 821, 823, 827, 829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 929, 937, 941, 947, 953, 967, 971, 977, 983, 991, 997, 1009, 1013, 1019, 1021, 1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069, 1087, 1091, 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163, 1171, 1181, 1187, 1193, 1201, 1213, 1217, 1223, 1229, 1231, 1237, 1249, 1259, 1277, 1279, 1283, 1289, 1291, 1297, 1301, 1303, 1307, 1319, 1321, 1327, 1361, 1367, 1373, 1381, 1399, 1409, 1423, 1427, 1429, 1433, 1439, 1447, 1451, 1453, 1459, 1471, 1481, 1483, 1487, 1489, 1493, 1499, 1511, 1523, 1531, 1543, 1549, 1553, 1559, 1567, 1571, 1579, 1583, 1597, 1601, 1607, 1609, 1613, 1619, 1621, 1627, 1637, 1657, 1663, 1667, 1669, 1693, 1697, 1699, 1709, 1721, 1723, 1733, 1741, 1747, 1753, 1759, 1777, 1783, 1787, 1789, 1801, 1811, 1823, 1831, 1847, 1861, 1867, 1871, 1873, 1877, 1879, 1889, 1901, 1907, 1913, 1931, 1933, 1949, 1951, 1973, 1979, 1987, 1993, 1997, 1999, 2003, 2011, 2017, 2027, 2029, 2039, 2053, 2063, 2069, 2081, 2083, 2087, 2089, 2099, 2111, 2113, 2129, 2131, 2137, 2141, 2143, 2153, 2161, 2179, 2203, 2207, 2213, 2221, 2237, 2239, 2243, 2251, 2267, 2269, 2273, 2281, 2287, 2293, 2297, 2309, 2311, 2333, 2339, 2341, 2347, 2351, 2357, 2371, 2377, 2381, 2383, 2389, 2393, 2399, 2411, 2417, 2423, 2437, 2441, 2447, 2459, 2467, 2473, 2477, 2503, 2521, 2531, 2539, 2543, 2549, 2551, 2557, 2579, 2591, 2593, 2609, 2617, 2621, 2633, 2647, 2657, 2659, 2663, 2671, 2677, 2683, 2687, 2689, 2693, 2699, 2707, 2711, 2713, 2719, 2729, 2731, 2741, 2749, 2753, 2767, 2777, 2789, 2791, 2797, 2801, 2803, 2819, 2833, 2837, 2843, 2851, 2857, 2861, 2879, 2887, 2897, 2903, 2909, 2917, 2927, 2939, 2953, 2957, 2963, 2969, 2971, 2999, 3001, 3011, 3019, 3023, 3037, 3041, 3049, 3061, 3067, 3079, 3083, 3089, 3109, 3119, 3121, 3137, 3163, 3167, 3169, 3181, 3187, 3191, 3203, 3209, 3217, 3221, 3229, 3251, 3253, 3257, 3259, 3271, 3299, 3301, 3307, 3313, 3319, 3323, 3329, 3331, 3343, 3347, 3359, 3361, 3371, 3373, 3389, 3391, 3407, 3413, 3433, 3449, 3457, 3461, 3463, 3467, 3469, 3491, 3499, 3511, 3517, 3527, 3529, 3533, 3539, 3541, 3547, 3557, 3559, 3571, 3581, 3583, 3593, 3607, 3613, 3617, 3623, 3631, 3637, 3643, 3659, 3671, 3673, 3677, 3691, 3697, 3701, 3709, 3719, 3727, 3733, 3739, 3761, 3767, 3769, 3779, 3793, 3797, 3803, 3821, 3823, 3833, 3847, 3851, 3853, 3863, 3877, 3881, 3889, 3907, 3911, 3917, 3919, 3923, 3929, 3931, 3943, 3947, 3967, 3989, 4001, 4003, 4007, 4013, 4019, 4021, 4027, 4049, 4051, 4057, 4073, 4079, 4091, 4093, 4099, 4111, 4127, 4129, 4133, 4139, 4153, 4157, 4159, 4177, 4201, 4211, 4217, 4219, 4229, 4231, 4241, 4243, 4253, 4259, 4261, 4271, 4273, 4283, 4289, 4297, 4327, 4337, 4339, 4349, 4357, 4363, 4373, 4391, 4397, 4409, 4421, 4423, 4441, 4447, 4451, 4457, 4463, 4481, 4483, 4493, 4507, 4513, 4517, 4519, 4523, 4547, 4549, 4561, 4567, 4583, 4591, 4597, 4603, 4621, 4637, 4639, 4643, 4649, 4651, 4657, 4663, 4673, 4679, 4691, 4703, 4721, 4723, 4729, 4733, 4751, 4759, 4783, 4787, 4789, 4793, 4799, 4801, 4813, 4817, 4831, 4861, 4871, 4877, 4889, 4903, 4909, 4919, 4931, 4933, 4937, 4943, 4951, 4957, 4967, 4969, 4973, 4987, 4993, 4999, 5003, 5009, 5011, 5021, 5023, 5039, 5051, 5059, 5077, 5081, 5087, 5099, 5101, 5107, 5113, 5119, 5147, 5153, 5167, 5171, 5179, 5189, 5197, 5209, 5227, 5231, 5233, 5237, 5261, 5273, 5279, 5281, 5297, 5303, 5309, 5323, 5333, 5347, 5351, 5381, 5387, 5393, 5399, 5407, 5413, 5417, 5419, 5431, 5437, 5441, 5443, 5449, 5471, 5477, 5479, 5483, 5501, 5503, 5507, 5519, 5521, 5527, 5531, 5557, 5563, 5569, 5573, 5581, 5591, 5623, 5639, 5641, 5647, 5651, 5653, 5657, 5659, 5669, 5683, 5689, 5693, 5701, 5711, 5717, 5737, 5741, 5743, 5749, 5779, 5783, 5791, 5801, 5807, 5813, 5821, 5827, 5839, 5843, 5849, 5851, 5857, 5861, 5867, 5869, 5879, 5881, 5897, 5903, 5923, 5927, 5939, 5953, 5981, 5987, 6007, 6011, 6029, 6037, 6043, 6047, 6053, 6067, 6073, 6079, 6089, 6091, 6101, 6113, 6121, 6131, 6133, 6143, 6151, 6163, 6173, 6197, 6199, 6203, 6211, 6217, 6221, 6229, 6247, 6257, 6263, 6269, 6271, 6277, 6287, 6299, 6301, 6311, 6317, 6323, 6329, 6337, 6343, 6353, 6359, 6361, 6367, 6373, 6379, 6389, 6397, 6421, 6427, 6449, 6451, 6469, 6473, 6481, 6491, 6521, 6529, 6547, 6551, 6553, 6563, 6569, 6571, 6577, 6581, 6599, 6607, 6619, 6637, 6653, 6659, 6661, 6673, 6679, 6689, 6691, 6701, 6703, 6709, 6719, 6733, 6737, 6761, 6763, 6779, 6781, 6791, 6793, 6803, 6823, 6827, 6829, 6833, 6841, 6857, 6863, 6869, 6871, 6883, 6899, 6907, 6911, 6917, 6947, 6949, 6959, 6961, 6967, 6971, 6977, 6983, 6991, 6997, 7001, 7013, 7019, 7027, 7039, 7043, 7057, 7069, 7079, 7103, 7109, 7121, 7127, 7129, 7151, 7159, 7177, 7187, 7193, 7207, 7211, 7213, 7219, 7229, 7237, 7243, 7247, 7253, 7283, 7297, 7307, 7309, 7321, 7331, 7333, 7349, 7351, 7369, 7393, 7411, 7417, 7433, 7451, 7457, 7459, 7477, 7481, 7487, 7489, 7499, 7507, 7517, 7523, 7529, 7537, 7541, 7547, 7549, 7559, 7561, 7573, 7577, 7583, 7589, 7591, 7603, 7607, 7621, 7639, 7643, 7649, 7669, 7673, 7681, 7687, 7691, 7699, 7703, 7717, 7723, 7727, 7741, 7753, 7757, 7759, 7789, 7793, 7817, 7823, 7829, 7841, 7853, 7867, 7873, 7877, 7879, 7883, 7901, 7907, 7919, 7927, 7933, 7937, 7949, 7951, 7963, 7993, 8009, 8011, 8017, 8039, 8053, 8059, 8069, 8081, 8087, 8089, 8093, 8101, 8111, 8117, 8123
};
int MaxStationsCount;
int MaxSid;

void EventAnnounceElections(int parsedMaxStationsCount, int parsedMaxSid) {
    DebugPrint("A %d %d\n", parsedMaxStationsCount, parsedMaxSid);

    // Assign global variables
    MaxStationsCount = parsedMaxStationsCount;
    MaxSid = parsedMaxSid;

    // Initialize Districts array
    for (int i = 0; i < DISTRICTS_SZ; i++) {
        Districts[i].did = -1;       
        Districts[i].seats = 0;
        Districts[i].blanks = 0;
        Districts[i].invalids = 0;
        for(int j=0; j<PARTIES_SZ; j++){
            Districts[i].partyVotes[j] = 0;
        }
    }
    // printf("perasa to district\n");

    // VRISKO TON PROTO PRIME ARITMO MEGALITERO APO TO MAXSTATIONSCOUNT
    for(int i=0; i<PRIMES_SZ; i++){
        if(Primes[i] > MaxStationsCount){
            MaxStationsCount = Primes[i];
            break;
        }
    }
    // printf("perasa to prime\n");
    // Initialize Stations hash table
    StationsHT = (Station**)malloc(MaxStationsCount * sizeof(Station*));
    for(int i=0; i<MaxStationsCount; i++){
        StationsHT[i] = (Station*)malloc(sizeof(Station));
    }
    // printf("perasa to malloc\n");
    if (StationsHT == NULL) {
        printf("Error: Memory allocation failed for StationsHT\n");
        exit(EXIT_FAILURE);
    }
    // Initialize Stations hash table to NULL
    for (int i = 0; i < MaxStationsCount; i++) {
        StationsHT[i]->sid = -1;
        StationsHT[i]->did = -1;
        StationsHT[i]->registered = 0;
        StationsHT[i]->voters = NULL;
        StationsHT[i]->next = NULL;
    }
    // printf("perasa to station\n");
    // Initialize Parties array
    for (int i = 0; i < PARTIES_SZ; i++) {
        Parties[i].pid = i;
        Parties[i].electedCount = 0;
        Parties[i].candidates = NULL; // Points to the root of candidates tree
    }

    // Initialize Parliament pointer to NULL
    Parliament = NULL;

    // Print the output as specified in the PDF
    printf("A %d %d\n", MaxStationsCount, MaxSid);
    printf("DONE\n");
}

void EventCreateDistrict(int did, int seats) {
    // Εκτύπωση των δεδομένων του event
    printf("D %d %d\n", did, seats);  

    // Ελέγχουμε αν το did είναι έγκυρο 
    if (did >= DISTRICTS_SZ) {
        printf("Error: Invalid district ID %d\n", did);
        exit(EXIT_FAILURE);
    }

    // Αναθέτουμε τις τιμές στις περιοχές
    Districts[did].did = did;    
    Districts[did].seats = seats; 
    Districts[did].blanks = 0;
    Districts[did].invalids = 0;
    // memset(Districts[did].partyVotes, 0, sizeof(Districts[did].partyVotes));

    // Εκτύπωση της λίστας των περιοχών
    printf("Districts\n");
    for (int i = 0; i < DISTRICTS_SZ; ++i) {
        if (Districts[i].did != -1) {  // Αν η περιοχή είναι έγκυρη
            printf("%d ", Districts[i].did);
        }
    }


    printf("\nDONE\n");
}

 
void EventCreateStation(int sid, int did) {
    DebugPrint("S %d %d\n", sid, did);
    
    // Υπολογίζουμε την τιμή του κατακερματισμού
    int h = sid % MaxStationsCount;

    // Δημιουργία νέου σταθμού
    Station* newStation = (Station*)malloc(sizeof(Station));
    if (newStation == NULL) {
        printf("Error: Memory allocation failed for new Station\n");
        exit(EXIT_FAILURE);
    }
    newStation->sid = sid;
    newStation->did = did;
    newStation->registered = 0;
    newStation->next = NULL;
    newStation->voters = NULL;

    // Εισάγουμε το νέο σταθμό στην κατάλληλη αλυσίδα
    if (StationsHT[h] == NULL) {
        StationsHT[h] = newStation;
    } else {
        newStation->next = StationsHT[h];
        StationsHT[h] = newStation;
    }

    printf("Stations[%d]\n", h);

    // Εκτύπωση των αναγνωριστικών των εκλογικών τμημάτων στην αλυσίδα
    Station* current = StationsHT[h];
    while (current != NULL) {
        if(current->sid != -1){
            printf("%d", current->sid);
            if (current->next != NULL) {
                printf(", ");
            }
        }
        current = current->next;
    }
    printf("\nDONE\n");
}


void EventRegisterVoter(int vid, int sid) {

    // Αναζητούμε τον σταθμό με το sid
    Station* station = StationsHT[sid % MaxStationsCount];  // Ο πίνακας StationsHT είναι πίνακας κατακερματισμού

    // Βρίσκουμε τον σταθμό που αντιστοιχεί στο sid
    while (station != NULL && station->sid != sid) {
        station = station->next;
    }

    if (station == NULL) {
        // ean den vrethi o stathmos me to sid, return
        printf("Error: Station with id %d not found.\n", sid);
        return;
    }

    // dimiourgoume ton neo voter
    Voter* newVoter = (Voter*)malloc(sizeof(Voter));
    if (newVoter == NULL) {
        printf("Error: Memory allocation failed for new Voter\n");
        exit(EXIT_FAILURE);
    }

    //afkanw kai ton arithmo to registers sto station
    StationsHT[sid % MaxStationsCount]->registered++;

    newVoter->vid = vid;
    newVoter->voted = false;
    newVoter->lc = NULL;
    newVoter->rc = NULL;
    newVoter->parent = NULL;

    // isagoume voter sto dentro
    Voter* current = station->voters;
    Voter* parent = NULL;

    // Ενδοδιατεταγμένη εισαγωγή στο δέντρο (bst)
    while (current != NULL) {
        parent = current;
        if (vid < current->vid) {
            current = current->lc;
        } else {
            current = current->rc;
        }
    }

    if (parent == NULL) {
        station->voters = newVoter;  
    } else if (vid < parent->vid) {
        parent->lc = newVoter;
        newVoter->parent = parent;
    } else {
        parent->rc = newVoter;
        newVoter->parent = parent;
    }

    printf("R %d %d\n", vid, sid);
    printf("Voters[%d]\n", sid);

    // Ενδοδιατεταγμένη διάσχιση και εκτύπωση των IDs
    // Χρησιμοποιούμε μια αναδρομική συνάρτηση για την ενδοδιατεταγμένη διάσχιση (in-order traversal)
    printVotersInOrder(station->voters);

    printf("\nDONE\n");
}

void printVotersInOrder(Voter* node) {
    if (node == NULL) {
        return;
    }
    // Αριστερό υποδέντρο
    printVotersInOrder(node->lc);
    // Εκτύπωση του ID του ψηφοφόρου
    printf("%d", node->vid);
    // Δεξί υποδέντρο
    if (node->rc != NULL) {
        printf(", ");
    }
    printVotersInOrder(node->rc);
}

void EventRegisterCandidate(int cid, int pid, int did) {
    
    // Δημιουργούμε τον νέο υποψήφιο
    Candidate* newCandidate = (Candidate*)malloc(sizeof(Candidate));
    if (newCandidate == NULL) {
        printf("Error: Memory allocation failed for new Candidate\n");
        exit(EXIT_FAILURE);
    }
    newCandidate->cid = cid;
    newCandidate->did = did;
    newCandidate->votes = 0;   // Ξεκινάμε με 0 ψήφους
    newCandidate->isElected = false; // Δεν είναι εκλεγμένος αρχικά
    newCandidate->lc = NULL;   // Αριστερό παιδί
    newCandidate->rc = NULL;   // Δεξί παιδί

    // Εισαγωγή του νέου υποψηφίου στο δέντρο υποψηφίων του κόμματος
    Candidate* current = Parties[pid].candidates; // Υποθέτουμε ότι το δέντρο υποψηφίων βρίσκεται στη δομή του κόμματος
    Candidate* parent = NULL;

    while (current != NULL) {
        parent = current;
        if (cid < current->cid) {
            current = current->lc;
        } else {
            current = current->rc;
        }
    }

    // Τοποθέτηση του νέου υποψηφίου στο σωστό σημείο του δέντρου
    if (parent == NULL) {
        // Το δέντρο ήταν κενό
        Parties[pid].candidates = newCandidate;
    } else if (cid < parent->cid) {
        parent->lc = newCandidate;
    } else {
        parent->rc = newCandidate;
    }

    // Εκτύπωση της κατάστασης του δέντρου υποψηφίων του κόμματος
    printf("C %d %d %d\n", cid, pid, did);
    printf("Candidates[%d]\n", pid);

    // Εκτύπωση των υποψηφίων του κόμματος με ενδοδιατεταγμένη διάσχιση (in-order traversal)
    Candidate* stack[100]; // Στοίβα για την ενδοδιατεταγμένη διάσχιση
    int top = -1;
    current = Parties[pid].candidates;
    
    while (current != NULL || top >= 0) {
        // Μετακίνηση στο αριστερό υποδέντρο όσο υπάρχουν κόμβοι
        while (current != NULL) {
            stack[++top] = current;
            current = current->lc;
        }

        // Αντιμετωπίζουμε τον κόμβο (υποψήφιος)
        current = stack[top--];
        printf("%d %d \n", current->cid, current->did);
        if (top >= 0) {
            printf(",\n ");
        }

        // Μετακίνηση στο δεξί υποδέντρο
        current = current->rc;
    }

    printf("DONE\n");
}

void EventVote(int vid, int sid, int cid, int pid) {
    DebugPrint("V %d %d %d %d\n", vid, sid, cid, pid);

    // vriski to station me to sid
    Station* station = StationsHT[sid % MaxStationsCount];
    
    while (station != NULL && station->sid != sid) {
        station = station->next;
        printf("empika/n");
    }
    printf("station->sid: %d\n", station->sid);

    // vriski ton voter mesa sto station's voter tree
    Voter* voter = NULL;
    Voter* current = station->voters;

        if (current == NULL) {
        printf("Voter tree is empty\n");
    } else {
        printf("Voter tree root: vid=%d\n", current->vid);
    }

    while (current != NULL) {
        if (current->vid == vid) {
            voter = current;
            break;
        } else if (vid < current->vid) {
            current = current->lc;
        } else {
            current = current->rc;
        }
    }

    if (voter == NULL) {
        printf("Voter not found\n");
        return;
    }

    // psifise o voter
    voter->voted = true;

    // Determine the district id
    int did = station->did;

    // Update vote
    if (cid == -1) {
        Districts[did].blanks++;
    } else if (cid == -2) {
        Districts[did].invalids++;
    } else {
        // vriski ton candidate sto party's candidate tree
        Candidate* candidate = NULL;
        Candidate* current = Parties[pid].candidates; //  ipothetioume oti to dentro ipopsifion vriskete stin domi tou kommatos
        Candidate* parent = NULL;

        while (current != NULL) {
            if(current->cid == cid){
                candidate = current;
                break;
            } else if (vid < current->cid) {
                current = current->lc;
            } else {
                current = current->rc;
            }
        }

        if (candidate != NULL) {
            candidate->votes++;
            Districts[did].partyVotes[pid]++;
        }
    }

    // Print information
    printf("District[%d]\n", did);
    printf("blanks %d\n", Districts[did].blanks);
    printf("invalids %d\n", Districts[did].invalids);
    printf("partyVotes\n");

    for (int i = 0; i < PARTIES_SZ; i++) {
        if (i > 0) {
            printf(",\n");
        }
        printf("%d %d", i, Districts[did].partyVotes[i]);
    }
    printf("\nDONE\n");
}


void EventCountVotes(int did) {
    DebugPrint("M %d\n", did);

    // elenxi an district ID ine mesa sto range
    if (did < 0 || did >= DISTRICTS_SZ) {
        printf("Error: Invalid district ID %d\n", did);
        return;
    }

    // Calculate the total valid votes
    int totalValidVotes = 0;
    for (int i = 0; i < PARTIES_SZ; i++) {
        totalValidVotes += Districts[did].partyVotes[i];
    }

    // Calculate the electoral quota
    int seats = Districts[did].seats;
    int electoralQuota = (seats == 0) ? 0 : totalValidVotes / seats;

    // Initialize the partyElected array
    int partyElected[PARTIES_SZ] = {0};

    // Calculate tous candidates pou tha eklegoun
    for (int i = 0; i < PARTIES_SZ; i++) {
        if (electoralQuota == 0) {
            partyElected[i] = 0;
        } else {
            partyElected[i] = Districts[did].partyVotes[i] / electoralQuota;
            if (partyElected[i] > seats) {
                partyElected[i] = seats;
            }
            seats -= partyElected[i];
        }
    }

    // Traverse the candidate tree and select the candidates with the most votes
    printf("M %d\n", did);
    printf("seats %d\n", Districts[did].seats);

    int first = 1; // Flag to handle the comma
    for (int i = 0; i < PARTIES_SZ; i++) {
        Candidate* current = Parties[i].candidates;
        Candidate* stack[100];
        int top = -1;
        Candidate* node = current;
        int electedCount = 0;

        while (node != NULL || top != -1) {
            // Go as left as possible
            while (node != NULL) {
                stack[++top] = node;  // Push node to stack
                node = node->lc;      // Move to left child
            }

            // Pop the last node
            node = stack[top--];

            // Check if the candidate should be elected
            if (electedCount < partyElected[i]) {
                node->isElected = 1;
                if (!first) {
                    printf(",\n");
                }
                printf("%d %d %d", node->cid, i, node->votes);
                first = 0;
                electedCount++;
            }

            // Move to the right child
            node = node->rc;
        }
    }

    printf("\nDONE\n");
}
void EventFormParliament(void) {
    DebugPrint("N\n");

    // Initialize the parliament seats
    int totalSeats = 0;
    for (int did = 0; did < DISTRICTS_SZ; did++) {
        totalSeats += Districts[did].seats;
    }

    // Initialize the elected candidates array
    Candidate* electedCandidates[totalSeats];
    int electedCount = 0;

    // Traverse all districts and select the candidates with the most votes
    for (int did = 0; did < DISTRICTS_SZ; did++) {
        for (int pid = 0; pid < PARTIES_SZ; pid++) {
            Candidate* current = Parties[pid].candidates;
            Candidate* stack[100];
            int top = -1;
            Candidate* node = current;

            while (node != NULL || top != -1) {
                // Go as left as possible
                while (node != NULL) {
                    stack[++top] = node;  // Push node to stack
                    node = node->lc;      // Move to left child
                }

                // Pop the last node
                node = stack[top--];

                // Check if the candidate is elected
                if (node->isElected) {
                    node->pid = pid; // Set the party ID
                    node->did = did; // Set the district ID
                    electedCandidates[electedCount++] = node;
                }

                // Move to the right child
                node = node->rc;
            }
        }
    }

    // Sort the elected candidates array in descending order by cid
    for (int i = 0; i < electedCount - 1; i++) {
        for (int j = 0; j < electedCount - i - 1; j++) {
            if (electedCandidates[j]->cid < electedCandidates[j + 1]->cid) {
                Candidate* temp = electedCandidates[j];
                electedCandidates[j] = electedCandidates[j + 1];
                electedCandidates[j + 1] = temp;
            }
        }
    }

    // Print the elected members
    printf("N\n");
    printf("members\n");
    for (int i = 0; i < electedCount; i++) {
        Candidate* candidate = electedCandidates[i];
        printf("%d %d %d", candidate->cid, candidate->pid, candidate->did);
        if (i < electedCount - 1) {
            printf(",\n");
        }
    }
    printf("\nDONE\n");
}

void EventPrintDistrict(int did) {
    DebugPrint("I %d\n", did);

    // elexoume tο did an ine engkiro
    if (did >= DISTRICTS_SZ || Districts[did].did == -1) {
        printf("Error: Invalid district ID %d\n", did);
        return;
    }

    // Print
    printf("I %d\n", did);
    printf("seats %d\n", Districts[did].seats);
    printf("blanks %d\n", Districts[did].blanks);
    printf("invalids %d\n", Districts[did].invalids);
    printf("partyVotes\n");

    // Print to psifon ton kommaton
    for (int i = 0; i < PARTIES_SZ; i++) {
        printf("%d %d", i, Districts[did].partyVotes[i]);
        if (i < PARTIES_SZ - 1) {
            printf(", ");
        }
    }

    printf("\nDONE\n");
}
void EventPrintStation(int sid) {
    DebugPrint("J %d\n", sid);

    // Ensure the station ID is within the valid range
    if (sid < 0 || sid >= MaxStationsCount) {
        printf("Error: Invalid station ID %d\n", sid);
        return;
    }

    // Find the station with the given sid in the hash table
    Station* station = StationsHT[sid % MaxStationsCount];

    // Traverse the linked list to find the exact station
    while (station != NULL && station->sid != sid) {
        station = station->next;
    }

    // If the station is not found
    if (station == NULL) {
        printf("Error: Station not found\n");
        return;
    }

    // Print the number of registered voters
    printf("J %d\n", sid);
    printf("registered %d\n", station->registered);
    printf("voters\n");

    // Perform in-order traversal of the voter tree
    Voter* current = station->voters;
    Voter* stack[100];  // Stack for in-order traversal
    int top = -1;
    Voter* node = current;
    int first = 1; // Flag to handle the comma

    while (node != NULL || top != -1) {
        // Go as left as possible
        while (node != NULL) {
            stack[++top] = node;  // Push node to stack
            node = node->lc;      // Move to left child
        }

        // Pop the last node
        node = stack[top--];

        // Print the voter's ID and whether they voted
        if (!first) {
            printf(",\n");
        }
        printf("%d %d", node->vid, node->voted);
        first = 0;

        // Move to the right child
        node = node->rc;
    }

    printf("\nDONE\n");
}

void EventPrintParty(int pid) {
    DebugPrint("K %d\n", pid);

    // Ensure the party ID is within the valid range
    if (pid < 0 || pid >= PARTIES_SZ) {
        printf("Error: Invalid party ID %d\n", pid);
        return;
    }

    // Find the party with the given pid
    Party* party = &Parties[pid];

    // If the party is not found
    if (party == NULL) {
        printf("Error: Party not found\n");
        return;
    }

    // Print the party ID
    printf("K %d\n", pid);
    printf("elected\n");

    // Perform in-order traversal of the candidate tree
    Candidate* current = party->candidates;
    Candidate* stack[100];  // Stack for in-order traversal
    int top = -1;
    Candidate* node = current;
    int first = 1; // Flag to handle the comma

    while (node != NULL || top != -1) {
        // Go as left as possible
        while (node != NULL) {
            stack[++top] = node;  // Push node to stack
            node = node->lc;      // Move to left child
        }

        // Pop the last node
        node = stack[top--];

        // Print the candidate's ID and votes
        if (!first) {
            printf(",\n");
        }
        printf("%d %d", node->cid, node->votes);
        first = 0;

        // Move to the right child
        node = node->rc;
    }

    printf("\nDONE\n");
}
void EventPrintParliament(void) {
    DebugPrint("L\n");
    // TODO
}
void EventBonusUnregisterVoter(int vid, int sid) {
    DebugPrint("BU %d %d\n", vid, sid);
    // TODO
}
void EventBonusFreeMemory(void) {
    DebugPrint("BF\n");
    // TODO
}
