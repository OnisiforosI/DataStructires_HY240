/*
 * ============================================
 * file: main.c
 * @Author John Malliotakis (jmal@csd.uoc.gr)
 * @Version 24/10/2023
 *
 * @e-mail hy240@csd.uoc.gr
 *
 * @brief Main function
 *        for CS240 Project Phase 1,
 *        Winter Semester 2024-2025
 * @see   Compile using supplied Makefile by running: make
 * ============================================
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "elections.h"

/* Maximum input line size */
#define MAX_LINE 1024

/* 
 * Uncomment the following line to
 * enable debugging prints
 * or comment to disable it
 */
/* #define DEBUG */
#ifdef DEBUG
#define DPRINT(...) fprintf(stderr, __VA_ARGS__);
#else
#define DPRINT(...)
#endif /* DEBUG */

/*
 * Globals:
 * you may add some here for certain events
 * (such as D and P)
 */
struct district Districts[56];
struct party Parties[5];
struct parliament Parliament;

void destroy_structures(void)
{
	/*
	 * TODO: For a bonus
	 * empty all structures
	 * and free all memory associated
	 * with list nodes here
	 */
}

void announce_elections(void) {
    int i;

    /* Αρχικοποίηση των districts */
    for (i = 0; i < 56; i++) {
        Districts[i].did = -1;
        Districts[i].seats = 0;
        Districts[i].allotted = 0;
        Districts[i].blanks = 0;
        Districts[i].voids = 0;
        Districts[i].stations = NULL;
        Districts[i].candidates = NULL;
    }

    /* Αρχικοποίηση των parties */
    for (i = 0; i < 5; i++) {
        Parties[i].pid = -1;
        Parties[i].nelected = 0;
        Parties[i].elected = NULL;
    }

    /* Αρχικοποίηση του parliament */
    Parliament.members = NULL;
}


int create_district(int did, int seats) {
    if (did < 1 || did >= 57) {
        return 1; // FAIL, αν η περιοχή είναι εκτός ορίων
    }

    Districts[did-1].did = did;
    Districts[did-1].seats = seats;
    Districts[did-1].allotted = 0;
    Districts[did-1].blanks = 0;
    Districts[did-1].voids = 0;
    Districts[did-1].stations = NULL;
    Districts[did-1].candidates = NULL;

print_district(did);
    return 0; 
}

int create_station(int sid, int did)
{
    if (did < 1 || did >= 57) {
        return 1; // FAIL, αν η περιοχή δεν υπάρχει
    }

    struct station *new_station = malloc(sizeof(struct station));
    if (!new_station) return 1; // FAIL αν δεν υπάρχει μνήμη

    new_station->sid = sid;
    new_station->registered = 0;
    new_station->voters = NULL;
    new_station->vsentinel = NULL;
    new_station->next = Districts[did-1].stations;
    Districts[did-1].stations = new_station;

print_station(sid,did);
    return 0; 

}

void create_party(int pid)
{
    if (pid < 0 || pid >= 5) return; // FAIL αν το κόμμα είναι εκτός ορίων

    Parties[pid].pid = pid;
    Parties[pid].nelected = 0;
    Parties[pid].elected = NULL;

    print_party(pid);
}

int register_candidate(int cid, int did, int pid)
{
    if (did < 1 || did >= 57 || pid < 0 || pid >= 5) {
        return 1; // FAIL αν η περιοχή ή το κόμμα είναι εκτός ορίων
    }

    struct candidate *new_candidate = malloc(sizeof(struct candidate));
    if (!new_candidate) return 1; // FAIL αν δεν υπάρχει μνήμη

    new_candidate->cid = cid;
    new_candidate->pid = pid;
    new_candidate->votes = 0;
    new_candidate->elected = 0;
    new_candidate->next = Districts[did-1].candidates;
    new_candidate->prev = NULL;

    if (Districts[did-1].candidates) {
        Districts[did-1].candidates->prev = new_candidate;
    }

    Districts[did-1].candidates = new_candidate;

    print_district(did);
    return 0; // Επιτυχία
}

int register_voter(int vid, int did, int sid)
{
    if (did < 1 || did >= 57 || sid < 1) {
        return 1; // FAIL αν η περιοχή ή ο σταθμός δεν είναι έγκυρος
    }

    struct station *station = Districts[did-1].stations;
    while (station) {
        if (station->sid == sid) break;
        station = station->next;
    }

    if (!station) return 1; // Αποτυχία αν δεν βρέθηκε ο σταθμός

    struct voter *new_voter = malloc(sizeof(struct voter));
    if (!new_voter) return 1; // Αποτυχία αν δεν υπάρχει μνήμη

    new_voter->vid = vid;
    new_voter->voted = 0;
    new_voter->next = station->voters;
    station->voters = new_voter;

    station->registered++;

   print_station( sid,  did);
    return 0; // Επιτυχία
}

int unregister_voter(int vid)
{
    // Εύρεση και διαγραφή του ψηφοφόρου
    for (int i = 0; i < 56; i++) {
        struct station *station = Districts[i].stations;
        while (station) {
            struct voter *voter = station->voters;
            struct voter *prev = NULL;
            while (voter) {
                if (voter->vid == vid) {
                    if (prev) {
                        prev->next = voter->next;
                    } else {
                        station->voters = voter->next;
                    }
                    free(voter);
                    station->registered--;
                    return 0; // Επιτυχία
                }
                prev = voter;
                voter = voter->next;
            }
            station = station->next;
        }
    }
    return 1; // Αποτυχία αν δεν βρέθηκε ο ψηφοφόρος
}

int vote(int vid, int sid, int cid)
{
    // Έλεγχος αν ο σταθμός και η περιφέρεια υπάρχουν
    for (int i = 0; i < 56; i++) {
        struct station *station = Districts[i].stations;

        while (station) {
            if (station->sid == sid) {
                // Έλεγχος αν ο ψηφοφόρος υπάρχει στον σταθμό
                struct voter *voter = station->voters;
                while (voter) {
                    if (voter->vid == vid) {
                        if (voter->voted) {
                            // Ο ψηφοφόρος έχει ήδη ψηφίσει
                            return 1;
                        }

                        // Ψήφος για λευκό ή άκυρο
                        if (cid == 0) {
                            Districts[i].blanks++;
                            voter->voted = 1;
                        } else if (cid == 1) {
                            Districts[i].voids++;
                            voter->voted = 1;
                        } else {
                            // Εύρεση του υποψηφίου
                            struct candidate *candidate = Districts[i].candidates;
                            while (candidate) {
                                if (candidate->cid == cid) {
                                    // Αύξηση ψήφων για τον υποψήφιο
                                    candidate->votes++;
                                    voter->voted = 1;

                                    // Ταξινόμηση: Μετακίνηση του υποψηφίου στη λίστα
                                    while (candidate->prev && candidate->votes > candidate->prev->votes) {
                                        // Swap τα στοιχεία (ασφαλές swap μόνο των δεδομένων)
                                        int temp_cid = candidate->cid;
                                        int temp_pid = candidate->pid;
                                        int temp_votes = candidate->votes;

                                        candidate->cid = candidate->prev->cid;
                                        candidate->pid = candidate->prev->pid;
                                        candidate->votes = candidate->prev->votes;

                                        candidate->prev->cid = temp_cid;
                                        candidate->prev->pid = temp_pid;
                                        candidate->prev->votes = temp_votes;

                                        candidate = candidate->prev;
                                    }

                                    break;
                                }
                                candidate = candidate->next;
                            }
                        }

                        // Εκτύπωση σύμφωνα με το πρότυπο:
                        printf("V %d %d %d\n", vid, sid, cid);
                        printf("District %d\n", i + 1);

                        // Εκτύπωση των ψήφων των υποψηφίων
                        printf("Candidate votes = ");
                        struct candidate *candidate = Districts[i].candidates;
                        int first = 1;
                        while (candidate) {
                            if (!first) {
                                printf(", ");
                            }
                            printf("(%d, %d)", candidate->cid, candidate->votes);
                            first = 0;
                            candidate = candidate->next;
                        }
                        printf("\n");

                        // Εκτύπωση του αριθμού των λευκών και άκυρων ψήφων
                        printf("Blanks %d\n", Districts[i].blanks);
                        printf("Voids = %d\n", Districts[i].voids);
                        printf("DONE\n");

                        return 0; // Επιτυχία
                    }
                    voter = voter->next;
                }
                return 1; // Ο ψηφοφόρος δεν βρέθηκε
            }
            station = station->next;
        }
    }

    return 1; // Σταθμός δεν βρέθηκε
}



void delete_empty_stations(void)
{
    int i;
    int deleted_any = 0; // Flag για να ελέγξουμε αν διαγράφηκε κάποιος σταθμός

    for (i = 0; i < 56; i++) {
        struct station *station = Districts[i].stations;
        struct station *prev_station = NULL;

        while (station) {
            if (station->registered == 0) {
                // Εκτύπωση σύμφωνα με το πρότυπο: <sid,> <did,>
                printf("%d, %d,\n", station->sid, i + 1);
                deleted_any = 1; // Σημειώνουμε ότι διαγράφηκε σταθμός

                // Διαγραφή του σταθμού από τη λίστα
                if (prev_station) {
                    prev_station->next = station->next;
                } else {
                    Districts[i].stations = station->next;
                }

                free(station);
                station = (prev_station) ? prev_station->next : Districts[i].stations;
            } else {
                prev_station = station;
                station = station->next;
            }
        }
    }

    if (deleted_any) {
        printf("DONE\n"); // Εκτυπώνουμε DONE αν διαγράφηκαν σταθμοί
    }
}


/* Καταμέτρηση ψήφων (M) */
void count_votes(int did)
{
    if (did < 1 || did >= 57) return;  // Έλεγχος εγκυρότητας

    struct district *district = &Districts[did-1];
    struct candidate *candidate = district->candidates;
    int total_votes = 0;
    int seats_to_allocate;

    // Υπολογισμός του συνολικού αριθμού των ψήφων
    while (candidate) {
        total_votes += candidate->votes;
        candidate = candidate->next;
    }

    // Επιστροφή αν δεν υπάρχουν ψήφοι 
    if (total_votes == 0) {
        printf("No votes in district %d.\n", did);
        return;
    }

    // Κατανομή εδρών ανά κόμμα
    for (int i = 0; i < 5; i++) {
        struct party *party = &Parties[i];
        
        // Εάν το κόμμα έχει υποψηφίους
        if (party->nelected == 0) continue;

        // Υπολογισμός των εδρών που αναλογούν στο κόμμα
        seats_to_allocate = (party->nelected * district->seats) / total_votes;

        struct candidate *party_candidate = district->candidates;
        while (party_candidate && seats_to_allocate > 0) {
            if (party_candidate->pid == party->pid) {
                party_candidate->elected = 1;
                seats_to_allocate--;
            }
            party_candidate = party_candidate->next;
        }

        // Ενημερώνουμε την περιφέρεια με τις εδρές που ανατέθηκαν
        district->allotted += seats_to_allocate;
    }
    print_district(did);
}

/* Δημιουργία κυβέρνησης (G) */
void form_government(void)
{
    int remaining_seats;
    int i, j;

    // Διανείμουμε τις έδρες για κάθε περιφέρεια
    for (i = 0; i < 56; i++) {
        struct district *district = &Districts[i];
        remaining_seats = district->seats - district->allotted;

        // Εξετάζουμε τα κόμματα και διανέμουμε τις έδρες
        for (j = 0; j < 5; j++) {
            struct party *party = &Parties[j];
            int seats_to_allocate = remaining_seats * party->nelected / district->seats;

            // Αναθέτουμε τις έδρες στους εκλεγμένους υποψηφίους του κόμματος
            struct candidate *candidate = district->candidates;
            while (candidate && seats_to_allocate > 0) {
                if (candidate->pid == party->pid) {
                    candidate->elected = 1;
                    seats_to_allocate--;
                    party->nelected++;
                }
                candidate = candidate->next;
            }

            // Μειώνουμε τις διαθέσιμες έδρες
            remaining_seats -= seats_to_allocate;
        }

        district->allotted = district->seats - remaining_seats;
    }
}


/* Δημιουργία κοινοβουλίου (N) */
void form_parliament(void)
{
    // Συγκέντρωση όλων των εκλεγμένων υποψηφίων από τα κόμματα
    struct candidate *all_candidates = NULL;
    for (int i = 0; i < 5; i++) {
        struct party *party = &Parties[i];
        struct candidate *current_candidate = party->elected;

        // Προσθήκη υποψηφίων του κόμματος στην λίστα
        while (current_candidate) {
            struct candidate *new_candidate = malloc(sizeof(struct candidate));
            if (!new_candidate) return; // Αποτυχία αν δεν υπάρχει μνήμη

            *new_candidate = *current_candidate;
            new_candidate->next = all_candidates;
            all_candidates = new_candidate;

            current_candidate = current_candidate->next;
        }
    }

    // Ταξινόμηση της λίστας με βάση τις ψήφους (φθίνουσα σειρά)
    struct candidate *sorted_list = NULL;
    while (all_candidates) {
        struct candidate *max_candidate = all_candidates;
        struct candidate *iter = all_candidates->next;

        // Βρίσκουμε τον υποψήφιο με τις περισσότερες ψήφους
        while (iter) {
            if (iter->votes > max_candidate->votes) {
                max_candidate = iter;
            }
            iter = iter->next;
        }

        // Αφαιρούμε τον υποψήφιο με τις περισσότερες ψήφους από τη λίστα
        if (max_candidate == all_candidates) {
            all_candidates = all_candidates->next;
        } else {
            struct candidate *prev = all_candidates;
            while (prev->next != max_candidate) {
                prev = prev->next;
            }
            prev->next = max_candidate->next;
        }

        // Προσθέτουμε τον υποψήφιο με τις περισσότερες ψήφους στην ταξινομημένη λίστα
        max_candidate->next = sorted_list;
        sorted_list = max_candidate;
    }

    // Ενημερώνουμε το κοινοβούλιο με τη ταξινομημένη λίστα υποψηφίων
    Parliament.members = sorted_list;

    // Καλούμε την print_parliament για να τυπώσουμε τα μέλη του κοινοβουλίου
    print_parliament();
}


/* Εκτύπωση της περιοχής (I) */
void print_district(int did) {
    if (did < 1 || did >= 57) {
        printf("Invalid district ID\n");
        return;
    }

    struct district *district = &Districts[did - 1];  // Αναζήτηση της περιφέρειας
    printf("I %d\n", did);  // Εκτυπώνεται το Event I και το did

    printf("Seats %d\n", district->seats);
    printf("Blanks = %d\n", district->blanks);
    printf("Voids = %d\n", district->voids);

    // Εκτύπωση των υποψηφίων
    printf("Candidates =\n");
    struct candidate *candidate = district->candidates;
    while (candidate) {
        printf("<%d,> <%d,> <%d,>\n", candidate->cid, candidate->pid, candidate->votes);
        candidate = candidate->next;
    }

    // Εκτύπωση των εκλογικών τμημάτων
    printf("Stations » ");
    struct station *station = district->stations;
    int first = 1;
    while (station) {
        if (!first) printf(", ");
        printf("<%d>", station->sid);
        first = 0;
        station = station->next;
    }
    printf("\nDONE\n");
}


/* Εκτύπωση του σταθμού (J) */
void print_station(int sid, int did) {
    if (did < 1 || did >= 57|| sid < 1) {
        printf("Invalid station or district ID\n");
        return;
    }

    struct station *station = Districts[did - 1].stations;
    while (station) {
        if (station->sid == sid) {
            printf("J %d\n", sid);
            printf("Registered %d Voters.\n", station->registered);

            // Εκτύπωση των ψηφοφόρων και του αν ψήφισαν ή όχι
            struct voter *voter = station->voters;
            while (voter) {
                printf("<%d,> <%d>\n", voter->vid, voter->voted);
                voter = voter->next;
            }
            printf("DONE\n");
            return;
        }
        station = station->next;
    }

    printf("Station not found\n");
}


/* Εκτύπωση του κόμματος (K) */
void print_party(int pid) {
    if (pid < 0 || pid >= 5){
    printf("Invalid party ID\n");
        return;
    }

    struct party *party = &Parties[pid];
    printf("K %d\n", pid);
    printf("Elected.\n");

    struct candidate *candidate = party->elected;
    while (candidate) {
        printf("<%d,> <%d>\n", candidate->cid, candidate->votes);
        candidate = candidate->next;
    }

    printf("DONE\n");
}


/* Εκτύπωση του κοινοβουλίου (L) */
void print_parliament(void) {
    struct candidate *current = Parliament.members;
    printf("Members\n");

    while (current) {
        printf("<%d,> <%d,> <%d>\n", current->cid, current->pid, current->votes);
        current = current->next;
    }

    printf("DONE\n");
}


int main(int argc, char *argv[])
{
	FILE *event_file;
	char line_buffer[MAX_LINE];

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <input_file>\n",
				argv[0]);
		exit(EXIT_FAILURE);
	}

	event_file = fopen(argv[1], "r");
	if (!event_file) {
		perror("fopen error for event file open");
		exit(EXIT_FAILURE);
	}

	while (fgets(line_buffer, MAX_LINE, event_file)) {
		char *trimmed_line;
		char event;
		int did, sid, vid, cid, pid, seats;
		int ret = 0;
		/*
		 * First trim any whitespace
		 * leading the line.
		 */
		trimmed_line = line_buffer;
		while (trimmed_line && isspace(*trimmed_line))
			trimmed_line++;
		if (!trimmed_line)
			continue;
		/* 
		 * Find the event,
		 * or comment starting with #
		 */
		if (sscanf(trimmed_line, "%c", &event) != 1) {
			fprintf(stderr, "Could not parse event type out of input line:\n\t%s",
					trimmed_line);
			fclose(event_file);
			exit(EXIT_FAILURE);
		}

		switch (event) {
			/* Comment, ignore this line */
			case '#':
				break;
			case 'A':
				announce_elections();
				fprintf(stdout, "A Done\n");
				break;
			case 'D':
				if (sscanf(trimmed_line, "D %d %d", &did, &seats) != 2) {
					fprintf(stderr, "Event D parsing error\n");
					ret = 1;
					break;
				}
				ret = create_district(did, seats);
				break;
			case 'S':
				if (sscanf(trimmed_line, "S %d %d", &sid, &did) != 2) {
					fprintf(stderr, "Event S parsing error\n");
					ret = 1;
					break;
				}
				ret = create_station(sid, did);
				break;
			case 'P':
				if (sscanf(trimmed_line, "P %d", &pid) != 1) {
					fprintf(stderr, "Event P parsing error\n");
					ret = 1;
					break;
				}
				create_party(pid);
				break;
			case 'C':
				if (sscanf(trimmed_line, "C %d %d %d", &cid, &did, &pid) != 3) {
					fprintf(stderr, "Event C parsing error\n");
					ret = 1;
					break;
				}
				ret = register_candidate(cid, did, pid);
				break;
			case 'R':
				if (sscanf(trimmed_line, "R %d %d %d", &vid, &did, &sid) != 3) {
					fprintf(stderr, "Event R parsing error\n");
					ret = 1;
					break;
				}
				ret = register_voter(vid, did, sid);
				break;
			case 'U':
				if (sscanf(trimmed_line, "U %d", &vid) != 1) {
					fprintf(stderr, "Event U parsing error\n");
					ret = 1;
					break;
				}
				ret = unregister_voter(vid);
				break;
			case 'E':
				delete_empty_stations();
				break;
			case 'V':
				if (sscanf(trimmed_line, "V %d %d %d", &vid, &sid, &cid) != 3) {
					fprintf(stderr, "Event V parsing error\n");
					ret = 1;
					break;
				}
				ret = vote(vid, sid, cid);
				break;
			case 'M':
				if (sscanf(trimmed_line, "M %d", &did) != 1) {
					fprintf(stderr, "Event M parsing error\n");
					ret = 1;
					break;
				}
				count_votes(did);
			case 'G':
				form_government();
				break;
			case 'N':
				form_parliament();
				break;
			case 'I':
				if (sscanf(trimmed_line, "I %d", &did) != 1) {
					fprintf(stderr, "Event I parsing error\n");
					ret = 1;
					break;
				}
				print_district(did);
			case 'J':
				if (sscanf(trimmed_line, "J %d %d", &sid, &did) != 2) {
					fprintf(stderr, "Event J parsing error\n");
					ret = 1;
					break;
				}
				print_station(sid, did);
				break;
			case 'K':
				if (sscanf(trimmed_line, "K %d", &pid) != 1) {
					fprintf(stderr, "Event I parsing error\n");
					ret = 1;
					break;
				}
				print_party(pid);
				break;
			case 'L':
				print_parliament();
				break;
			default:
				fprintf(stderr, "WARNING: Unrecognized event %c. Continuing...\n",
						event);
				ret = 1;
				break;
		}
		fprintf(stderr, "Event %c %s.\n", event,
				ret ? "failed" : "succeeded");
	}
	fclose(event_file);
	destroy_structures();
	return 0;
}
