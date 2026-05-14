#include <iostream>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <climits>
using namespace std;

#define DECK_SIZE    52
#define MAX_PLAYERS   4
#define HASH_SIZE   101
#define START_CHIPS 1000

struct Card {
    char suit[10];
    char rank[5];
    int  value;
};

struct StackNode {
    Card       card;
    StackNode* next;
};

struct Stack {
    StackNode* top;
    int        size;
};

void initStack    (Stack* s)         { s->top = NULL; s->size = 0; }
bool isStackEmpty (Stack* s)         { return s->top == NULL; }

void pushCard     (Stack* s, Card c) {
    StackNode* node = new StackNode();
    node->card = c;
    node->next = s->top;
    s->top     = node;
    s->size++;
}

Card popCard      (Stack* s) {
    if (isStackEmpty(s)) { Card empty = {"", "", 0}; return empty; }
    StackNode* temp = s->top;
    Card c          = temp->card;
    s->top          = s->top->next;
    delete temp;
    s->size--;
    return c;
}

void freeStack    (Stack* s) {
    while (!isStackEmpty(s)) popCard(s);
}

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

void      initBetStack (BetStack* bs)                   { bs->top = NULL; }
bool      isBetEmpty   (BetStack* bs)                   { return bs->top == NULL; }

void      pushBet      (BetStack* bs, int idx, int amt) {
    BetNode* node = new BetNode();
    node->record  = {idx, amt};
    node->next    = bs->top;
    bs->top       = node;
}

BetRecord popBet       (BetStack* bs) {
    BetRecord empty = {-1, 0};
    if (isBetEmpty(bs)) return empty;
    BetNode*  temp = bs->top;
    BetRecord r    = temp->record;
    bs->top        = bs->top->next;
    delete temp;
    return r;
}

void      freeBetStack (BetStack* bs) {
    while (!isBetEmpty(bs)) popBet(bs);
}

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

int calculateHandValue (Hand* h) {
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

struct QueueNode {
    int        playerIndex;
    QueueNode* next;
};

struct Queue {
    QueueNode* front;
    QueueNode* rear;
    int        size;
};

void initQueue    (Queue* q) { q->front = NULL; q->rear = NULL; q->size = 0; }
bool isQueueEmpty (Queue* q) { return q->front == NULL; }

void enqueue      (Queue* q, int idx) {
    QueueNode* node   = new QueueNode();
    node->playerIndex = idx;
    node->next        = NULL;
    if (q->rear == NULL) { q->front = q->rear = node; }
    else { q->rear->next = node; q->rear = node; }
    q->size++;
}

int  dequeue      (Queue* q) {
    if (isQueueEmpty(q)) return -1;
    QueueNode* temp = q->front;
    int idx         = temp->playerIndex;
    q->front        = q->front->next;
    if (q->front == NULL) q->rear = NULL;
    delete temp;
    q->size--;
    return idx;
}

void clearQueue   (Queue* q) { while (!isQueueEmpty(q)) dequeue(q); }

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

int hashFn (const char* name) {
    int hash = 0;
    for (int i = 0; name[i] != '\0'; i++)
        hash = (hash * 31 + (unsigned char)name[i]) % HASH_SIZE;
    return hash;
}

void insertStats (HashTable* ht, const char* name, int chips) {
    int idx = hashFn(name);
    while (ht->table[idx].occupied && strcmp(ht->table[idx].name, name) != 0)
        idx = (idx + 1) % HASH_SIZE;
    if (!ht->table[idx].occupied) {
        ht->table[idx] = {"", 0, 0, 0, chips, true};
        strcpy(ht->table[idx].name, name);
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
    PlayerStats* ps = findStats(ht, name);
    if (!ps) return;
    if      (result ==  1) ps->wins++;
    else if (result == -1) ps->losses++;
    else                   ps->draws++;
    ps->totalChips = chips;
}

void displayLeaderboard (HashTable* ht) {
    PlayerStats sorted[HASH_SIZE];
    int count = 0;
    for (int i = 0; i < HASH_SIZE; i++)
        if (ht->table[i].occupied)
            sorted[count++] = ht->table[i];

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

struct Player {
    char name[50];
    int  chips;
    int  bet;
    Hand hand;
    bool isActive;
    bool isBust;
    bool isBlackjackWin;
};

Card createCard (const char* rank, const char* suit) {
    Card c;
    strcpy(c.rank, rank);
    strcpy(c.suit, suit);
    if (strcmp(rank, "J") == 0 || strcmp(rank, "Q") == 0 || strcmp(rank, "K") == 0)
        c.value = 10;
    else if (strcmp(rank, "A") == 0)
        c.value = 11;
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

Player    players[MAX_PLAYERS];
int       numPlayers = 0;
Player    dealer;
Stack     deck;
Queue     turnQueue;
BetStack  betHistory;
HashTable leaderboard;

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

void placeBets () {
    cout << "\n--- PLACE BETS ---\n";
    freeBetStack(&betHistory);

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

void dealInitialCards () {
    for (int round = 0; round < 2; round++) {
        for (int i = 0; i < numPlayers; i++)
            if (players[i].isActive)
                addCardToHand(&players[i].hand, dealCard(&deck));
        addCardToHand(&dealer.hand, dealCard(&deck));
    }
}

void playerTurn (int idx) {
    printLine();
    cout << "\n>>> " << players[idx].name << "'s Turn <<<\n";
    displayHand(&dealer.hand, "Dealer", true);
    displayHand(&players[idx].hand, players[idx].name);

    if (isBlackjack(&players[idx].hand)) {
        cout << "  🎉 BLACKJACK! " << players[idx].name << " wins 3:2!\n";
        players[idx].chips         += (int)(players[idx].bet * 2.5);
        players[idx].bet            = 0;
        players[idx].isBlackjackWin = true;
        updateStats(&leaderboard, players[idx].name, 1, players[idx].chips);
        return;
    }

    bool turnDone = false;
    while (!turnDone) {
        cout << "\n  [1] Hit   [2] Stand   [3] Double Down\n  Choice: ";
        int choice = safeInputInt(1, 3);

        if (choice == 1) {
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
            cout << "  " << players[idx].name << " stands at "
                 << calculateHandValue(&players[idx].hand) << ".\n";
            turnDone = true;
        } else if (choice == 3) {
            int extra           = (players[idx].chips >= players[idx].bet)
                                  ? players[idx].bet
                                  : players[idx].chips;
            players[idx].chips -= extra;
            players[idx].bet   += extra;
            cout << "  ⬆ Double Down! New bet: " << players[idx].bet << "\n";
            addCardToHand(&players[idx].hand, dealCard(&deck));
            displayHand(&players[idx].hand, players[idx].name);
            if (isBust(&players[idx].hand)) {
                cout << "  💥 BUST! " << players[idx].name << " loses.\n";
                players[idx].isBust = true;
                updateStats(&leaderboard, players[idx].name, -1, players[idx].chips);
            }
            turnDone = true;
        }
    }
}

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

void resolveRound () {
    printLine();
    cout << "\n=== ROUND RESULTS ===\n";
    int  dealerVal  = calculateHandValue(&dealer.hand);
    bool dealerBust = isBust(&dealer.hand);

    for (int i = 0; i < numPlayers; i++) {
        if (!players[i].isActive)      continue;
        if (players[i].isBlackjackWin) continue;
        if (players[i].isBust)         continue;

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
    }
}

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
    initDeck(&deck);
}

void playRound () {
    resetRound();

    cout << "\n━━━━━━━━━━━━━ BETTING PHASE ━━━━━━━━━━━━━\n";
    placeBets();

    char undo;
    cout << "\n  Undo a bet? (y/n): "; cin >> undo;
    if (undo == 'y' || undo == 'Y') undoLastBet();

    dealInitialCards();

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

    cout << "\n━━━━━━━━━━━━━ DEALER TURN ━━━━━━━━━━━━━━\n";
    dealerTurn();

    resolveRound();
}

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

void mainMenu () {
    while (true) {
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
            case 1: playRound();                      break;
            case 2: displayLeaderboard(&leaderboard); break;
            case 3: showChipStatus();                 break;
            case 4: searchPlayer();                   break;
            case 5:
                cout << "\n  Thanks for playing! Goodbye 🃏\n\n";
                return;
        }
    }
}

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

    freeStack(&deck);
    clearQueue(&turnQueue);
    freeBetStack(&betHistory);

    return 0;
}
