#include <iostream>
#include <cstring>
#include <fstream>
#include <ctime>
#include <string> 

using namespace std;

void ResCor(char* str) {
    for (int i = 0; str[i]; i++) {
        if (str[i] >= 'A' && str[i] <= 'Z')
            str[i] = str[i] - 'A' + 'a';
    }
}

void ResCor(string& str) {
    for (char& c : str) {
        if (c >= 'A' && c <= 'Z')
            c = c - 'A' + 'a';
    }
}

bool isNumber(const char* str) {
    for (int i = 0; str[i]; i++) {
        if (str[i] < '0' || str[i] > '9') return false;
    }
    return true;
}

bool getValidatedNumber(const char* prompt, int& result, int min, int max, int maxRetries = 5) {
    int retries = 0;
    char buffer[100];
    while (retries < maxRetries) {
        cout << prompt;
        cin.getline(buffer, 100);
        if (!isNumber(buffer)) {
            cout << "Invalid input. Numbers only.\n";
            retries++;
            continue;
        }
        int temp = atoi(buffer);
        if (temp < min || temp > max) {
            cout << "Input out of range (" << min << " to " << max << "). Try again.\n";
            retries++;
            continue;
        }
        result = temp;
        return true;
    }
    return false;
}

bool clearEnabled = true;
void clearScreen() {
    if (!clearEnabled) return;
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

int atoi(const char* str) {
    int result = 0;
    int sign = 1;
    int i = 0;

    while (str[i] == ' ' || str[i] == '\t') {  
        i++;  
    }  

    if (str[i] == '-') {  
        sign = -1;  
        i++;  
    } else if (str[i] == '+') {  
        i++;  
    }  

    while (str[i] >= '0' && str[i] <= '9') {  
        result = result * 10 + (str[i] - '0');  
        i++;  
    }  

    return sign * result;
}

class settings {
protected:
    int loop = 3, player = 3;
    const int MIN_PLAYERS = 3, MAX_PLAYERS = 15;
    const int MIN_LOOPS = 1, MAX_LOOPS = 5;

public:
    void adjustloop() {
        cout << "Current number of loops: " << loop << endl;
        cout << "How many loops are playing? (between 1 to 5): " << endl;
        if (getValidatedNumber("Enter loops (1-5): ", loop, 1, 5))
            return;
        cout << "Too many invalid attempts. Returning to menu.\n";
    }

    void adjustplayer()  
    {  
        cout << "Current number of players: " << player << endl;  
        cout << "How many players are playing? (between 3 to 15): " << endl;  
        if (getValidatedNumber("Enter players (3-15): ", player, 3, 15))   // FIX: Corrected variable
            return;  
        cout << "Too many invalid attempts. Returning to menu.\n";  
    }  

    int getLoop() const { return loop; }  
    int getPlayer() const { return player; }
};

class Game : public settings {
public:
    void maingame() {

        int totalTurns = loop * player;  
        string* Input = new string[totalTurns];  

        clearScreen();  
        cout << "Player 1 Turn\nWrite Down a Word: ";  
        getline(cin, Input[0]);  

        for (int turn = 1; turn < totalTurns; turn++) {  
            clearScreen();  

            int currentPlayer = (turn % player) + 1;  
            bool isSentenceTurn = (turn % 2 == 1);    

            if (isSentenceTurn) {  
                cout << "Player " << currentPlayer << " Turn\nWrite a sentence using: " << Input[turn - 1] << "\nHere: ";  
                getline(cin, Input[turn]);  
            } else {  
                cout << "Player " << currentPlayer << " Turn\n Sentence : " << Input[turn - 1]  
                     << "\nWrite Down a Word from the sentence: ";  
                getline(cin, Input[turn]);  
            }  
        }  

        clearScreen();  
        cout << "Game Over!\n";  
        for(int i=0;i<loop;i++)  
        {  
            for(int j=0;j<player;j++)  
            {  
                cout << "Player " << j+1 << " said : " << Input[j+i*player] << endl;  
            }  
        }  
        cout << endl;  
        cout << "Starting word: " << Input[0] << "\n";  
        cout << "Final input: " << Input[totalTurns - 1] << "\n";  

        AskUpdateLog(Input, totalTurns);  

        delete[] Input;
    }

    void AskUpdateLog(string* inputs, int totalTurns) {
        char response[10];
        while (true) {
            cout << "\nDo you want to update the log? (Yes/No): ";
            cin >> response;

            ResCor(response);  // Convert input to lowercase  

            if (response[0] == 'y') {  
                UpdateLog(inputs, totalTurns);  
                break;  
            } else if (response[0] == 'n') {  
                cout << "Log update skipped. Exiting...\n";  
                break;  
            } else {  
                cout << "Invalid input. Please type Yes or No.\n";  
            }  
        }
    }

    void UpdateLog(string* inputs, int totalTurns) {
        // Generate timestamp
        time_t now = time(0);
        tm* ltm = localtime(&now);

        char filename[100];  
        strftime(filename, sizeof(filename), "log_%Y-%m-%d_%H-%M-%S.txt", ltm);  

        ofstream file(filename);  
        if (!file) {  
            cerr << "Error opening file for writing!" << endl;  
            return;  
        }  

        file << "Game Log\n";  
        file << "Total Turns: " << totalTurns << "\n\n";  

        for (int i = 0; i < totalTurns; i++) {  
            int currentPlayer = (i % player) + 1;  
            int currentLoop = (i / player) + 1;  
            file << "Loop " << currentLoop << ", Player " << currentPlayer << ": " << inputs[i] << "\n";  
        }  

        file.close();  
        cout << "\nGame log saved to '" << filename << "'.\n";

    }
};

void Credits() {
    clearScreen();
    cout << "This game is created by \n\n"
         << ">> Vedant Mandanka \n   12402130501063 \n\n"
         << ">> Shreya Thacker \n   12402130501056 \n\n"
         << "Whole idea was created by Shreya and Vedant implemented that idea into code!"
         << endl;
}

void Help() {
    while (true) {
        clearScreen();
        cout << "What help do you need?" << endl
             << "Type 1 or 2 or 3" << endl << endl
             << "1: How to play the game?" << endl
             << "2: What are the requirements of this game?" << endl
             << "3: What is the goal of this game?" << endl
             << "4: Return to the Main Menu" << endl;

        char Temp1[100];  
        cout << "Your Response :- ";  
        cin.getline(Temp1, 100);  

        if (!isNumber(Temp1)) {  
            cout << "Invalid input. Please enter a number between 1 and 4.\n";  
            cout << "Press Enter to continue...";  
            cin.getline(Temp1, 100);  
            continue;  
        }  

        int choice = atoi(Temp1);  

        switch (choice) {  
            case 1:  
                clearScreen();  
                cout << "-: Here is the detailed guide about the Wordic Phone :-\n" << endl;  
                cout << "Instructions :- " << endl;  
                cout << "Step 1 :- Give the Phone to Player 1 and tell him to write down a word.\n"  
                     << "Step 2 :- After typing give the phone to Player 2/Next Player\n"  
                     << "Step 3 :- Player 2 tries to create a sentence out of that word and pass the phone to Player 3\n"  
                     << "Step 4 :- Player 3 tries to pick a word out of that sentence and the loop continues\n\n"  
                     << endl;  
                break;  
            case 2:  
                clearScreen();  
                cout << "Requirements :- " << endl;  
                cout << "1. There will be at least 3 players playing this game.\n"  
                     << "2. Follow the instructions given below.\n"  
                     << "3. Play with a smile :)\n" << endl;  
                break;  
            case 3:  
                clearScreen();  
                cout << "Main goal of this game is to find out what we started with and what we ended with! XD\n";  
                break;  
            case 4:  
                return;      
            default:  
                cout << "Invalid option. Please try again." << endl;  
                continue;  
        }  

        while (true) {  
            cout << "Do you need any more help?" << endl;  
            cout << "Type Yes or No" << endl;  

            char response[10];  
            cin >> response;  
            ResCor(response);  

            if (response[0] == 'y') {  
                cin.ignore(10000, '\n');  
                break;   
            } else if (response[0] == 'n') {  
                cin.ignore(10000, '\n');  
                return;   
            } else {  
                cout << "Invalid input. Please try again." << endl;  
            }  
        }  
    }
}

int main() {
    Game game;
    char input[100];

    while (true) {  
        clearScreen();  
        cout << "Welcome to Wordic Phone Game!\n\n";  
        cout << "Menu:\n";  
        cout << "1. Start\n";  
        cout << "2. Settings\n";  
        cout << "3. Guide\n";  
        cout << "4. Credits\n";  
        cout << "5. Exit\n";  
        cout << "Enter your choice (1-5): ";  

        cin.getline(input, 100);  

        if (!isNumber(input)) {  
            cout << "Invalid input. Please enter a number between 1 and 5.\n";  
            cout << "Press Enter to continue...";  
            cin.getline(input, 100);  
            continue;  
        }  

        int choice = atoi(input);  

        switch (choice) {  
            case 1: {  
                do {  
                    game.maingame();  
                    cout << "\nPlay again? (Yes/No): ";  
                    char response[10];  
                    cin >> response;  
                    ResCor(response);  
                    cin.ignore(10000, '\n');  
                    if (response[0] != 'y') break;  
                } while (true);  
                cout << "\nPress Enter to return to menu...";  
                cin.getline(input, 100);  
                break;  
            }  
            case 2: {  
                while (true) {  
                    clearScreen();  
                    cout << "Settings:\n";  
                    cout << "What do you want to adjust? " << endl;  
                    cout << "The current adjustments are : \n    Loops = " << game.getLoop() << "\n    Players = " << game.getPlayer() << endl;  
                    cout << "1. Change Loops\n";  
                    cout << "2. Change Players\n";  
                    cout << "3. Return to Main Menu\n";  
                    cout << "Enter your choice (1-3): ";  

                    char input[100];  
                    cin.getline(input, 100);  

                    if (!isNumber(input)) {  
                        cout << "Invalid input. Please enter a number between 1 and 3.\n";  
                        cout << "Press Enter to continue...";  
                        cin.getline(input, 100);  
                        continue;  
                    }  

                    int Tempe = atoi(input);  

                    if (Tempe == 1) {  
                        game.adjustloop();  
                    } else if (Tempe == 2) {  
                        game.adjustplayer();  
                    } else if (Tempe == 3) {  
                        break;  
                    } else {  
                        cout << "Invalid choice.\n";  
                        cout << "Press Enter to continue...";  
                        cin.getline(input, 100);  
                        continue;  
                    }  

                    cout << "\nPress Enter to return to menu...";  
                    cin.getline(input, 100);  
                }  
                break;  
            }  
            case 3:  
                Help();  
                cout << "\nPress Enter to return to menu...";  
                cin.getline(input, 100);  
                break;  

            case 4:  
                Credits();  
                cout << "\nPress Enter to return to menu...";  
                cin.getline(input, 100);  
                break;  

            case 5:  
                clearScreen();  
                cout << "Thank you for playing! Goodbye.\n";  
                return 0;  

            default:  
                cout << "Invalid choice. Please try again.\n";  
                cout << "Press Enter to continue...";  
                cin.getline(input, 100);  
                break;  
        }  
    }
}
