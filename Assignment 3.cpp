#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cstring>

using namespace std;

const int RECORD_SIZE = 51; // author(16) + title(24) + isbn(10) + deleted(1)

// fixed length book class
class Book {
private:
    char author[16];
    char title[24];
    char isbn[10];
    bool deleted; 

public:
    Book(const string& author = "", const string& title = "", const string& isbn = "") {
        deleted = false;
        strncpy(this->author, author.c_str(), sizeof(this->author) - 1);
        this->author[sizeof(this->author) - 1] = '\0';

        strncpy(this->title, title.c_str(), sizeof(this->title) - 1);
        this->title[sizeof(this->title) - 1] = '\0';

        strncpy(this->isbn, isbn.c_str(), sizeof(this->isbn) - 1);
        this->isbn[sizeof(this->isbn) - 1] = '\0';
    }
    //getters and setters
    string getAuthor() const { return string(author); }
    string getTitle() const { return string(title); }
    string getIsbn() const { return string(isbn); }
    void setAuthor(const string& newAuthor) {
        strncpy(author, newAuthor.c_str(), sizeof(author) - 1);
        author[sizeof(author) - 1] = '\0'; // null termination
    }

    void setTitle(const string& newTitle) {
        strncpy(title, newTitle.c_str(), sizeof(title) - 1);
        title[sizeof(title) - 1] = '\0'; // null termination
    }

    void setIsbn(const string& newIsbn) {
        strncpy(isbn, newIsbn.c_str(), sizeof(isbn) - 1);
        isbn[sizeof(isbn) - 1] = '\0'; // null termination
    }
    bool isDeleted() const { return deleted; }
    void markAsDeleted() { deleted = true; }

    void writeToDataFile(fstream& file) const { // these are alwways 51 bytes, read or write
        file.write(author, sizeof(author));
        file.write(title, sizeof(title));
        file.write(isbn, sizeof(isbn));
        file.write(reinterpret_cast<const char*>(&deleted), sizeof(deleted));
    }

    void readFromDataFile(fstream& file) {
        file.read(author, sizeof(author));
        file.read(title, sizeof(title));
        file.read(isbn, sizeof(isbn));
        file.read(reinterpret_cast<char*>(&deleted), sizeof(deleted));
    }

    void display() const {
        if (!deleted) {
            cout << "Author: " << getAuthor() << ", Title: " << getTitle() << ", ISBN: " << getIsbn() << endl;
        }
        else {
            cout << "[Deleted Record]" << endl;
        }
    }
};

string trim(const string& str) { //trimming whitespace
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == string::npos) return ""; // no content

    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);

}

int convertIsbnToInt(const string& isbnStr) { // since isbn is a string, some cases I needed it to be an int
    string numericIsbn;
    for (char c : isbnStr) {
        if (isdigit(c)) { // keep only numbers, as some ISBNs have dashes, etc
            numericIsbn += c;
        }
    }
    return numericIsbn.empty() ? -1 : stoi(numericIsbn);
}

int searchIndexTitle(const vector<string>& rrnTitleIndex, string title) { // search titleIndex and return rrn
    // trim title to get rid of weird bugs
    title = trim(title);

    int low = 0, high = rrnTitleIndex.size() - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        // use delimited to extract title and rrn
        stringstream ss(rrnTitleIndex[mid]);
        string currentTitle;
        int rrn;
        getline(ss, currentTitle, '|'); 
        ss >> rrn;


        currentTitle = trim(currentTitle);

        if (currentTitle == title) {

            return rrn; // return the RRN of the found book
        }
        else if (currentTitle < title) {
            low = mid + 1; // search in the upper half
        }
        else {
            high = mid - 1; // search in the lower half
        }
    }

    return -1; // title not found
}

int searchIndexPositionTitle(const vector<string>& rrnTitleIndex, string title) { // search titleIndex for mid (position in vector)
    title = trim(title);

    int low = 0, high = rrnTitleIndex.size() - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2; 



        stringstream ss(rrnTitleIndex[mid]);
        string currentTitle;
        int rrn;
        getline(ss, currentTitle, '|'); 
        ss >> rrn;


        currentTitle = trim(currentTitle);

        if (currentTitle == title) {

            return mid; // return the position of the found book
        }
        else if (currentTitle < title) {
            low = mid + 1; // search in the upper half
        }
        else {
            high = mid - 1; // search in the lower half
        }
    }

    return -1; // title not found
}

int searchIndexPositionIsbn(const vector<string>& rrnIsbnIndex, int isbn) { //search isbn index for position

    int low = 0, high = rrnIsbnIndex.size() - 1;

    while (low <= high) {
        int mid = (low + high) / 2;
        stringstream ss(rrnIsbnIndex[mid]);
        string tempIsbn, tempRrn;
        getline(ss, tempIsbn, '|'); // extract the ISBN part
        getline(ss, tempRrn);      // extract the RRN part
        int indexIsbn = stoi(tempIsbn); // convert ISBN to an int
        int rrn = stoi(tempRrn);       // convert RRN to an int
        if (indexIsbn == isbn) //return position
            return mid;
        else if (indexIsbn < isbn) // upper half
            low = mid + 1;
        else // lower half
            high = mid - 1;
    }
    return -1; // book not found
}

int searchIndexIsbn(const vector<string>& rrnIsbnIndex, int isbn) {

    int low = 0, high = rrnIsbnIndex.size() - 1;

    while (low <= high) {
        int mid = (low + high) / 2;
        stringstream ss(rrnIsbnIndex[mid]);
        string tempIsbn, tempRrn;
        getline(ss, tempIsbn, '|'); // extract the ISBN part
        getline(ss, tempRrn);      // extract the RRN part
        int indexIsbn = stoi(tempIsbn); // convert ISBN to an int
        int rrn = stoi(tempRrn);       // convert RRN to an int
        if (indexIsbn == isbn) //return rrn of found book
            return rrn;
        else if (indexIsbn < isbn) //upper
            low = mid + 1;
        else //lower
            high = mid - 1;
    }
    return -1; // book not found
}

// Function to add a new book
void addBook(vector<string>& rrnTitleIndex, vector<string>& rrnIsbnIndex, fstream& dataFile, int& rrn) {
    string author, title, isbn;
    cout << "Enter Author: ";
    cin.ignore();
    getline(cin, author);
    cout << "Enter Title: ";
    getline(cin, title);
    cout << "Enter ISBN: ";
    getline(cin, isbn);
    int bookCheck = searchIndexTitle(rrnTitleIndex, title);
    if (bookCheck != -1) {
        cout << "This book already exists!" << endl;
        return;
    }

    // trim title before adding it to the index
    title = trim(title);

    Book newBook(author, title, isbn);
    int byteOffset = rrn * RECORD_SIZE;
    // write to datafile
    dataFile.seekp(byteOffset, ios::beg);
    cout << "File pointer position: " << dataFile.tellp() << endl;

    newBook.writeToDataFile(dataFile);

    // update indexes
    rrnIsbnIndex.push_back(isbn + "|" + to_string(rrn));
    rrnTitleIndex.push_back(title + "|" + to_string(rrn));
    rrn++; // increment rrn after adding

    auto compareIsbn = [](const string& a, const string& b) { //the sort function waa sorting the isbn as a string, lexiographically, so i wrote this to compare them numerically
        int isbnA = stoi(a.substr(0, a.find('|')));
        int isbnB = stoi(b.substr(0, b.find('|')));
        return isbnA < isbnB;
    };
    // sort the indexes
    sort(rrnIsbnIndex.begin(), rrnIsbnIndex.end(), compareIsbn);
    sort(rrnTitleIndex.begin(), rrnTitleIndex.end());
    cout << "Book added successfully.\n";
    cout << "TitleIndex after sorting:\n";
    for (const auto& entry : rrnTitleIndex) {
        cout << entry << endl;  // output the index entries to verify the format
    }
    cout << "ISBNIndex after sorting: \n";
;    for (const auto& entry : rrnIsbnIndex) {
        cout << entry << endl;  // output the index entries to verify the format
    }
}
// passing both expected title and expected isbn, then we can select which one in the function (i was proud of this)
void deleteBook(vector<string>& rrnTitleIndex, vector<string>& rrnIsbnIndex, fstream& dataFile, int foundRRN, const string& expectedTitle = "", int expectedIsbn = -1) {

    if (foundRRN == -1) {
        cout << "Book not found\n";
        return;
    }

    int byteOffset = foundRRN * RECORD_SIZE;
    dataFile.seekp(byteOffset, ios::beg);

    Book book;
    book.readFromDataFile(dataFile);

    if (book.isDeleted()) {
        cout << "The record is already marked as deleted.\n";
        return;
    }

    // validate either by title or by ISBN depending on what user calls
    if ((!expectedTitle.empty() && book.getTitle() != expectedTitle) ||
        (expectedIsbn != -1 && convertIsbnToInt(book.getIsbn()) != expectedIsbn)) {
        cout << "Record mismatch. Aborting operation.\n";
        return;
    }
    // after validating, set the variables so we can delete them
    string validatedTitle = book.getTitle();
    int validatedIsbn = convertIsbnToInt(book.getIsbn());
    // mark the record as deleted
    book.markAsDeleted();
    dataFile.seekp(byteOffset, ios::beg);
    book.writeToDataFile(dataFile);

    cout << "Book at RRN " << foundRRN << " marked as deleted.\n";
        // remove old entries from indices using binary search, no need to sort after deletions
    int titlePosition = searchIndexPositionTitle(rrnTitleIndex, validatedTitle);
    if (titlePosition != -1) {
        rrnTitleIndex.erase(rrnTitleIndex.begin() + titlePosition);
    }

    int isbnPosition = searchIndexPositionIsbn(rrnIsbnIndex, validatedIsbn);
    if (isbnPosition != -1) {
        rrnIsbnIndex.erase(rrnIsbnIndex.begin() + isbnPosition);
    }
    cout << "TitleIndex after deleting: \n";
    for (const auto& entry : rrnTitleIndex) {
        cout << entry << endl;  // output the index entries to verify the format
    }
    cout << "ISBNIndex after deleting: \n";
    ;    for (const auto& entry : rrnIsbnIndex) {
        cout << entry << endl;  // output the index entries to verify the format
    }
}

// function to display a book record
void displayBookRecord(fstream& dataFile, int rrn) {
    if (rrn < 0) {
        cout << "Book does not exist\n";
        return;
    }

    int byteOffset = rrn * RECORD_SIZE;
    cout << "byte offset: " << byteOffset;
    dataFile.seekp(byteOffset, ios::beg);
    cout << "File pointer position: " << dataFile.tellp() << endl;

    Book book;
    book.readFromDataFile(dataFile);
    book.display();
}

// i realized after i wrote this that i could have called my add and delete functions within the update function. but i already wrote this... so enjoy...
void updateTitleBookRecord(fstream& dataFile, int rrnToUpdate, vector<string>& rrnIsbnIndex, vector<string>& rrnTitleIndex) {
    if (rrnToUpdate == -1) {
        cout << "Book not found\n";
        return;
    }

    int byteOffset = rrnToUpdate * RECORD_SIZE;
    dataFile.seekp(byteOffset, ios::beg);

    Book book;
    book.readFromDataFile(dataFile);
    string oldTitle = book.getTitle();
    string tempOldIsbn = book.getIsbn();
    int oldIsbn = convertIsbnToInt(tempOldIsbn);

    if (book.isDeleted()) {
        cout << "The record is marked as deleted.\n";
        return;
    }

    // get updated book details
    Book updatedBook;
    string updateAuthor, updateTitle, updateISBN;
    cout << "Enter the new author: ";
    getline(cin, updateAuthor); // read the full title including spaces
    cout << "Enter the new title: ";
    getline(cin, updateTitle); // read the full title including spaces
    cout << "Enter the new ISBN: ";
    getline(cin, updateISBN); // read the full title including spaces
    updatedBook.setAuthor(updateAuthor);
    updatedBook.setTitle(updateTitle);
    updatedBook.setIsbn(updateISBN);
    dataFile.seekp(byteOffset, ios::beg);
    updatedBook.writeToDataFile(dataFile);

    cout << "Book at RRN " << rrnToUpdate << " updated to "
        << updateAuthor << " " << updateTitle << " " << updateISBN << endl;

    // remove old entries from indices
    int titlePosition = searchIndexPositionTitle(rrnTitleIndex, oldTitle);
    if (titlePosition != -1) {
        rrnTitleIndex.erase(rrnTitleIndex.begin() + titlePosition);
    }

    int isbnPosition = searchIndexPositionIsbn(rrnIsbnIndex, oldIsbn);
    if (isbnPosition != -1) {
        rrnIsbnIndex.erase(rrnIsbnIndex.begin() + isbnPosition);
    }

    // add updated entries to indices
    rrnIsbnIndex.push_back(updateISBN + "|" + to_string(rrnToUpdate));
    rrnTitleIndex.push_back(updateTitle + "|" + to_string(rrnToUpdate));

    // sort indices
    auto compareIsbn = [](const string& a, const string& b) {
        int isbnA = stoi(a.substr(0, a.find('|')));
        int isbnB = stoi(b.substr(0, b.find('|')));
        return isbnA < isbnB;
    };

    sort(rrnIsbnIndex.begin(), rrnIsbnIndex.end(), compareIsbn);
    sort(rrnTitleIndex.begin(), rrnTitleIndex.end());

    // show new indexes:
    cout << "Book updated successfully.\n";
    cout << "TitleIndex after updating:\n";
    for (const auto& entry : rrnTitleIndex) {
        cout << entry << endl;
    }
    cout << "ISBNIndex after updating: \n";
    for (const auto& entry : rrnIsbnIndex) {
        cout << entry << endl;
    }
}



int main() {
    fstream dataFile("C:\\Users\\jonat\\Documents\\data\\datafile.bin", ios::in | ios::out | ios::binary);
    ifstream titleIndexFileRead("C:\\Users\\jonat\\Documents\\data\\titleIndex.txt");
    ifstream isbnIndexFileRead("C:\\Users\\jonat\\Documents\\data\\isbnIndex.txt");
    string line;
    vector<string> rrnTitleIndex;
    vector<string> rrnIsbnIndex;
    int rrn = 0; 

    // create file if it doesn't exist, make sure all files are open, all the good stuff
    if (!dataFile) {
        fstream createEmpFile("C:\\Users\\jonat\\Documents\\data\\datafile.bin", ios::out | ios::binary);
        createEmpFile.close();
        dataFile.open("C:\\Users\\jonat\\Documents\\data\\datafile.bin", ios::in | ios::out | ios::binary);
    }
    if (!titleIndexFileRead) {
        fstream createTitleIndexFile("C:\\Users\\jonat\\Documents\\data\\titleIndex.txt", ios::out);
        createTitleIndexFile.close();
        titleIndexFileRead.open("C:\\Users\\jonat\\Documents\\data\\titleIndex.txt");
    }
    if (!isbnIndexFileRead) {
        fstream createIsbnIndexFile("C:\\Users\\jonat\\Documents\\data\\isbnIndex.txt", ios::out);
        createIsbnIndexFile.close();
        isbnIndexFileRead.open("C:\\Users\\jonat\\Documents\\data\\isbnIndex.txt");
    }
    if (!dataFile || !titleIndexFileRead || !isbnIndexFileRead) {
        cerr << "Error opening file!" << endl;
        return 1;
    }
    while (getline(titleIndexFileRead, line)) { //put indexes in main memory for processing
        rrnTitleIndex.push_back(line);
    }
    while (getline(isbnIndexFileRead, line)) {
        rrnIsbnIndex.push_back(line);
    }
    if (!rrnTitleIndex.empty()) { // get rrn from the title index if it already created
        if (!rrnTitleIndex.empty()) {

            int maxRRN = -1;  // look for biggest RRN 

            // sequential search... i think if this was in industry, i would use a 3rd file to store the largest RRN, but it wasn't on the requirements so im just gonna be lazy here. please don't fail me greenburg
            for (const auto& entry : rrnTitleIndex) {
                // use delimiter to split the data
                size_t delimiterPos = entry.find('|');

                if (delimiterPos != string::npos) {
                    // extract the RRN part after the delimiter
                    int currentRRN = stoi(entry.substr(delimiterPos + 1));  // convert to int to compare sizes since strings compare lexiographically

                    // update maxRRN if a larger RRN is found
                    if (currentRRN > maxRRN) {
                        maxRRN = currentRRN;
                    }
                }
                else {
                    // handle error if delimiter is not found (invalid format)
                    cout << "Error: Invalid TitleIndex entry format" << endl;
                }
            }

            // set the RRN to the next available value (one more than the largest RRN)
            rrn = maxRRN + 1;
        }
    }

    char choice;
    bool running = true;

    while (running) { //user menu
        cout << "\nChoose an option:\n";
        cout << "1. Add Book\n";
        cout << "2. Display Book by title\n";
        cout << "3. Display Book by ISBN\n";
        cout << "4. Delete Book by title\n";
        cout << "5. Delete Book by ISBN\n";
        cout << "6. Update Book by title\n";
        cout << "7. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
        case '1': {
            addBook(rrnTitleIndex, rrnIsbnIndex, dataFile, rrn);
            break;
        }
        case '2': {
            string titleToDisplay;
            cout << "Enter Title to display: ";
            cin.ignore(); // clear any leftover newline character from previous input
            getline(cin, titleToDisplay); // read the full title including spaces
            int rrnToDisplay = searchIndexTitle(rrnTitleIndex, titleToDisplay);
            displayBookRecord(dataFile, rrnToDisplay);
            break;
        }
        case '3': {
            int isbnToDisplay;
            cout << "Enter ISBN to display: " << endl;
            cin.ignore();
            cin >> isbnToDisplay;
            int rrnToDisplay = searchIndexIsbn(rrnIsbnIndex, isbnToDisplay);
            displayBookRecord(dataFile, rrnToDisplay);
            break;
        }
        case '4': {
            string titleToDelete;
            cout << "Enter title to delete: ";
            cin.ignore(); // clear any leftover newline character from previous input
            getline(cin, titleToDelete); // read the full title including spaces
            int rrnToDelete = searchIndexTitle(rrnTitleIndex, titleToDelete);
            deleteBook(rrnTitleIndex, rrnIsbnIndex ,dataFile, rrnToDelete, titleToDelete, -1);
            break;
        }
        case '5': {
            int isbnToDelete;
            cout << "Enter ISBN to delete: ";
            cin >> isbnToDelete;
            int rrnToDelete = searchIndexIsbn(rrnIsbnIndex, isbnToDelete);
            deleteBook(rrnTitleIndex, rrnIsbnIndex, dataFile, rrnToDelete, "", isbnToDelete);
            break;
        }
        case '6': {
            string titleToUpdate;
            cout << "Enter Title to update: ";
            cin.ignore(); // clear any leftover newline character from previous input
            getline(cin, titleToUpdate); // read the full title including spaces
            int rrnToUpdate = searchIndexTitle(rrnTitleIndex, titleToUpdate);
            updateTitleBookRecord(dataFile, rrnToUpdate, rrnIsbnIndex, rrnTitleIndex);
            break;
        }
        case '7':
            running = false;
            break;
        default:
            cout << "Invalid choice. Please try again.\n";
        }
    }
    // write to index files after you exit the menu
    ofstream titleIndexFileWrite("C:\\Users\\jonat\\Documents\\data\\titleIndex.txt", ios::trunc);
    ofstream isbnIndexFileWrite("C:\\Users\\jonat\\Documents\\data\\isbnIndex.txt", ios::trunc);
    for (const string& line : rrnTitleIndex) {
        titleIndexFileWrite << line << endl;
    }
    for (const string& line : rrnIsbnIndex) {
        isbnIndexFileWrite << line << endl;
    }
    dataFile.close();
    titleIndexFileWrite.close();
    isbnIndexFileWrite.close();
    return 0;

}