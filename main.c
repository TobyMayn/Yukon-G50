//#include <iostream>
#include <string.h>
//#include <cstring>
#include <stdio.h>
#include <malloc.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <limits.h>


typedef struct card Card;
typedef struct moves Moves;

// Array of the different suits, Clubs, Diamonds, Hearts and Spades
const char suits[] = {'C', 'D', 'H', 'S'};
char ranks[] = {'A', '2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K'};

//Array consisting of the initial number of cards turned face down;
int facedown[7];

char *command_approver = "OK";
bool play = false;
// Foundations
Card *foundations[4];
//Columns
Card *columns[7];
//header pointing at moves struct, used in undo command
Moves *latest;

struct card {
    Card *prev;
    Card *next;
    char rank;
    char suit;
};

Card *new_card(char rank, char suit){
    Card *card = (Card *)malloc(sizeof(Card));
    card->prev = NULL;
    card->next = NULL;
    card->rank = rank;
    card->suit = suit;
    return card;
}

struct moves {
    Moves *prev;
    char *command;
};

//moves will be implemented as a stack, hence why there is only a pointer to a prev node.
Moves *new_move(char *command){
    Moves *moves = (Moves *)malloc(sizeof(Moves));
    moves->prev = NULL;
    moves->command = command;
    return moves;
}

void update_undo_header(char *command){
    Moves *prev = latest;
    latest = new_move(command);
    latest->prev = prev;
}

void initialize_undo(){
    bool init = false;
    if(!init) {
        latest = new_move("UNDO");
        init = true;
    }
}

Card *default_deck(){
    //Initialize deck, with a dummy bottom card
    Card *head = new_card('B', 'B');
    Card *current = head;
    for (int suit = 0; suit < 4; suit++) {
        for (int rank = 0; rank < 13; rank++) {
            Card *newCard = new_card(ranks[rank], suits[suit]);
            current->next = newCard;
            head->prev = newCard;
            newCard->prev = current;
            newCard->next = head;
            current = newCard;
        }
    }
    return head;
}

//Still needs work - currently only takes input file and prints content to terminal
Card *load_deck(char* filename){
    FILE* ptr;
    char ch;
    char check_cards[52][2];
    int n = 0;

    ptr = fopen(filename, "r");

    if (NULL == ptr) {
        printf("File doesn't exist\n");
    }

    // Add dummy card to bottom
    Card *head = new_card('B', 'B');
    Card *current = head;

    char input[3]; // char array to hold line characters for input

    // Do-while that reads an entire line, creates a new card and adds it to the deck
    do {

        for (int i = 0; i < 3; ++i) {
            ch = fgetc(ptr);
            input[i] = ch;
        }
        Card *newCard = new_card(input[0], input[1]);
        current->next = newCard;
        head->prev = newCard;
        newCard->prev = current;
        newCard->next = head;
        current = newCard;
        snprintf(check_cards[n],4,"%c%c",newCard->rank, newCard->suit);
        for (int i = 0; i < n; ++i) {
            if(check_cards[i][0] == newCard->rank && check_cards[i][1] == newCard->suit){
                printf("Card: %c%c is a dublicate card\nLine: %i of %s", check_cards[i][0], check_cards[i][1], n+1, filename);
                free(newCard);
                return 0;
            }
        }
        n++;

        // Checking if character is not EOF.
        // If it is EOF stop reading.
    } while (ch != EOF);

    // Closing the file
    fclose(ptr);
    return head;
}
void *save_cards(Card *deck, char* filename){
    FILE* ptr;

    ptr = fopen(filename, "w");
    do {
        deck = deck->next;
        fprintf(ptr,"%c%c\n", deck->rank, deck->suit);
    }while(deck->next != NULL);


    // Closing the file
    fclose(ptr);
}

void update_facedown(int index){
    if(facedown[index] != 0)
        facedown[index]--;
}

void error_message(){
    command_approver = "Invalid action..!";
}

int find_string_length(const char *string){
    int size = 0;
    for (int i = 0; i < strlen(string); ++i) {
        if(string[i] != ' ') {
            size++;
        }
    }
    return size;
}

//method to find pile ie foundation or column, to start searching for a card.
//If the given pile doesn't exist the method will return a null pointer.
Card *get_pile(char rank,char suit){
    Card *temp = NULL;
    for (int i = 0; i < ((sizeof(columns) / sizeof(columns[0])) + sizeof(foundations) / sizeof(foundations[0])) ; ++i) {
        if (i >= 7){
            if(foundations[i % 7]->rank == rank && foundations[i % 7]->suit == suit)
                temp = foundations[i % 7];
        }
        else if(columns[i % 7]->rank == rank && columns[i % 7]->suit == suit){
            temp = columns[i];
            break;
        }
    }
    return temp;
}

Card *find_card(char rank,char suit,Card *ptr){
    Card *pile = ptr;
    while(ptr->next != pile || (ptr->rank != rank && ptr->suit != suit)){
        ptr = ptr->next;
        if(ptr->rank == rank && ptr->suit == suit)
            return ptr;
    }
    if(ptr->rank != rank && ptr->suit != suit || (ptr->prev == pile && ptr->next == pile)){ //Checks if the specified card, is the dummy card + checking if the card exists
        error_message();
        return NULL;
    }
    return ptr;
}

void move_bunch(Card *from, Card *frompile, Card *to, Card *topile){
    Card *temp = from;

    while(temp->next != frompile){
        temp = temp->next;
    }

    from->prev->next = frompile;
    frompile->prev = from->prev;

    topile->prev = temp;
    temp->next = topile;
    from->prev = to;
    to->next = from;
}

void move_card(Card *from,Card *to){
    Card *temp = from;
    temp->prev->next = temp->next;
    temp->next->prev = temp->prev;

    temp->next = to->next;
    to->next = temp;
    temp->prev = to;
}

// Helper method for interleave_shuffle
Card *split_deck(Card *deck, int amount){
    Card *temp_deck = deck;

    //First actual card in deck
    temp_deck = temp_deck->next;
    // Loop to find spot to split
    for (int i = 0; i < amount; ++i) {
        temp_deck = temp_deck->next;
    }

    temp_deck->prev->next = NULL;
    temp_deck->prev = NULL;

    Card *part = temp_deck; // Variable for holding split part of deck
    return part;
}


Card *interleave_shuffle(Card *head, int amount){
    Card *new_deck_head = new_card('B', 'B'); // Variable for holding interleaved deck
    Card *new_deck = new_deck_head;
    Card *part;
    //If amount == 0, just return original deck
    if (amount > 52) {
        printf("Amount too large, pick lower amount!!");
        return new_deck;
    }
    if (amount == 0 || amount == 52){
        return head;
    }

    part = split_deck(head, amount);

    Card *deck_next = head->next;
    deck_next->prev = NULL;
    // Deck now has first actual card

    while(deck_next != NULL && deck_next->rank != 'B'){
        // Add card from first pile to new_deck
        new_deck->next = new_card(deck_next->rank, deck_next->suit);
        new_deck->next->prev = new_deck;
        new_deck = new_deck->next;

        // Update first pile pointer
        deck_next = deck_next->next;

        //Check if card is the last card in second pile
        if(part->next == NULL || part->next->rank == 'B'){
            //Add last card in part
            new_deck->next = new_card(part->rank, part->suit);
            new_deck->next->prev = new_deck;
            new_deck = new_deck->next;

            //Add the rest of the other cards in the first pile
            new_deck->next = deck_next;
            new_deck->next->prev = new_deck;

            // Update pointers for head and tail
            Card *placeholder = new_deck_head;
            for (int i = 0; i < 52; ++i) {
                placeholder = placeholder->next;
            }
            new_deck_head->prev = placeholder;
            placeholder->next = new_deck_head;
            return new_deck_head;
        }


        if(part->next != NULL && part->next->rank != 'B' ){
            //Add card from second pile to new_deck
            new_deck->next = new_card(part->rank, part->suit);
            new_deck->next->prev = new_deck;
            new_deck = new_deck->next;

            // update second pile pointer
            part = part->next;
        }
    }
    new_deck->next = part;
    new_deck->next->prev = new_deck;

    // Update pointers for head and tail
    Card *placeholder = new_deck_head;
    for (int i = 0; i < 52; ++i) {
        placeholder = placeholder->next;
    }
    new_deck_head->prev = placeholder;
    placeholder->next = new_deck_head;

    return new_deck_head;
}

Card *random_shuffle(Card *deck){
    int n = 2;
    if(!deck->next){
        return deck;
    }
    srand(time(NULL)); // Seeds rand() once, generating better random numbers.

    Card *shuffled_deck = deck->next; // Add first card from deck to shuffled_deck
    Card *current_card = deck->next->next; // Add second card from deck to current card
    Card *next_card = deck->next->next->next; // Add third card from deck to next_card
    shuffled_deck->next = NULL;
    shuffled_deck->prev = NULL;

    while (current_card->rank != 'B'){
        Card *next_next_card = next_card->next;
        int random = rand() % n;
        if (random == 0){
            shuffled_deck->prev = current_card;
            current_card->next = shuffled_deck;
            current_card->prev = NULL;
            shuffled_deck = current_card;
        }
        else {
            Card *placeholder = shuffled_deck;
            for (int i = 1; i < random; ++i) {
                placeholder = placeholder->next;
            }
            if (placeholder->next != NULL) {
                placeholder->next->prev = current_card;
            }
            current_card->next = placeholder->next;
            placeholder->next = current_card;
            current_card->prev = placeholder;

        }
        current_card = next_card;
        next_card = next_next_card; // Update next_card
        n++;
    }
    //Add dummy card in start of shuffled deck
    shuffled_deck->prev = deck;
    deck->next = shuffled_deck;
    deck->prev = NULL;
    shuffled_deck = deck;

    // Update pointers for head and tail
    Card *placeholder = shuffled_deck;
    for (int i = 0; i < 52; ++i) {
        placeholder = placeholder->next;
    }
    shuffled_deck->prev = placeholder;
    placeholder->next = shuffled_deck;

    return shuffled_deck;
}

void undo(){
    if(latest->command == "UNDO"){
        error_message();
        return;
    }//checks for dummy card

    char *command = latest->command;
    Card *topile = get_pile(command[0],command[1]);

    int strlen = find_string_length(command);

    if(strlen == 9) {
        Card *frompile = get_pile(command[7], command[8]);
        Card *fromcard = find_card(command[3],command[4],frompile);
        Card *tocard = topile;
        while(tocard->next != topile) {
            tocard = tocard->next;
        }
        move_bunch(fromcard,frompile,tocard,topile);
    }
    else{
        Card *frompile = get_pile(command[4],command[5]);
        Card *from = frompile;
        Card *to = topile;
        while(from->next != frompile){
            from = from->next;
        }
        while(to->next != topile){
            to = to->next;
        }
        move_card(from,to);
    }
    //making the pointer point to the latest move command.
    Moves *temp = latest;
    latest = temp->prev;
    free(temp); //deleting the node from memory.
}

//the foundations will not have a predefined suit. The suit of the foundation will be defined by the first card moved to it.
bool move_to_foundation(Card *card,Card *topilepos, Card *toPile) {
    bool possMove = false;
    if(card->next->rank != 'C')//checks if we're trying to move more than one card.
        return possMove;

    if (toPile->next == toPile){//Checks if the foundation is empty.
        if(card->rank == ranks[0])//if the foundation is empty check if the card is an Ace.
            possMove = true;
    }else{
        int i = 0;
        while(ranks[i] != card->rank){ //Find the index for the cards rank.
            i++;
        }

        if(ranks[i-1] == topilepos->rank){//checks if the topcard is one rank less, than the card we with to move.
            if(card->suit == topilepos->suit)//checks for same suit.
                possMove = true;
        }
    }
    return possMove;
}

bool valid_move(Card *cardToMove, Card *topilepos, Card *topile){
    bool valid = false;
    int i = 0;

    if(topile->rank == foundations[0]->rank){
        return move_to_foundation(cardToMove,topilepos,topile);
    }

    while(ranks[i] != cardToMove->rank){
        i++;
    }
    if(i < 12 && ranks[i+1] == topilepos->rank){
        if(cardToMove->suit != topilepos->suit)
            valid = true;
    }
    if(i == 12){
        //checks if pile is empty for king move
        if(topile->next == topile)
            valid = true;
    }
    return valid;
}

void move_specific(char *command,Card *cardtomove) {
    Card *frompile = cardtomove;
    //checks if card exists in pile / list
    cardtomove = find_card(command[3], command[4], cardtomove);
    if(cardtomove == NULL){
        error_message();
        return;
    }

    //finds the column or foundation specified in the command.
    Card *to = get_pile(command[7],command[8]);
    if(to == NULL){
        error_message();
        return;
    }

    Card *tonode = to;
    while(tonode->next != to){
        tonode = tonode->next;
    }
    //cardtomove this point we have found the card cardtomove a pile, and the pile it's supposed to go to.
    //now we check if the move is valid.
    if(valid_move(cardtomove, tonode, to)){
        move_bunch(cardtomove,frompile,tonode,to);
        update_undo_header(command);
    }
    else
        error_message();
}


void pile_to_pile(char *command,Card *pointer) {
    Card *from = pointer;
    Card *to = get_pile(command[4],command[5]);
    Card *pile = to;
    if (to == NULL){
        error_message();
        return;
    }

    while(from->next != pointer){
        from = from->next;
    }
    while(to->next != pile){
        to = to->next;
    }

    Card *temp;
    if(valid_move(from,to,pile)){
        move_card(from,to);
        update_undo_header(command);
    }
    else
        error_message();
}

void move(const char *command) {
    Card *pile = get_pile(command[0], command[1]);//gets pointer of list to move from
    if (pile == NULL) {
        error_message();//if the get_pile method returns a null pointer we return to caller with error message
        return;
    }
    if(pile->next == pile){//checks if pile is empty
        error_message();
        return;
    }

    int strlen = find_string_length(command);

    switch (strlen) {
        case 6 :
            pile_to_pile(command, pile);
            break;
        case 9:
            move_specific(command, pile);
            break;
        default:
            error_message();
            return;
    }
}

//Showmethod only works on unshuffled deck
void show(Card *deck){
    Card *temp = deck;
    printf("\n\tC1\tC2\tC3\tC4\tC5\tC6\tC7\n");
    int i = 0;
    int j = 1;
    int f;
    char s[2];
    while(temp->next->rank != 'B'){
        if(i % 7 == 0) {
            if(j % 2 == 0 && j != 1) {
                f = j / 2;
                s[0] = 'F';
                s[1] = f + '0';
                printf("\t[]\t%c%c",s[0],s[1]);
            }
            printf("\n\t");
            j++;
        }
        temp = temp->next;
        printf("%c%c\t",temp->rank, temp->suit);

        i++;
    }
    printf("\n\n");
}

int find_longest_list(){
    int max_length = 0;
    //temp pointer to iterate through lists
    Card *temp;
    //finding the longest linked list.
    for (int i = 0; i < sizeof(columns) / sizeof(columns[0]); ++i) {
        int j = 0;
        temp = columns[i];
        while(temp->next != columns[i]){
            temp = temp->next;
            j++;
        }
        if(j > max_length)
            max_length = j;
    }
    return max_length;
}

//method for printing the contents of each list
void print_gamestate(){
    Card *placeholder[7];

    //copying head pointers to foundations linked lists over to another array.
    for (int i = 0; i < sizeof(placeholder) / sizeof(placeholder[0]); ++i) {
        placeholder[i] = columns[i];
    }

    int max_length = find_longest_list(); //finding the length of the longest list, to know how much to print to the console
    if(max_length < 8) //if the length is less than 8, it is set to 8, so the foundations will be printed.
        max_length = 8;

    printf("\n");
    for (int i = 0; i < sizeof(placeholder) / sizeof(placeholder[0]); ++i) {//Prints the dummy cards for all columns
        printf("\t%c%c",placeholder[i]->rank,placeholder[i]->suit);
    }
    printf("\n");
    int f = 1;
    Card* foundation_temp;
    //printing one card at a time from each column. If there is no card, an empty space will be printed.
    for (int i = 0; i < max_length; ++i) {
        printf("\n\t");
        for (int j = 0; j < sizeof(placeholder) / sizeof(placeholder[0]); ++j) {
            //Checks that next card is not the last card
            if (placeholder[j]->next != columns[j]) {
                placeholder[j] = placeholder[j]->next;
                //checks that the facedown cards has been turned or not
                if(facedown[j] >= i + 1){
                    //checks whether a card is supposed to be turned
                    if(facedown[j] == i + 1 && placeholder[j]->next == columns[j]) {
                        //Decrements the amount of facedown cards for the given column.
                        update_facedown(j);
                        printf("%c%c\t", placeholder[j]->rank, placeholder[j]->suit);
                    }
                    else
                        //prints facedown card
                        printf("[]\t");
                }
                else
                    //if we are past the facedown cards just print the card if there is a nextcard
                    printf("%c%c\t", placeholder[j]->rank, placeholder[j]->suit);
            }
            else
                printf("  \t");
        }
        //printing foundation cards for evert second row.
        f++;
        if (f % 2 == 0 && ((f/2) - 1) < (sizeof(foundations) / sizeof(foundations[0]))){
            foundation_temp = foundations[(f / 2) - 1];
            while (foundation_temp->next != foundations[(f / 2) - 1]) {
                foundation_temp = foundation_temp->next;
            }
            if(foundations[(f / 2) - 1]->next == foundations[(f / 2) - 1]){
                printf("[]\tF%c",(f / 2) + '0');
            }else{
                printf("%c%c\t%c%c",foundation_temp->rank,foundation_temp->suit,foundations[(f / 2) - 1]->rank,foundations[(f / 2) - 1]->suit);
            }
        }
    }
}

//Distributes the cards on to the starting piles.
void distribute_cards(Card* play_deck){
    bool distributed = false;
    Card *temp;
    Card *column_temp;

    if(!distributed){
        int pile_lengths[7] = {1,6,7,8,9,10,11}; //array determines how many cards should be placed in a column
        for (int i = 0; play_deck->next->rank != 'B'; ++i) {
            for (int j = 0; j < sizeof(pile_lengths) / sizeof(pile_lengths[0]); ++j) {
                //if the right amount of cards have already been placed in a column, the following will iterate to the next column
                if(pile_lengths[j] > i && play_deck->next->rank != 'B') {
                    play_deck = play_deck->next;
                    temp = play_deck;
                    column_temp = columns[j];
                    while(column_temp->next->rank != 'C'){
                        column_temp = column_temp->next;
                    }
                    //erasing node from play_deck
                    play_deck = temp->prev;
                    temp->prev->next = temp->next;
                    temp->next->prev = temp->prev;

                    column_temp->next = temp;
                    temp->prev = column_temp;
                    temp->next = columns[j];
                }
            }
        }
        distributed = true;
    }

}

void setup_columns_foundations(){
    for (int i = 0; i < 4; ++i) {
        foundations[i] = new_card('F', i + 1 +'0'); //dummy card
        foundations[i]->prev = foundations[i];
        foundations[i]->next = foundations[i];
    }
    for (int i = 0; i < 7; ++i) {
        columns[i] = new_card('C', i + 1 + '0'); //dummy card
        columns[i]->prev = columns[i];
        columns[i]->next = columns[i];
        facedown[i] = i;
    }
}

void startup_phase() {
    char input[10];
    char *lastcommand = input;
    char filename[155];
    Card *initial_deck = NULL;
    Card *shuffled_deck = NULL;
    while (!play) {
        printf("\n");
        printf("\nLast command: %s\nMessage: %s\n", lastcommand,command_approver);
        command_approver = "OK";
        printf("Enter command: ");
        scanf("%s", &input);

//"C:\\Users\\emil1\\OneDrive\\Documents\\GitHub\\Yukon-G50\\Test_input.txt"
        if(strcmp(input, "ld") == 0){
            printf("Enter filename of file to be loaded,\n"
                   " or enter 'default' to use default deck of cards: ");
            scanf("%s", &filename);

            if (strcmp(filename,"default") == 0) {
                char *strarr = "C:\\Users\\emil1\\OneDrive\\Documents\\GitHub\\Yukon-G50\\Test_input.txt";
                initial_deck = load_deck(strarr);
            }else {
                initial_deck = load_deck(filename);
            }
        }else if(strcmp(input, "sw") == 0){
            if (initial_deck == NULL){
                error_message();
            }
            else{
                show(initial_deck);
            }

        }else if(strcmp(input, "sl") == 0){
            int amount = NULL;
            printf("Enter amount of cards to split, if no amount is entered it is chosen at random: ");
            scanf("%d", &amount);

            if (amount == '\n') {
                amount = rand() % 52;
                shuffled_deck = interleave_shuffle(initial_deck, amount);
            }else{
                shuffled_deck = interleave_shuffle(initial_deck, amount);
            }

        }else if(strcmp(input, "sr") == 0){
            shuffled_deck = random_shuffle(initial_deck);

        }else if(strcmp(input, "sd") == 0){
            char *save_filename;
            printf("Enter filename of file to be saved,\n"
                   " or enter 'default' to use default filename cards.txt: ");
            scanf("%s", &save_filename);

            if (strcmp(save_filename,"default") == 0) {
                char *savefiletemp = "C:\\Users\\emil1\\OneDrive\\Documents\\GitHub\\Yukon-G50\\cards.txt";
                save_cards(shuffled_deck, savefiletemp);
            }else{
                save_cards(shuffled_deck, save_filename);
            }

        }else if(strcmp(input, "qq") == 0){
            exit(0);

        }else if(input[0] == 'p'){
            setup_columns_foundations();
            distribute_cards(shuffled_deck);
            play = true;

        }else
            error_message();

        lastcommand = input;
    }
}

bool game_won(){
    bool gamewon = true;

    for (int i = 0; i < sizeof(foundations) / sizeof(foundations[0]); ++i) {
        if(foundations[i]->prev->rank != ranks[12])
            gamewon = false;
    }
    return gamewon;
}

void play_phase(){
    char input[10];
    char *lastcommand = input;
    initialize_undo();
    while(play){
        printf("\n");
        print_gamestate();
        printf("\nLast command: %s\nMessage: %s\n", lastcommand,command_approver);
        command_approver = "OK";
        printf("Enter command: ");
        scanf("%s", &input);

        if(input[0] == 'u')
            undo();
        else if(input[0] == 'q')
            play = false;
        else{
            move(input);
        }

        lastcommand = input;
    }
}

int main() {
    bool won = false;
    while(!won) {
        startup_phase();
        play_phase();
        if(game_won()){
            won = true;
            printf("Congrats, you've won the game!");
        }
    }
    return 0;
}