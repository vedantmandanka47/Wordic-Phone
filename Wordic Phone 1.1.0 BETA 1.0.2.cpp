#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <ctime>
#include <algorithm>
#include <filesystem>
#include <cctype>
#include <cstdio>

namespace fs = std::filesystem;
using namespace std;

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pressEnterToContinue() {
    cout << "\nPress Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void toLower(string &s) {
    transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return tolower(c); });
}

bool isNumber(const string& s) {
    return !s.empty() && all_of(s.begin(), s.end(), ::isdigit);
}

int getInt(const string& prompt, int min, int max, int maxRetries = 5) {
    string input;
    int value, tries = 0;
    while (tries < maxRetries) {
        cout << prompt;
        getline(cin, input);
        if (isNumber(input)) {
            value = stoi(input);
            if (value >= min && value <= max)
                return value;
        }
        cout << "\033[31mInvalid input. Please enter a number between " << min << " and " << max << ".\033[0m\n";
        tries++;
    }
    throw runtime_error("Too many invalid attempts.");
}

string getInput(const string& prompt) {
    cout << prompt;
    string input;
    getline(cin, input);
    return input;
}

int countWords(const string& s) {
    istringstream iss(s);
    int count = 0;
    string word;
    while (iss >> word) count++;
    return count;
}

bool isSingleWord(const string& s) {
    return s.find(' ') == string::npos && !s.empty();
}

bool isSentence(const string& s) {
    return s.find(' ') != string::npos && !s.empty() && (s.back() == '.' || s.back() == '!' || s.back() == '?');
}

void ensureLogsDir() {
    if (!fs::exists("logs"))
        fs::create_directory("logs");
}

vector<string> listLogFiles() {
    ensureLogsDir();
    vector<string> files;
    for (auto& entry : fs::directory_iterator("logs")) {
        if (entry.is_regular_file())
            files.push_back(entry.path().filename().string());
    }
    return files;
}

void showLog(const string& filename) {
    clearScreen();
    ifstream file("logs/" + filename);
    if (!file) {
        cout << "\033[31mError opening log file: " << filename << "\033[0m\n";
        return;
    }
    cout << "\033[36m--- Log: " << filename << " ---\033[0m\n";
    string line;
    while (getline(file, line))
        cout << line << endl;
    file.close();
}

void renameLogFile() {
    vector<string> logs = listLogFiles();
    if (logs.empty()) {
        cout << "No logs available to rename.\n";
        return;
    }
    cout << "Available logs:\n";
    for (size_t i = 0; i < logs.size(); ++i)
        cout << i+1 << ". " << logs[i] << endl;
    int idx = getInt("Select a log to rename (number): ", 1, logs.size());
    string oldName = logs[idx-1];
    string newName = getInput("Enter new name (without extension): ");
    if (newName.empty()) {
        cout << "Cancelled.\n";
        return;
    }
    string newPath = "logs/" + newName + ".txt";
    if (fs::exists(newPath)) {
        cout << "A log with that name already exists.\n";
        return;
    }
    fs::rename("logs/" + oldName, newPath);
    cout << "Log renamed successfully.\n";
}

struct Player {
    string name;
    bool active = true;
};

vector<Player> getPlayerNames(int numPlayers) {
    vector<Player> players(numPlayers);
    cout << "Would you like to enter player names? (y/n): ";
    string resp;
    getline(cin, resp);
    toLower(resp);
    if (!resp.empty() && resp[0] == 'y') {
        for (int i = 0; i < numPlayers; ++i) {
            string name = getInput("Enter name for Player " + to_string(i+1) + " (leave blank for default): ");
            if (name.empty()) name = "Player " + to_string(i+1);
            players[i].name = name;
        }
    } else {
        for (int i = 0; i < numPlayers; ++i)
            players[i].name = "Player " + to_string(i+1);
    }
    return players;
}

struct GameState {
    vector<string> turns;
    vector<Player> players;
    int loops;
    int minSentenceWords = 5;
    vector<int> turnPlayerIdx; // For undo support
};

void playGame(GameState& state) {
    int totalTurns = state.loops * state.players.size();
    state.turns.resize(totalTurns);
    state.turnPlayerIdx.resize(totalTurns);
    vector<bool> playerActive(state.players.size(), true);

    int turn = 0;
    cout << "\033[33mGame Start!\033[0m\n";
    cout << "Type /back to undo previous input, /drop to drop out a player.\n";
    while (turn < totalTurns) {
        int playerIdx = turn % state.players.size();
        if (!state.players[playerIdx].active) {
            turn++;
            continue;
        }
        state.turnPlayerIdx[turn] = playerIdx;

        clearScreen();
        cout << "\033[36mTurn " << (turn+1) << " of " << totalTurns << "\033[0m\n";
        cout << "\033[32m" << state.players[playerIdx].name << "'s turn\033[0m\n";

        string prompt;
        if (turn == 0) {
            prompt = "Write down a word: ";
        } else if (turn % 2 == 1) {
            prompt = "Write a sentence using the word: \"" + state.turns[turn-1] + "\"\n(Min " + to_string(state.minSentenceWords) + " words, end with . ! or ?)\nHere: ";
        } else {
            prompt = "Pick a word from the sentence: \"" + state.turns[turn-1] + "\"\nHere: ";
        }

        string input = getInput(prompt);

        // Special commands
        if (input == "/back") {
            if (turn > 0) {
                turn--;
                continue;
            } else {
                cout << "Already at the first turn.\n";
                pressEnterToContinue();
                continue;
            }
        }
        if (input == "/drop") {
            cout << "Are you sure you want to drop " << state.players[playerIdx].name << "? (y/n): ";
            string conf;
            getline(cin, conf);
            toLower(conf);
            if (!conf.empty() && conf[0] == 'y') {
                state.players[playerIdx].active = false;
                cout << state.players[playerIdx].name << " has dropped out.\n";
                pressEnterToContinue();
                turn++;
                continue;
            } else {
                cout << "Dropout cancelled.\n";
                pressEnterToContinue();
                continue;
            }
        }

        // Input validation
        if (turn == 0) {
            if (!isSingleWord(input)) {
                cout << "Please enter a single word (no spaces).\n";
                pressEnterToContinue();
                continue;
            }
        } else if (turn % 2 == 1) {
            if (!isSentence(input)) {
                cout << "Please write a valid sentence ending with a period, exclamation, or question mark.\n";
                pressEnterToContinue();
                continue;
            }
            if (countWords(input) < state.minSentenceWords) {
                cout << "Sentence too short! Minimum " << state.minSentenceWords << " words required.\n";
                pressEnterToContinue();
                continue;
            }
            // Optionally: check if previous word is present in sentence
            string prevWord = state.turns[turn-1];
            string lowerInput = input; toLower(lowerInput);
            string lowerPrev = prevWord; toLower(lowerPrev);
            if (lowerInput.find(lowerPrev) == string::npos) {
                cout << "Sentence must use the word \"" << prevWord << "\".\n";
                pressEnterToContinue();
                continue;
            }
        } else {
            if (!isSingleWord(input)) {
                cout << "Please enter a single word (no spaces).\n";
                pressEnterToContinue();
                continue;
            }
            // Optionally: check if word is present in previous sentence
            stringstream ss(state.turns[turn-1]);
            bool found = false;
            string w;
            while (ss >> w) {
                if (w == input) { found = true; break; }
            }
            if (!found) {
                cout << "Word not found in previous sentence. Please pick a word from the sentence.\n";
                pressEnterToContinue();
                continue;
            }
        }
        state.turns[turn] = input;
        turn++;
    }
    clearScreen();
    cout << "\033[33mGame Over!\033[0m\n";
    for (int i = 0; i < totalTurns; ++i) {
        int playerIdx = state.turnPlayerIdx[i];
        cout << "Turn " << (i+1) << " [" << state.players[playerIdx].name << "]: " << state.turns[i] << endl;
    }
}

string getLogFilename(bool custom) {
    ensureLogsDir();
    if (custom) {
        string name = getInput("Enter custom log name (no extension): ");
        string filename = "logs/" + name + ".txt";
        if (fs::exists(filename)) {
            cout << "A log with that name already exists. Overwrite? (y/n): ";
            string resp;
            getline(cin, resp);
            toLower(resp);
            if (resp.empty() || resp[0] != 'y')
                return "";
        }
        return filename;
    } else {
        time_t now = time(0);
        tm* ltm = localtime(&now);
        char buf[100];
        strftime(buf, sizeof(buf), "logs/log_%Y-%m-%d_%H-%M-%S.txt", ltm);
        return string(buf);
    }
}

void saveLog(const GameState& state) {
    cout << "Would you like to name the log file? (y/n): ";
    string resp;
    getline(cin, resp);
    toLower(resp);
    string filename = getLogFilename(!resp.empty() && resp[0] == 'y');
    if (filename.empty()) {
        cout << "Log not saved.\n";
        return;
    }
    ofstream file(filename);
    if (!file) {
        cout << "Failed to save log.\n";
        return;
    }
    file << "Wordic Phone Game Log\n";
    file << "Players: ";
    for (auto& p : state.players)
        file << p.name << (p.active ? "" : " (dropped out)") << ", ";
    file << "\nLoops: " << state.loops << "\n";
    file << "Turns:\n";
    for (size_t i = 0; i < state.turns.size(); ++i)
        file << "Turn " << (i+1) << " [" << state.players[state.turnPlayerIdx[i]].name << "]: " << state.turns[i] << "\n";
    file.close();
    cout << "Log saved as: " << filename << endl;
}

void showWelcomePage() {
    clearScreen();
    cout << setfill('=') << setw(60) << "=" << endl;
    cout << setfill(' ') << setw(30) << " " << "\033[1;34mWORDIC PHONE GAME\033[0m" << endl;
    cout << setw(23) << " " << "by Vedant Mandanka & Shreya Thacker" << endl;
    cout << setfill('=') << setw(60) << "=" << endl << setfill(' ');
    cout << "Welcome!\n";
    cout << "Minimum 3 players required.\n";
    cout << "Follow the on-screen prompts.\n";
    cout << "Have fun and enjoy!\n";
    cout << setfill('=') << setw(60) << "=" << endl << setfill(' ');
    pressEnterToContinue();
}

void showHelp() {
    while (true) {
        clearScreen();
        cout << "Help Menu:\n";
        cout << "1. How to play\n";
        cout << "2. Requirements\n";
        cout << "3. Game Goal\n";
        cout << "4. Return to Main Menu\n";
        int choice = getInt("Your choice: ", 1, 4);
        if (choice == 1) {
            clearScreen();
            cout << "- Give the phone to Player 1 to write a word.\n";
            cout << "- Next player makes a sentence using that word.\n";
            cout << "- Next player picks a word from the sentence.\n";
            cout << "- Continue for all players and loops.\n";
            pressEnterToContinue();
        } else if (choice == 2) {
            clearScreen();
            cout << "- Minimum 3 players, maximum 15.\n";
            cout << "- 1 to 5 loops.\n";
            cout << "- Play with a smile :)\n";
            pressEnterToContinue();
        } else if (choice == 3) {
            clearScreen();
            cout << "Goal: See how the starting word changes by the end!\n";
            pressEnterToContinue();
        } else {
            break;
        }
    }
}

void showCredits() {
    clearScreen();
    cout << "This game is created by:\n";
    cout << "Vedant Mandanka (12402130501063)\n";
    cout << "Shreya Thacker (12402130501056)\n";
    cout << "Idea: Shreya, Implementation: Vedant\n";
    pressEnterToContinue();
}

int main() {
    int loops = 3, numPlayers = 3;
    showWelcomePage();
    while (true) {
        clearScreen();
        cout << "\033[1;34mWordic Phone Game\033[0m\n";
        cout << "1. Start Game\n";
        cout << "2. Settings\n";
        cout << "3. Help\n";
        cout << "4. Credits\n";
        cout << "5. View Logs\n";
        cout << "6. Rename Log File\n";
        cout << "7. Exit\n";
        int choice = getInt("Enter your choice: ", 1, 7);

        if (choice == 1) {
            GameState state;
            state.loops = loops;
            state.players = getPlayerNames(numPlayers);
            playGame(state);
            cout << "\nWould you like to save this game log? (y/n): ";
            string resp;
            getline(cin, resp);
            toLower(resp);
            if (!resp.empty() && resp[0] == 'y')
                saveLog(state);
            pressEnterToContinue();
        } else if (choice == 2) {
            clearScreen();
            cout << "Settings:\n";
            try {
                loops = getInt("Number of loops (1-5): ", 1, 5);
                numPlayers = getInt("Number of players (3-15): ", 3, 15);
            } catch (...) {
                cout << "Returning to main menu.\n";
                pressEnterToContinue();
            }
        } else if (choice == 3) {
            showHelp();
        } else if (choice == 4) {
            showCredits();
        } else if (choice == 5) {
            vector<string> logs = listLogFiles();
            if (logs.empty()) {
                cout << "No logs found.\n";
            } else {
                cout << "Available logs:\n";
                for (size_t i = 0; i < logs.size(); ++i)
                    cout << i+1 << ". " << logs[i] << endl;
                int idx = getInt("Select a log to view (number, 0 to cancel): ", 0, logs.size());
                if (idx > 0)
                    showLog(logs[idx-1]);
            }
            pressEnterToContinue();
        } else if (choice == 6) {
            renameLogFile();
            pressEnterToContinue();
        } else if (choice == 7) {
            cout << "Thank you for playing! Goodbye.\n";
            break;
        }
    }
    return 0;
}
