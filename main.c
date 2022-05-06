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
    Card *card = malloc(sizeof(Card));
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

// Helper method for interleave_shuffle
Card *split_deck(Card *deck, int amount){
    Card *temp_deck = deck;
    // Loop to find spot to split
    for (int i = 0; i <= amount; ++i) {
        temp_deck = temp_deck->next;
    }
    temp_deck->prev = NULL;
    Card *part = temp_deck; // Variable for holding split part of deck
    //temp_deck->next = NULL;
    return part;
}


Card *interleave_shuffle(Card *deck, int amount){
    Card *new_deck = deck; // Variable for holding interleaved deck
    Card *part = split_deck(deck, amount);

    // Add dummy card to new card deck
    deck = deck->next; // Deck now has first actual card
    new_deck->next = NULL; // Set next pointer to NULL
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

void show(){
    Card *temp = head;
    printf("\tC1\tC2\tC3\tC4\tC5\tC6\tC7\n");
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

void print_gamestate(){
    Card *placeholder[7];

    //copying head pointers to foundations linked lists over to another array.
    for (int i = 0; i < sizeof(placeholder) / sizeof(placeholder[0]); ++i) {
        placeholder[i] = columns[i];
    }

    int max_length = 0;
    //temp pointer to iterate through lists
    Card *temp;
    //finding the longest linked list.
    for (int i = 0; i < sizeof(placeholder) / sizeof(placeholder[0]); ++i) {
        int j = 0;
        temp = placeholder[i];
        while(temp->next != NULL){
            temp = temp->next;
            j++;
        }
        if(j > max_length)
            max_length = j;
    }

    printf("\tC1\tC2\tC3\tC4\tC5\tC6\tC7\n");
    int f = 1;
    Card* foundation_temp;
    //printing all cards in the column or and empty space if there is no card.
    for (int i = 0; i < max_length; ++i) {
        printf("\n");
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
        if (f % 2 == 0 && (f/2) < sizeof(foundations) / sizeof(foundations[0])){
            foundation_temp = foundations[(f / 2) - 1];
            if(foundation_temp->next == NULL){
                printf("[]\tF%c",(f / 2) + '0');
            }
            else {
                while (foundation_temp->next != NULL) {
                    foundation_temp = foundation_temp->next;
                }
                printf("%c%c\t",foundation_temp->rank,foundation_temp->suit);
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
        foundations[i] = new_card('d', 'd'); //dummy card
    }
    for (int i = 0; i < 7; ++i) {
        columns[i] = new_card('d', 'd'); //dummy card
    }
}


int main() {
    int a = 4;
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

    //Test for show method
    //head = load_deck("C:\\Users\\emil1\\OneDrive\\Documents\\GitHub\\YukonS-G50\\Test_input.txt");
    //show();

    // Test to print all cards, if no input file is provided
    Card *deck = default_deck();
    head = interleave_shuffle(deck, 10);
    show();

    //Card *play_deck = interleave_shuffle(deck, 20);
//    do {
//        deck = deck->next;
//        printf("%c%c\n",deck->rank, deck->suit);
//    }  while (deck->next != NULL);
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
