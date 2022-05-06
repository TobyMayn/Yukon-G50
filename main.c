#include <iostream>
#include <string.h>
#include <cstring>
#include <stdio.h>
#include <malloc.h>

typedef struct card Card;

// Array of the different suits, Clubs, Diamonds, Hearts and Spades
const char suits[] = {'C', 'D', 'H', 'S'};
char ranks[] = {'A', '2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K'};

// Foundations
Card *foundations[4];

//Columns
Card *columns[7];

//Head for initial linked list
Card *head = NULL;

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
Card *default_deck(){
    //Initialize deck, with a dummy bottom card
    Card *deck = new_card('B', 'B');
    Card *prev = deck;
    for (int suit = 0; suit < 4; suit++) {
        for (int rank = 0; rank < 13; rank++) {
            Card *card = new_card(ranks[rank], suits[suit]);
            prev->next = card;
            card->prev = prev;
            prev = card;
        }
    }
    return deck;
}

//Still needs work - currently only takes input file and prints content to terminal
Card *load_deck(char* filename){
    FILE* ptr;
    char ch;

    ptr = fopen(filename, "r");

    if (NULL == ptr) {
        printf("File doesn't exist");
    }

    // Add dummy card to bottom
    Card *deck = new_card('B', 'B');
    Card *prev = deck;
    head = deck;

    char input[3]; // char array to hold line characters for input

    // Do-while that reads an entire line, creates a new card and adds it to the deck
    do {
        for (int i = 0; i < 3; ++i) {
            ch = fgetc(ptr);
            input[i] = ch;
        }
        Card *card = new_card(input[0], input[1]);
        prev->next = card;
        card->prev = prev;
        prev = card;

        // Checking if character is not EOF.
        // If it is EOF stop reading.
    } while (ch != EOF);

    // Closing the file
    fclose(ptr);
    return deck;
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

void error_message(){
    printf("\nInvalid move..!\n");
}

// Helper method for interleave_shuffle
Card *split_deck(Card *deck, int amount){
    Card *temp_deck = deck;
    // Loop to find spot to split
    for (int i = 0; i <= amount; ++i) {
        temp_deck = temp_deck->next;
    }
    //Emil//Bliver prev og next ikke slettet for den samme node her? (115-117) Ville man ikke skulle gøre sådan her?
    // [120][100] - [90][70] - [100][130]

    Card *holder = temp_deck->prev;
    holder->next = NULL;
    // [120][NULL] - [90][70] - [100][130]
    temp_deck->prev = NULL;
    // [120][NULL] - [NULL][70] - [100][130]

    temp_deck->prev = NULL;
    Card *part = temp_deck->next; // Variable for holding split part of deck
    temp_deck->prev->next = NULL;
    return part;
}


Card *interleave_shuffle(Card *deck, int amount){
    Card *new_deck = deck; // Variable for holding interleaved deck
    Card *part = split_deck(deck, amount);

    // Add dummy card to new card deck
    new_deck->next = NULL; // Set next pointer to NULL
    deck = deck->next; // Deck now has first actual card
    //deck->prev = NULL;

    while(deck->next != NULL){
        if(part->next == NULL){
            new_deck->next = deck;
            deck->prev = new_deck;
            return new_deck;
        }

        // Add card from first pile to new_deck
        new_deck->next = deck;
        new_deck->next->prev = new_deck;
        new_deck = new_deck->next;


        if(part->next != NULL){
            //Add card from second pile to new_deck
            new_deck->next = part;
            new_deck->next->prev = new_deck;
            new_deck = new_deck->next;
        }
        // Update pointers
        deck->prev = NULL;
        part->prev = NULL;
        deck = deck->next;
        part = part->next;
    }
    new_deck->next = part;
    part->prev = new_deck;

    return new_deck;
}

Card *random_shuffle(Card *deck){
    return deck;
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
    while(ptr->next != NULL && (ptr->rank != rank && ptr->suit != suit)){
        ptr = ptr->next;
    }
    if(ptr->rank != rank && ptr->suit != suit || ptr->prev == NULL){ //Checks if the specified card, is the dummy card + checking if the card exists
        error_message();
        return NULL;
    }
    return ptr;
}

//the foundations will not have a predefined suit. The suit of the foundation will be defined by the first card moved to it.
bool move_to_foundation(Card *card, Card *toPile) {
    bool possMove = false;
    if(card->next != NULL)//checks if we're trying to move more than one card.
        return possMove;

    if (toPile->prev == NULL){//Checks if the foundation is empty.
        if(card->rank == ranks[0])//if the foundation is empty check if the card is an Ace.
            possMove = true;
    }else{
        //find topcard in foundation
        while(toPile->next != NULL){
            toPile = toPile->next;
        }
        int i = 0;
        while(ranks[i] != card->rank){ //Find the index for the cards rank.
            i++;
        }

        if(ranks[i-1] == toPile->rank){//checks if the topcard is one rank less, than the card we with to move.
            if(card->suit == toPile->suit)//checks for same suit.
                possMove = true;
        }
    }
    return possMove;
}

bool valid_move(Card *card,Card *topile){
    bool valid = false;
    int i = 0;

    if(topile->rank == foundations[0]->rank){
        return move_to_foundation(card,topile);
    }

    while(topile->next != NULL){
        topile = topile->next;
    }
    while(ranks[i] != card->rank){
        i++;
    }
    if(i < 12 && ranks[i+1] == topile->rank){
        if(card->suit != topile->suit)
            valid = true;
    }
    if(i == 12){
        //checks if pile is empty for king move
        if(topile->prev == NULL)
            valid = true;
    }
    return valid;
}

void move_specific(const char *command,Card *pointer) {
    //checks if card exists in pile / list
    pointer = find_card(command[3],command[4],pointer);
    if(pointer == NULL){
        error_message();
        return;
    }

    //finds the column or foundation specified in the command.
    Card *to = get_pile(command[7],command[8]);
    if(to == NULL){
        error_message();
        return;
    }

    while(to->next != NULL){
        to = to->next;
    }
    //from this point we have found the card from a pile, and the pile it's supposed to go to.
    //now we check if the move is valid.
    if(valid_move(pointer,to)){
        //moves card from a stack
        Card *temp = pointer->prev;
        temp->next = NULL;

        pointer->prev = to;
        to->next = pointer;
    }
}


void pile_to_pile(const char *command,Card *pointer) {
    Card *to = get_pile(command[4],command[5]);
    if (to == NULL){
        error_message();
        return;
    }

    while(pointer->next != NULL){
        pointer = pointer->next;
    }
    while(to->next != NULL){
        to = to->next;
    }

    Card *temp;
    if(valid_move(pointer,to)){
        temp = pointer->prev;
        temp->next = NULL; //Dereferencing card from old list.

        pointer->prev =  to;
        to->next = pointer;//Referencing card in new list.
    }

}

void move(const char *command, int strlen) {
    Card *temp = get_pile(command[0], command[1]);//gets pointer of list to move from
    if (temp == NULL) {
        error_message();//if the get_pile method returns a null pointer we return to caller with error message
        return;
    }

    int type = 2;
    if (strlen == 6)
        type = 0;
    else if (strlen == 9)
        type = 1;


    switch (type) {
        case 0 :
            pile_to_pile(command, temp);
            break;
        case 1:
            move_specific(command, temp);
            break;
        case 2:
            error_message();
            return;
    }

}


void show(){
    Card *temp = head;
    printf("\n\tC1\tC2\tC3\tC4\tC5\tC6\tC7\n");
    int i = 0;
    int j = 1;
    int f;
    char s[2];
    while(temp->next != NULL){
        if(i % 7 == 0) {
            if(j % 2 == 0 && j != 1) {
                f = j / 2;
                s[0] = 'F';
                s[1] = f + '0';
                printf("\t[]\t%s",s);
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
        while(temp->next != NULL){
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

    int max_length = find_longest_list();
    if(max_length < 8)
        max_length = 8;

    printf("\n");
    for (int i = 0; i < sizeof(placeholder) / sizeof(placeholder[0]); ++i) {//Prints the dummy cards for all columns
        printf("\t%c%c",placeholder[i]->rank,placeholder[i]->suit);
    }
    printf("\n");
    int f = 1;
    Card* foundation_temp;
    //printing all cards in the column or and empty space if there is no card.
    for (int i = 0; i < max_length; ++i) {
        printf("\n\t");
        for (int j = 0; j < sizeof(placeholder) / sizeof(placeholder[0]); ++j) {
            if (placeholder[j]->next != NULL){
                placeholder[j] = placeholder[j]->next;
                printf("%c%c\t",placeholder[j]->rank, placeholder[j]->suit);
            }
            else
                printf("  \t");
        }
        //printing foundation cards for evert second row.
        f++;
        if (f % 2 == 0 && ((f/2) - 1) < (sizeof(foundations) / sizeof(foundations[0]))){
            foundation_temp = foundations[(f / 2) - 1];
            while (foundation_temp->next != NULL) {
                foundation_temp = foundation_temp->next;
            }
            if(foundation_temp->prev == NULL){
                printf("[]\tF%c",(f / 2) + '0');
            }else{
                printf("%c%c\t%c%c",foundation_temp->rank,foundation_temp->suit,foundations[(f / 2) - 1]->rank,foundations[(f / 2) - 1]->suit);
            }
        }
    }


}

const char* get_input() {
    char input[127];
    printf("Enter command: ");
    scanf("%s", &input);
    return input;
}

void setup_columns_foundations(){
    for (int i = 0; i < 4; ++i) {
        foundations[i] = new_card('F', i + 1 +'0'); //dummy card
    }
    for (int i = 0; i < 7; ++i) {
        columns[i] = new_card('C', i + 1 + '0'); //dummy card
    }
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

int main() {

//    setup_columns_foundations();
//    Card *first_card = columns[0];
//    Card *card  = new_card('A', 'C');
//    first_card->next = card;
//    //Test to show how it could be made in conole
//    printf("\tC1\tC2\tC3\tC4\tC5\tC6\tC7\n\n");
//    printf("\t%c%c\t[]\t[]\t[]\t[]\t[]\t[]\t\tF1\n", first_card->next->rank, first_card->next->suit);
//    printf("\t  \t7H\t[]\t[]\t[]\t[]\t[]\t\t\n");
//    printf("\t  \t  \t5H\t[]\t[]\t[]\t[]\t\tF2\n");
//    printf("\t  \t  \t  \t6C\t[]\t[]\t[]\t\t\n");
//    printf("\t  \t  \t  \t  \t6S\t[]\t[]\t\tF3\n");
//    printf("\t  \t  \t  \t  \t  \tQC\t[]\t\t\n");
//    printf("\t  \t  \t  \t  \t  \t  \tKH\t\tF4\n");
//    printf("\t  \t  \t  \t  \t  \t  \t  \t\t\n");

//test for move method
/*
setup_columns_foundations();
    Card *tempCard = new_card('K','H');
    columns[4]->next = tempCard;
    tempCard->prev = columns[4];
    card *tempcard2 = new_card('A','H');
    columns[0]->next = tempcard2;
    tempcard2->prev = columns[0];
    print_gamestate();
    char moveF[] = "C5:KH->C6  ";
    move(moveF, find_string_length(moveF));
    char moveE[] = "C1->F1";
    move(moveE, find_string_length(moveE));
    print_gamestate();
    */

    //Test for show method
    head = load_deck("C:\\Users\\emil1\\OneDrive\\Documents\\GitHub\\YukonS-G50\\Test_input.txt");
    show();

    // Test to print all cards, if no input file is provided
    Card *deck = default_deck();

    Card *play_deck = interleave_shuffle(deck, 20);
    do {
        play_deck = play_deck->next;
        printf("%c%c\n",play_deck->rank, play_deck->suit);
    }  while (play_deck->next != NULL);
//
//
//    do {
//        printf("%c%c\n",first_card->rank, first_card->suit);
//        first_card = first_card->next;
//        printf("%c%c\n",first_card->rank, first_card->suit);
//    }  while (first_card->next != NULL);

    //clear screen
    //system("cls");

    //save_cards(deck, "C:\\DTU\\2-semester\\MaskinarProgrammering\\Yukon\\YukonS-G50\\Test1_input.txt");



    return 0;
}