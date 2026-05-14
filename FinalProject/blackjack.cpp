// ============================================================
//  BLACKJACK CASINO SYSTEM
//  CPE112: Programming with Data Structures — Final Project
//  2/2025 Semester
//
//  Data Structures Used:
//    1. Stack        — Card deck (deal from top) + Bet undo history
//    2. Linked List  — Each player's hand (dynamic card storage)
//    3. Queue        — Player turn order (FIFO)
//    4. Hash Table   — Leaderboard / player stats (O(1) lookup)
//
//  Algorithm:
//    - Fisher-Yates Shuffle (O(n)) for deck randomization
//    - Ace resolution (greedy reduction: Ace = 11 → 1 if bust)
//    - Insertion Sort on leaderboard by chip count
//    - Hash function: polynomial rolling hash with linear probing
//
//  Compile:  g++ -o blackjack blackjack.cpp
//  Run:      ./blackjack
// ============================================================

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <climits>
using namespace std;

// ============================================================
//  CONSTANTS
// ============================================================
#define DECK_SIZE   52
#define MAX_PLAYERS  4
#define HASH_SIZE   101   // prime number → better distribution
#define START_CHIPS 1000

// ============================================================
//  CARD STRUCTURE
// ============================================================
struct Card {
    char suit[10];  // "Hearts" | "Diamonds" | "Clubs" | "Spades"
    char rank[5];   // "2"–"10" | "J" | "Q" | "K" | "A"
    int  value;     // numeric value (Ace starts as 11)
};

// ============================================================
//  DATA STRUCTURE 1: STACK
//  Used for: (a) Card deck — cards are pushed in, dealt from top (LIFO)
//            (b) Bet undo history — store bets, pop to undo
//
//  Why Stack?
//    • Dealing a card = pop from top → O(1)
//    • Undo last action = pop from top → O(1)
//    • Alternative: Array with index — Stack is cleaner & dynamic
// ============================================================

// --- Generic Card Stack (Deck) ---
struct StackNode {
    Card        card;
    StackNode*  next;
};

struct Stack {
    StackNode* top;
    int        size;
};

void  initStack      (Stack* s)         { s->top = NULL; s->size = 0; }
bool  isStackEmpty   (Stack* s)         { return s->top == NULL; }

void  pushCard       (Stack* s, Card c) {
    StackNode* node = new StackNode();
    node->card = c;
    node->next = s->top;
    s->top     = node;
    s->size++;
}

Card  popCard        (Stack* s) {
    if (isStackEmpty(s)) { Card empty = {"", "", 0}; return empty; }
    StackNode* temp = s->top;
    Card c          = temp->card;
    s->top          = s->top->next;
    delete temp;
    s->size--;
    return c;
}

void  freeStack      (Stack* s) {
    while (!isStackEmpty(s)) popCard(s);
}

// --- Bet Undo Stack ---
struct BetRecord {
    int playerIndex;
    int betAmount;
};

struct BetNode {
    BetRecord  record;
    BetNode*   next;
};

struct BetStack {
    BetNode* top;
};

void      initBetStack  (BetStack* bs)                   { bs->top = NULL; }
bool      isBetEmpty    (BetStack* bs)                   { return bs->top == NULL; }

void      pushBet       (BetStack* bs, int idx, int amt) {
    BetNode* node    = new BetNode();
    node->record     = {idx, amt};
    node->next       = bs->top;
    bs->top          = node;
}

BetRecord popBet        (BetStack* bs) {
    BetRecord empty  = {-1, 0};
    if (isBetEmpty(bs)) return empty;
    BetNode* temp    = bs->top;
    BetRecord r      = temp->record;
    bs->top          = bs->top->next;
    delete temp;
    return r;
}

void      freeBetStack  (BetStack* bs) {
    while (!isBetEmpty(bs)) popBet(bs);
}

// ============================================================
//  DATA STRUCTURE 2: LINKED LIST
//  Used for: Each player's hand of cards
//
//  Why Linked List?
//    • Number of cards in hand is unknown at runtime → dynamic size
//    • Inserting a new card = O(1) append (or O(n) traverse to tail)
//    • No wasted memory unlike a fixed array
//    • Alternative: vector<Card> — Linked List avoids reallocation overhead
// ============================================================
struct HandNode {
    Card      card;
    HandNode* next;
};

struct Hand {
    HandNode* head;
    int       count;
};

void initHand      (Hand* h) { h->head = NULL; h->count = 0; }

void addCardToHand (Hand* h, Card c) {
    HandNode* node = new HandNode();
    node->card     = c;
    node->next     = NULL;
    if (h->head == NULL) {
        h->head = node;
    } else {
        HandNode* curr = h->head;
        while (curr->next) curr = curr->next;
        curr->next = node;
    }
    h->count++;
}

void clearHand     (Hand* h) {
    HandNode* curr = h->head;
    while (curr) {
        HandNode* temp = curr;
        curr = curr->next;
        delete temp;
    }
    h->head  = NULL;
    h->count = 0;
}

// Ace resolution algorithm: Ace = 11, reduce to 1 if hand > 21
int  calculateHandValue (Hand* h) {
    int total = 0, aces = 0;
    HandNode* curr = h->head;
    while (curr) {
        total += curr->card.value;
        if (strcmp(curr->card.rank, "A") == 0) aces++;
        curr = curr->next;
    }
    while (total > 21 && aces > 0) { total -= 10; aces--; }
    return total;
}

void displayHand (Hand* h, const char* name, bool hideFirst = false) {
    cout << "  " << name << ": ";
    HandNode* curr = h->head;
    bool isFirst   = true;
    while (curr) {
        if (isFirst && hideFirst)
            cout << "[??] ";
        else
            cout << curr->card.rank << "-" << curr->card.suit[0] << " ";
        isFirst = false;
        curr    = curr->next;
    }
    if (!hideFirst)
        cout << "  (Total: " << calculateHandValue(h) << ")";
    cout << "\n";
}

bool isBlackjack (Hand* h) { return h->count == 2 && calculateHandValue(h) == 21; }
bool isBust      (Hand* h) { return calculateHandValue(h) > 21; }

// ============================================================
//  DATA STRUCTURE 3: QUEUE
//  Used for: Player turn order
//
//  Why Queue?
//    • Turns go in the order players sat down (FIFO)
//    • Enqueue all players at round start, dequeue each turn
//    • O(1) enqueue and dequeue
//    • Alternative: Loop with index — Queue makes turn order explicit & flexible
// ============================================================
struct QueueNode {
    int        playerIndex;
    QueueNode* next;
};

struct Queue {
    QueueNode* front;
    QueueNode* rear;
    int        size;
};

void initQueue   (Queue* q) { q->front = NULL; q->rear = NULL; q->size = 0; }
bool isQueueEmpty(Queue* q) { return q->front == NULL; }

void enqueue     (Queue* q, int idx) {
    QueueNode* node  = new QueueNode();
    node->playerIndex = idx;
    node->next       = NULL;
    if (q->rear == NULL) { q->front = q->rear = node; }
    else { q->rear->next = node; q->rear = node; }
    q->size++;
}

int  dequeue     (Queue* q) {
    if (isQueueEmpty(q)) return -1;
    QueueNode* temp = q->front;
    int idx         = temp->playerIndex;
    q->front        = q->front->next;
    if (q->front == NULL) q->rear = NULL;
    delete temp;
    q->size--;
    return idx;
}

void clearQueue  (Queue* q) { while (!isQueueEmpty(q)) dequeue(q); }

// ============================================================
//  DATA STRUCTURE 4: HASH TABLE
//  Used for: Player leaderboard / stats storage
//
//  Why Hash Table?
//    • O(1) average lookup by player name
//    • Persistent stats across multiple rounds
//    • Collision resolved with linear probing
//    • Hash: polynomial rolling hash → good distribution
//    • Alternative: Sorted array — O(log n) search vs O(1) hash lookup
// ============================================================
struct PlayerStats {
    char name[50];
    int  wins;
    int  losses;
    int  draws;
    int  totalChips;
    bool occupied;
};

struct HashTable {
    PlayerStats table[HASH_SIZE];
};

void initHashTable (HashTable* ht) {
    for (int i = 0; i < HASH_SIZE; i++)
        ht->table[i].occupied = false;
}

// Polynomial rolling hash function
int hashFn (const char* name) {
    int hash = 0;
    for (int i = 0; name[i] != '\0'; i++)
        hash = (hash * 31 + (unsigned char)name[i]) % HASH_SIZE;
    return hash;
}

void insertStats (HashTable* ht, const char* name, int chips) {
    int idx = hashFn(name);
    while (ht->table[idx].occupied && strcmp(ht->table[idx].name, name) != 0)
        idx = (idx + 1) % HASH_SIZE;   // linear probing
    if (!ht->table[idx].occupied) {
        strcpy(ht->table[idx].name, name);
        ht->table[idx] = {"", 0, 0, 0, chips, true};
        strcpy(ht->table[idx].name, name);
        ht->table[idx].totalChips = chips;
    }
}

PlayerStats* findStats (HashTable* ht, const char* name) {
    int idx   = hashFn(name);
    int start = idx;
    while (ht->table[idx].occupied) {
        if (strcmp(ht->table[idx].name, name) == 0)
            return &ht->table[idx];
        idx = (idx + 1) % HASH_SIZE;
        if (idx == start) break;
    }
    return NULL;
}

void updateStats (HashTable* ht, const char* name, int result, int chips) {
    // result: 1 = win, -1 = loss, 0 = draw
    PlayerStats* ps = findStats(ht, name);
    if (!ps) return;
    if      (result ==  1) ps->wins++;
    else if (result == -1) ps->losses++;
    else                   ps->draws++;
    ps->totalChips = chips;
}

// Insertion Sort (ascending by totalChips, reversed for display)
void displayLeaderboard (HashTable* ht) {
    PlayerStats sorted[HASH_SIZE];
    int count = 0;
    for (int i = 0; i < HASH_SIZE; i++)
        if (ht->table[i].occupied)
            sorted[count++] = ht->table[i];

    // Insertion sort descending by totalChips
    for (int i = 1; i < count; i++) {
        PlayerStats key = sorted[i];
        int j = i - 1;
        while (j >= 0 && sorted[j].totalChips < key.totalChips) {
            sorted[j + 1] = sorted[j]; j--;
        }
        sorted[j + 1] = key;
    }

    cout << "\n╔══════════════════════════════════════════╗\n";
    cout <<   "║            🏆  LEADERBOARD               ║\n";
    cout <<   "╠══════════════════════════════════════════╣\n";
    cout <<   "║  #   Name            Chips   W    L    D ║\n";
    cout <<   "╠══════════════════════════════════════════╣\n";
    for (int i = 0; i < count; i++) {
        printf("║  %-3d %-15s %-7d %-4d %-4d %-3d║\n",
            i + 1,
            sorted[i].name,
            sorted[i].totalChips,
            sorted[i].wins,
            sorted[i].losses,
            sorted[i].draws);
    }
    cout << "╚══════════════════════════════════════════╝\n";
}

// ============================================================
//  PLAYER STRUCTURE
// ============================================================
struct Player {
    char name[50];
    int  chips;
    int  bet;
    Hand hand;
    bool isActive;    // still has chips / in the game
    bool isBust;
    bool isBlackjackWin;
};

// ============================================================
//  ALGORITHM: FISHER-YATES SHUFFLE  O(n)
//  Builds 52-card deck array, shuffles in place, pushes to Stack
// ============================================================
Card createCard (const char* rank, const char* suit) {
    Card c;
    strcpy(c.rank, rank);
    strcpy(c.suit, suit);
    if (strcmp(rank, "J") == 0 || strcmp(rank, "Q") == 0 || strcmp(rank, "K") == 0)
        c.value = 10;
    else if (strcmp(rank, "A") == 0)
        c.value = 11;   // Ace = 11 initially; ace resolution in calculateHandValue
    else
        c.value = atoi(rank);
    return c;
}

void initDeck (Stack* deck) {
    freeStack(deck);
    const char* suits[] = {"Hearts", "Diamonds", "Clubs", "Spades"};
    const char* ranks[] = {"2","3","4","5","6","7","8","9","10","J","Q","K","A"};

    Card cards[DECK_SIZE];
    int  idx = 0;
    for (int s = 0; s < 4; s++)
        for (int r = 0; r < 13; r++)
            cards[idx++] = createCard(ranks[r], suits[s]);

    // Fisher-Yates Shuffle: O(n)
    for (int i = DECK_SIZE - 1; i > 0; i--) {
        int j    = rand() % (i + 1);
        Card tmp = cards[i];
        cards[i] = cards[j];
        cards[j] = tmp;
    }

    for (int i = 0; i < DECK_SIZE; i++)
        pushCard(deck, cards[i]);
}

Card dealCard (Stack* deck) { return popCard(deck); }

// ============================================================
//  UI HELPERS
// ============================================================
void printBanner () {
    cout << "\n";
    cout << " ██████╗ ██╗      █████╗  ██████╗██╗  ██╗     ██╗ █████╗  ██████╗██╗  ██╗\n";
    cout << " ██╔══██╗██║     ██╔══██╗██╔════╝██║ ██╔╝     ██║██╔══██╗██╔════╝██║ ██╔╝\n";
    cout << " ██████╔╝██║     ███████║██║     █████╔╝      ██║███████║██║     █████╔╝ \n";
    cout << " ██╔══██╗██║     ██╔══██║██║     ██╔═██╗ ██   ██║██╔══██║██║     ██╔═██╗ \n";
    cout << " ██████╔╝███████╗██║  ██║╚██████╗██║  ██╗╚█████╔╝██║  ██║╚██████╗██║  ██╗\n";
    cout << " ╚═════╝ ╚══════╝╚═╝  ╚═╝ ╚═════╝╚═╝  ╚═╝ ╚════╝ ╚═╝  ╚═╝ ╚═════╝╚═╝  ╚═╝\n";
    cout << "                      🃏  Casino Edition  🃏\n";
    cout << "                 CPE112 Final Project — 2/2025\n\n";
}

void printLine () { cout << "────────────────────────────────────────────\n"; }

int safeInputInt (int minVal, int maxVal) {
    int val;
    while (!(cin >> val) || val < minVal || val > maxVal) {
        cin.clear();
        cin.ignore(INT_MAX, '\n');
        cout << "  Invalid input. Enter a number (" << minVal << "-" << maxVal << "): ";
    }
    return val;
}

// ============================================================
//  GLOBAL GAME STATE
// ============================================================
Player   players[MAX_PLAYERS];
int      numPlayers = 0;
Player   dealer;
Stack    deck;
Queue    turnQueue;
BetStack betHistory;
HashTable leaderboard;

// ============================================================
//  SETUP
// ============================================================
void setupPlayers () {
    cout << "\nHow many players? (1-" << MAX_PLAYERS << "): ";
    numPlayers = safeInputInt(1, MAX_PLAYERS);

    for (int i = 0; i < numPlayers; i++) {
        cout << "Enter name for Player " << i + 1 << ": ";
        cin >> players[i].name;
        players[i].chips          = START_CHIPS;
        players[i].bet            = 0;
        players[i].isActive       = true;
        players[i].isBust         = false;
        players[i].isBlackjackWin = false;
        initHand(&players[i].hand);
        insertStats(&leaderboard, players[i].name, START_CHIPS);
    }

    strcpy(dealer.name, "Dealer");
    dealer.chips = 0;
    dealer.bet   = 0;
    initHand(&dealer.hand);
}

// ============================================================
//  BETTING PHASE
// ============================================================
void placeBets () {
    cout << "\n--- PLACE BETS ---\n";
    freeBetStack(&betHistory);   // clear undo history each round

    for (int i = 0; i < numPlayers; i++) {
        if (!players[i].isActive) continue;
        cout << "  " << players[i].name << " (Chips: " << players[i].chips << ") — Bet amount: ";
        int bet = safeInputInt(1, players[i].chips);
        players[i].bet    = bet;
        players[i].chips -= bet;
        pushBet(&betHistory, i, bet);
        cout << "  ✔ " << players[i].name << " bets " << bet << " chips.\n";
    }
}

void undoLastBet () {
    if (isBetEmpty(&betHistory)) { cout << "  No bets to undo!\n"; return; }
    BetRecord r = popBet(&betHistory);
    if (r.playerIndex < 0) return;
    players[r.playerIndex].chips += r.betAmount;
    players[r.playerIndex].bet    = 0;
    cout << "  ↩ Undo: " << players[r.playerIndex].name
         << "'s bet of " << r.betAmount << " chips returned.\n";
}

// ============================================================
//  DEAL INITIAL CARDS
// ============================================================
void dealInitialCards () {
    // Two rounds: each player and dealer get one card per round
    for (int round = 0; round < 2; round++) {
        for (int i = 0; i < numPlayers; i++)
            if (players[i].isActive)
                addCardToHand(&players[i].hand, dealCard(&deck));
        addCardToHand(&dealer.hand, dealCard(&deck));
    }
}

// ============================================================
//  PLAYER TURN  (uses dequeued player index)
// ============================================================
void playerTurn (int idx) {
    printLine();
    cout << "\n>>> " << players[idx].name << "'s Turn <<<\n";
    displayHand(&dealer.hand,      "Dealer",                true);  // hide first card
    displayHand(&players[idx].hand, players[idx].name);

    // Instant blackjack win
    if (isBlackjack(&players[idx].hand)) {
        cout << "  🎉 BLACKJACK! " << players[idx].name << " wins 3:2!\n";
        players[idx].chips          += (int)(players[idx].bet * 2.5);
        players[idx].bet             = 0;
        players[idx].isBlackjackWin  = true;
        updateStats(&leaderboard, players[idx].name, 1, players[idx].chips);
        return;
    }

    bool turnDone = false;
    while (!turnDone) {
        cout << "\n  [1] Hit   [2] Stand   [3] Double Down\n  Choice: ";
        int choice = safeInputInt(1, 3);

        if (choice == 1) {
            // HIT
            addCardToHand(&players[idx].hand, dealCard(&deck));
            displayHand(&players[idx].hand, players[idx].name);
            if (isBust(&players[idx].hand)) {
                cout << "  💥 BUST! " << players[idx].name << " loses.\n";
                players[idx].isBust = true;
                updateStats(&leaderboard, players[idx].name, -1, players[idx].chips);
                turnDone = true;
            } else if (calculateHandValue(&players[idx].hand) == 21) {
                cout << "  🎯 21! Perfect hand!\n";
                turnDone = true;
            }
        } else if (choice == 2) {
            // STAND
            cout << "  " << players[idx].name << " stands at "
                 << calculateHandValue(&players[idx].hand) << ".\n";
            turnDone = true;
        } else if (choice == 3) {
            // DOUBLE DOWN — double bet, receive exactly one more card, then stand
            int extra            = (players[idx].chips >= players[idx].bet)
                                   ? players[idx].bet
                                   : players[idx].chips;
            players[idx].chips  -= extra;
            players[idx].bet    += extra;
            cout << "  ⬆ Double Down! New bet: " << players[idx].bet << "\n";
            addCardToHand(&players[idx].hand, dealCard(&deck));
            displayHand(&players[idx].hand, players[idx].name);
            if (isBust(&players[idx].hand)) {
                cout << "  💥 BUST! " << players[idx].name << " loses.\n";
                players[idx].isBust = true;
                updateStats(&leaderboard, players[idx].name, -1, players[idx].chips);
            }
            turnDone = true;   // can only get one card on double down
        }
    }
}

// ============================================================
//  DEALER TURN  (dealer hits until >= 17)
// ============================================================
void dealerTurn () {
    printLine();
    cout << "\n>>> Dealer's Turn <<<\n";
    displayHand(&dealer.hand, "Dealer");

    while (calculateHandValue(&dealer.hand) < 17) {
        cout << "  Dealer hits...\n";
        addCardToHand(&dealer.hand, dealCard(&deck));
        displayHand(&dealer.hand, "Dealer");
    }

    if (isBust(&dealer.hand))
        cout << "  💥 Dealer BUSTS!\n";
    else
        cout << "  Dealer stands at " << calculateHandValue(&dealer.hand) << ".\n";
}

// ============================================================
//  RESOLVE ROUND  — compare hands and pay out
// ============================================================
void resolveRound () {
    printLine();
    cout << "\n=== ROUND RESULTS ===\n";
    int  dealerVal  = calculateHandValue(&dealer.hand);
    bool dealerBust = isBust(&dealer.hand);

    for (int i = 0; i < numPlayers; i++) {
        if (!players[i].isActive)       continue;
        if (players[i].isBlackjackWin)  continue;  // already paid 3:2
        if (players[i].isBust)          continue;  // already lost

        int playerVal = calculateHandValue(&players[i].hand);
        cout << "  " << players[i].name
             << " (" << playerVal << ")  vs  Dealer (" << dealerVal << "): ";

        if (dealerBust || playerVal > dealerVal) {
            cout << "✅ WIN  +" << players[i].bet << " chips\n";
            players[i].chips += players[i].bet * 2;
            updateStats(&leaderboard, players[i].name, 1, players[i].chips);
        } else if (playerVal == dealerVal) {
            cout << "🤝 DRAW — bet returned\n";
            players[i].chips += players[i].bet;
            updateStats(&leaderboard, players[i].name, 0, players[i].chips);
        } else {
            cout << "❌ LOSE  -" << players[i].bet << " chips\n";
            updateStats(&leaderboard, players[i].name, -1, players[i].chips);
        }

        // Sync chip count in hash table
        PlayerStats* ps = findStats(&leaderboard, players[i].name);
        if (ps) ps->totalChips = players[i].chips;
    }
}

// ============================================================
//  RESET FOR NEXT ROUND
// ============================================================
void resetRound () {
    for (int i = 0; i < numPlayers; i++) {
        clearHand(&players[i].hand);
        players[i].bet            = 0;
        players[i].isBust         = false;
        players[i].isBlackjackWin = false;
        if (players[i].chips <= 0) {
            cout << "  " << players[i].name << " is out of chips — ELIMINATED.\n";
            players[i].isActive = false;
        }
    }
    clearHand(&dealer.hand);
    initDeck(&deck);   // reshuffle for every round
}

// ============================================================
//  PLAY ONE FULL ROUND
// ============================================================
void playRound () {
    resetRound();

    // ── Betting Phase ─────────────────────────────────────
    cout << "\n━━━━━━━━━━━━━ BETTING PHASE ━━━━━━━━━━━━━\n";
    placeBets();

    char undo;
    cout << "\n  Undo a bet? (y/n): "; cin >> undo;
    if (undo == 'y' || undo == 'Y') undoLastBet();

    // ── Initial Deal ──────────────────────────────────────
    dealInitialCards();

    // ── Player Turns via Queue ────────────────────────────
    cout << "\n━━━━━━━━━━━━━ PLAYER TURNS ━━━━━━━━━━━━━\n";
    clearQueue(&turnQueue);
    for (int i = 0; i < numPlayers; i++)
        if (players[i].isActive)
            enqueue(&turnQueue, i);

    while (!isQueueEmpty(&turnQueue)) {
        int idx = dequeue(&turnQueue);
        if (players[idx].isActive && !players[idx].isBust)
            playerTurn(idx);
    }

    // ── Dealer Turn ───────────────────────────────────────
    cout << "\n━━━━━━━━━━━━━ DEALER TURN ━━━━━━━━━━━━━━\n";
    dealerTurn();

    // ── Results ───────────────────────────────────────────
    resolveRound();
}

// ============================================================
//  SEARCH PLAYER (demonstrates search functionality)
// ============================================================
void searchPlayer () {
    char name[50];
    cout << "\nEnter player name to search: "; cin >> name;
    PlayerStats* ps = findStats(&leaderboard, name);
    if (!ps) { cout << "  Player not found.\n"; return; }
    cout << "\n  ── Stats for " << ps->name << " ──\n";
    cout << "  Chips:  " << ps->totalChips << "\n";
    cout << "  Wins:   " << ps->wins       << "\n";
    cout << "  Losses: " << ps->losses     << "\n";
    cout << "  Draws:  " << ps->draws      << "\n";
}

// ============================================================
//  SHOW ALL PLAYERS' CURRENT CHIP STATUS
// ============================================================
void showChipStatus () {
    printLine();
    cout << "  PLAYER CHIP STATUS\n";
    printLine();
    for (int i = 0; i < numPlayers; i++) {
        printf("  %-15s %4d chips%s\n",
            players[i].name,
            players[i].chips,
            players[i].isActive ? "" : "  [ELIMINATED]");
    }
}

// ============================================================
//  MAIN MENU
// ============================================================
void mainMenu () {
    while (true) {
        // Check if any player still active
        int active = 0;
        for (int i = 0; i < numPlayers; i++)
            if (players[i].isActive) active++;
        if (active == 0) {
            cout << "\n  All players eliminated. Game over!\n";
            break;
        }

        cout << "\n╔═══════════════════════════╗\n";
        cout <<   "║        MAIN MENU          ║\n";
        cout <<   "╠═══════════════════════════╣\n";
        cout <<   "║  [1] Play Round           ║\n";
        cout <<   "║  [2] View Leaderboard     ║\n";
        cout <<   "║  [3] Chip Status          ║\n";
        cout <<   "║  [4] Search Player        ║\n";
        cout <<   "║  [5] Quit                 ║\n";
        cout <<   "╚═══════════════════════════╝\n";
        cout <<   "  Choice: ";
        int choice = safeInputInt(1, 5);

        switch (choice) {
            case 1: playRound();        break;
            case 2: displayLeaderboard(&leaderboard); break;
            case 3: showChipStatus();   break;
            case 4: searchPlayer();     break;
            case 5:
                cout << "\n  Thanks for playing! Goodbye 🃏\n\n";
                return;
        }
    }
}

// ============================================================
//  MAIN
// ============================================================
int main () {
    srand((unsigned int)time(NULL));

    initStack(&deck);
    initQueue(&turnQueue);
    initBetStack(&betHistory);
    initHashTable(&leaderboard);

    printBanner();
    setupPlayers();
    initDeck(&deck);

    mainMenu();

    // Cleanup dynamic memory
    freeStack(&deck);
    clearQueue(&turnQueue);
    freeBetStack(&betHistory);

    return 0;
}
